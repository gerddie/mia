/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vistaio/vista4mia.hh>
#include <mia/mesh/triangularMesh.hh>

NS_BEGIN(vista_mesh_io)

NS_MIA_USE;
using namespace std;

static char const * const format = "vista";

class CVistaMeshIO: public CMeshIOPlugin {
public:
	CVistaMeshIO();
private:
	virtual PTriangleMesh do_load(string const &  filename) const;
	virtual bool do_save(string const &  filename, const CTriangleMesh& data) const;
	const string  do_get_descr() const;
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	//vista::prepare_vista_logging();
	return new CVistaMeshIO;
}

// implementation part


CVistaMeshIO::CVistaMeshIO():
	CMeshIOPlugin(format)
{
	//add_property(io_plugin_property_history);
	add_suffix(".v");
	add_suffix(".V");
	add_suffix(".vmesh");
	add_suffix(".VMESH");

}

const string  CVistaMeshIO::do_get_descr()const
{
	return string("Vista/Simbio triangle mesh input/output support");
}

template <class T>
struct TVNodeType: public VistaIONodeBaseRec {
	T type;
	T3DVector<T> vertex;
};

template <class T>
struct TVNNodeType: public TVNodeType<T> {
	T3DVector<T> normal;
};

template <class T>
struct TVSNodeType: public TVNodeType<T> {
	T scale;
};

template <class T>
struct TVCNodeType: public TVNodeType<T> {
	T3DVector<T> color;
};

template <class T>
struct TVNCNodeType: public TVNNodeType<T> {
	T3DVector<T> color;
};

template <class T>
struct TVCSNodeType: public TVCNodeType<T> {
	T scale;
};

template <class T>
struct TVNSNodeType: public TVNNodeType<T> {
	T scale;
};

template <class T>
struct TVNCSNodeType: public TVNCNodeType<T> {
	T scale;
};

struct TTriangelType {
	VistaIONodeBaseRec base;
	int type;
	T3DVector<unsigned int> t;
};


static  CTriangleMesh::PTrianglefield  read_triangles(VistaIOGraph graph, size_t v_size)
{
	cvdebug() << "vista_meshio::read_triangles()\n";

	long node_size = graph->nfields;
	if ( node_size != 4 ) {
		cverr() << "only triangles supported\n";
		return CTriangleMesh::PTrianglefield();
	}

	if (VistaIONodeRepn(graph) != VistaIOLongRepn) {
		cverr() <<"only long indices supported\n";
		return CTriangleMesh::PTrianglefield();
	}
	TTriangelType *node = (TTriangelType*)VistaIOGraphFirstNode(graph);
	CTriangleMesh::PTrianglefield triangles(new CTriangleMesh::CTrianglefield(VistaIOGraphNNodes(graph)));

	CTriangleMesh::triangle_iterator t = triangles->begin();
	CTriangleMesh::triangle_iterator e = triangles->end();

	while (node) {

		if (t == e) {
			return CTriangleMesh::PTrianglefield();
		}

		node->t.x--;
		node->t.y--;
		node->t.z--;
		if (node->t.x < v_size &&
		    node->t.y < v_size &&
		    node->t.z < v_size) {
			// set triangle values
			*t++ = node->t;
		}else{
			cverr() <<"adressing vertexes " << node->t << "but have only"<<v_size<< "vertices\n";
			cverr() <<"Bougus mesh: Triangle vertex out of bounds - bailing out\n";
			return CTriangleMesh::PTrianglefield();
		}
		node = (TTriangelType*)VistaIOGraphNextNode(graph);
	}
	return triangles;
}

static  CTriangleMesh *read_vgraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	TVNodeType<float> *node = (TVNodeType<float> *)VistaIOGraphFirstNode(vgraph);
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	while (node) {
		if (v == e)
			return NULL;
		*v++ = node->vertex;
		node = (TVNodeType<float> *)VistaIOGraphNextNode(vgraph);
	}
	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices);
}

static CTriangleMesh *read_vsgraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	TVSNodeType<float> *node =
		(TVSNodeType<float> *)VistaIOGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield scales(new CTriangleMesh::CScalefield(VistaIOGraphNNodes(vgraph)));

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::scale_iterator s =  scales->begin();


	while (node && v != e) {
		*v++ = node->vertex;
		*s++ = node->scale;
		node = (TVSNodeType<float> *)VistaIOGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices,
				 CTriangleMesh::PNormalfield(),
				 CTriangleMesh::PColorfield(), scales);

}

static CTriangleMesh *read_vcgraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield color(new CTriangleMesh::CColorfield(VistaIOGraphNNodes(vgraph)));

	TVCNodeType<float> *node =
		(TVCNodeType<float> *)VistaIOGraphFirstNode(vgraph);

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::color_iterator c = color->begin();

	while (node && v != e) {
		*v++ = node->vertex;
		*c++ = node->color;
		node = (TVCNodeType<float> *)VistaIOGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices,
				 CTriangleMesh::PNormalfield(),
				 color,
				 CTriangleMesh::PScalefield());
}


static CTriangleMesh *read_vcsgraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield  color(new CTriangleMesh::CColorfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield  scales(new CTriangleMesh::CScalefield(VistaIOGraphNNodes(vgraph)));

	TVCSNodeType<float> *node =
		(TVCSNodeType<float> *)VistaIOGraphFirstNode(vgraph);

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::color_iterator c = color->begin();
	CTriangleMesh::scale_iterator s = scales->begin();


	while (node && v != e) {
		*v++ = node->vertex;
		*c++ = node->color;
		*s++ = node->scale;
		node = (TVCSNodeType<float> *)VistaIOGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, CTriangleMesh::PNormalfield(), color, scales);
}

static CTriangleMesh *read_vngraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	TVNNodeType<float> *node =
		(TVNNodeType<float> *)VistaIOGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VistaIOGraphNNodes(vgraph)));

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::normal_iterator n = normals->begin();

	while (node && v != e) {
		*v++ =  node->vertex;
		*n++ =  node->normal;
		node = (TVNNodeType<float> *)VistaIOGraphNextNode(vgraph);

	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, normals,
				 CTriangleMesh::PColorfield(),
				 CTriangleMesh::PScalefield());
}

static CTriangleMesh *read_vnsgraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	TVNSNodeType<float> *node =
		(TVNSNodeType<float> *)VistaIOGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield  scales(new CTriangleMesh::CScalefield(VistaIOGraphNNodes(vgraph)));


	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::normal_iterator n = normals->begin();
	CTriangleMesh::scale_iterator s =  scales->begin();

	while (node && v != e) {
		*n++ = node->normal;
		*v++ = node->vertex;
		*s++ = node->scale;
		node = (TVNSNodeType<float> *)VistaIOGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices,
				 normals, CTriangleMesh::PColorfield(), scales);
}


static CTriangleMesh *read_vncgraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	TVNCNodeType<float> *node =
		(TVNCNodeType<float> *)VistaIOGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield  color(new CTriangleMesh::CColorfield(VistaIOGraphNNodes(vgraph)));


	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::normal_iterator n = normals->begin();
	CTriangleMesh::color_iterator c = color->begin();

	while (node && v != e) {
		*n++ = node->normal;
		*v++ = node->vertex;
		*c++ = node->color;
		node = (TVNCNodeType<float> *)VistaIOGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, normals, color, CTriangleMesh::PScalefield());

}

static CTriangleMesh *read_vncsgraph(VistaIOGraph vgraph, VistaIOGraph tgraph)
{
	TVNCSNodeType<float> *node =
		(TVNCSNodeType<float> *)VistaIOGraphFirstNode(vgraph);
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield  color(new CTriangleMesh::CColorfield(VistaIOGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield  scales(new CTriangleMesh::CScalefield(VistaIOGraphNNodes(vgraph)));


	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::normal_iterator n = normals->begin();
	CTriangleMesh::color_iterator c = color->begin();
	CTriangleMesh::scale_iterator s = scales->begin();

	while (node && v != e) {
		*n++ = node->normal;
		*v++ = node->vertex;
		*c++ = node->color;
		*s++ = node->scale;
		node = (TVNCSNodeType<float> *)VistaIOGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, normals, color, scales);
}


static VistaIOGraph create_vgraph(const CTriangleMesh& mesh)
{
	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 4, VistaIOFloatRepn, 0);


	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();

	TVNodeType<float> node;
	node.type = 1;

	int pos = 1;

	while (vi != ve) {
		node.vertex = *vi++;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}

static VistaIOGraph create_vngraph(const CTriangleMesh& mesh)
{
	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 7, VistaIOFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_normal_iterator ni = mesh.normals_begin();

	TVNNodeType<float> node;
	node.type = 2;
	int pos = 1;

	while (vi != ve) {
		node.vertex = *vi++;
		node.normal = *ni++;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}

static VistaIOGraph create_vsgraph(const CTriangleMesh& mesh)
{
	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 11, VistaIOFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_scale_iterator si = mesh.scale_begin();

	TVSNodeType<float> node;
	node.type = 4;
	int pos = 1;

	while (vi != ve) {
		node.vertex = *vi++;
		node.scale  = *si++;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}


static VistaIOGraph create_vnsgraph(const CTriangleMesh& mesh)
{

	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 8, VistaIOFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_normal_iterator ni = mesh.normals_begin();
	CTriangleMesh::const_scale_iterator si = mesh.scale_begin();

	int pos = 1;
	TVNSNodeType<float> node;
	node.type = 5;

	while (vi != ve) {
		node.vertex = *vi++;
		node.normal = *ni++;
		node.scale  = *si++;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}

static VistaIOGraph create_vcgraph(const CTriangleMesh& mesh)
{

	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 7, VistaIOFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_color_iterator  ci  = mesh.color_begin();

	int pos = 1;
	TVNCNodeType<float> node;
	node.type = 7;

	while (vi != ve) {
		node.vertex = *vi++;
		node.color  = *ci++;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}

static VistaIOGraph create_vncgraph(const CTriangleMesh& mesh)
{

	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 10, VistaIOFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_normal_iterator ni = mesh.normals_begin();
	CTriangleMesh::const_color_iterator  ci = mesh.color_begin();

	int pos = 1;
	TVNCNodeType<float> node;
	node.type = 3;

	while (vi != ve) {
		node.vertex = *vi++;
		node.normal = *ni++;
		node.color  = *ci++;

		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}

static VistaIOGraph create_vcsgraph(const CTriangleMesh& mesh)
{

	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 8, VistaIOFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_color_iterator  ci = mesh.color_begin();
	CTriangleMesh::const_scale_iterator si = mesh.scale_begin();

	TVCSNodeType<float> node;
	node.type = 8;
	int pos = 1;


	while (vi != ve) {
		node.vertex = *vi++;
		node.color  = *ci++;
		node.scale  = *si++;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}
static VistaIOGraph create_vncsgraph(const CTriangleMesh& mesh)
{

	VistaIOGraph result = VistaIOCreateGraph(mesh.vertices_size(), 11, VistaIOFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_normal_iterator ni = mesh.normals_begin();
	CTriangleMesh::const_color_iterator  ci = mesh.color_begin();
	CTriangleMesh::const_scale_iterator si = mesh.scale_begin();

	TVNCSNodeType<float> node;
	node.type = 6;
	int pos = 1;

	while (vi != ve) {
		node.vertex = *vi++;
		node.normal = *ni++;
		node.color  = *ci++;
		node.scale  = *si++;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
	}
	return result;
}

typedef VistaIOGraph (*vertexgraph_create)(const CTriangleMesh& mesh);
typedef CTriangleMesh *   (*vertexgraph_read)(VistaIOGraph graph1, VistaIOGraph graph2);

typedef struct {
	int simbio_type;
	const char *type_name;
	vertexgraph_create create;
	vertexgraph_read   read;
	int data_avail;
} vertexio_t;

vertexio_t vertexio_lookup[]  = {
	{ 1, "Vertex",                  create_vgraph,    read_vgraph   ,
	  CTriangleMesh::ed_vertex },

	{ 2, "VertexNormal",            create_vngraph,   read_vngraph  ,
	  CTriangleMesh::ed_vertex | CTriangleMesh::ed_normal},

	{ 4, "VertexScalar",            create_vsgraph,   read_vsgraph  ,
	  CTriangleMesh::ed_vertex | CTriangleMesh::ed_scale},

	{ 5, "VertexNormalScalar",      create_vnsgraph,  read_vnsgraph ,
	  CTriangleMesh::ed_vertex | CTriangleMesh::ed_normal| CTriangleMesh::ed_scale},

	{ 7, "VertexColor",             create_vcgraph,   read_vcgraph  ,
	  CTriangleMesh::ed_vertex | CTriangleMesh::ed_color},

	{ 8, "VertexColorScalar",       create_vcsgraph,  read_vcsgraph ,
	  CTriangleMesh::ed_vertex | CTriangleMesh::ed_color| CTriangleMesh::ed_scale},

	{ 3, "VertexNormalColor",       create_vncgraph,  read_vncgraph ,
	  CTriangleMesh::ed_vertex | CTriangleMesh::ed_normal | CTriangleMesh::ed_color},

	{ 6, "VertexNormalColorScalar", create_vncsgraph, read_vncsgraph,
	  CTriangleMesh::ed_vertex | CTriangleMesh::ed_normal
	  | CTriangleMesh::ed_color | CTriangleMesh::ed_scale},

	{ 0, NULL, NULL, NULL, 0}
};

typedef struct {
	VistaIOGraph graph;
	vertexgraph_read   read;
} vertex_reader_t;

static bool  lookup_vertex_graph(VistaIOAttrList list, VistaIOAttrListPosn *pos, vertex_reader_t *result)
{
	vertexio_t *lookup =  vertexio_lookup;
	while (lookup->simbio_type != 0) {
		if (VistaIOLookupAttr(list,lookup->type_name,pos)) {
			VistaIOGetAttrValue(pos, NULL, VistaIOGraphRepn, &result->graph);
			if (result->graph) {
				result->read = lookup->read;
				cvdebug() <<"found "<<  lookup->type_name << "\n";
				VistaIONextAttr(pos);
				return true;
			}
		}
		++lookup;
	}

	cvwarn() <<"I didn't find a known  graph attribute, will try generic graph\n";

	lookup =  vertexio_lookup;

	VistaIOFirstAttr(list, pos);
	while (VistaIOAttrExists(pos)) {
		if (VistaIOGetAttrRepn(pos) == VistaIOGraphRepn){
			cvdebug() <<"Got a graph\n";
			VistaIOGetAttrValue(pos, NULL, VistaIOGraphRepn, &result->graph);
			TVNodeType<float> *node = (TVNodeType<float> *)VistaIOGraphFirstNode(result->graph);
			while (lookup->simbio_type  && lookup->simbio_type != node->type)
				++lookup;

			cvdebug() <<"found "<<  lookup->type_name << "\n";

			result->read = lookup->read;

			VistaIONextAttr(pos);
			break;
		}
		VistaIONextAttr(pos);
	}
	return lookup->simbio_type != 0;
}


static VistaIOGraph lookup_triangle_graph(VistaIOAttrList list, VistaIOAttrListPosn *pos)
{
	VistaIOAttrListPosn save_pos = *pos;
	VistaIOGraph result = NULL;
	if (VistaIOLookupAttr(list,"Triangles",pos)) {
		VistaIOGetAttrValue(pos, NULL, VistaIOGraphRepn, &result);
		VistaIONextAttr(pos);
	}else{
		*pos = save_pos;
		cvdebug() <<"Didn't find an attribute called 'triangles', will try generic graph\n";
		while (VistaIOAttrExists(pos)) {
			if (VistaIOGetAttrRepn(pos) == VistaIOGraphRepn){
				cvdebug() <<"Got a graph\n";
				VistaIOGetAttrValue(pos, NULL, VistaIOGraphRepn, &result);
				VistaIONextAttr(pos);
				break;
			}
			VistaIONextAttr(pos);
		}
	}
	return result;
}

static CTriangleMesh *do_load_mesh(VistaIOAttrList list)
{
	VistaIOAttrListPosn pos;
	vertex_reader_t vertex_source;

	VistaIOGraph triangle_graph = NULL;

	if ( ! lookup_vertex_graph(list,&pos,&vertex_source) ) {
		cvwarn() <<"Didn't find a graph type attribute\n";
		return NULL;
	}
	// pos is now behind the vertex - graph

	if ((triangle_graph = lookup_triangle_graph(list,&pos) ) == NULL) {
		cvwarn() <<"Unable to find triangles\n";
		return NULL;
	}

	int v_size = VistaIOGraphNNodes(vertex_source.graph);
	int num_faces = VistaIOGraphNNodes(triangle_graph);

	if (VistaIONodeRepn(vertex_source.graph) != VistaIOFloatRepn) {
		cvwarn() <<"Sorry, only float vertex coordinates supported\n";
		return NULL;
	}

	cvdebug() << "Create mesh with " << v_size << " vertices and " << num_faces << " faces.\n";



	return vertex_source.read(vertex_source.graph, triangle_graph);
}

PTriangleMesh CVistaMeshIO::do_load(string const &  filename) const
{


	CInputFile file(filename);
	if (!file)
		return PTriangleMesh();

	VistaIOAttrList list= VistaIOReadFile(file,0);

	CTriangleMesh *result = NULL;
	if (list) {
		cvdebug() << "Got a valid vista file\n";
		VistaIOAttrList nested_list;
		VistaIOAttrListPosn pos;

		if (VistaIOLookupAttr(list,"mesh",&pos)) {
			VistaIOGetAttrValue(&pos,NULL,VistaIOAttrListRepn,&nested_list);
		}else{
			cvdebug() << "Let's put meshes in a nested list!\n";
			nested_list = list;
		}

		result = do_load_mesh(nested_list);

		VistaIODestroyAttrList(list);
	}

	return PTriangleMesh(result);
}


static VistaIOGraph create_triangle_graph_and_link(const CTriangleMesh& mesh, VistaIOGraph vertex_graph)
{
	VistaIOGraph result = VistaIOCreateGraph(mesh.triangle_size(), 4, VistaIOLongRepn, 0);

	VistaIOSetAttr(VistaIOGraphAttrList(result), "primitive_interp",  NULL, VistaIOStringRepn, "surface");
	VistaIOSetAttr(VistaIOGraphAttrList(result), "implicit_links", NULL, VistaIOStringRepn, "true");
	VistaIOSetAttr(VistaIOGraphAttrList(result), "component_interp", NULL, VistaIOStringRepn, "primitive");

	CTriangleMesh::const_triangle_iterator ti = mesh.triangles_begin();
	CTriangleMesh::const_triangle_iterator te = mesh.triangles_end();

	TTriangelType node;
	node.type = 3;
	int pos = 1;

	while (ti != te) {
		node.t = *ti++;
		++node.t.x;
		++node.t.y;
		++node.t.z;
		VistaIOGraphAddNodeAt(result,(VistaIONodestruct*)&node,pos++);
		VistaIOGraphLinkNodesBi(vertex_graph, node.t.x, node.t.y );
		VistaIOGraphLinkNodesBi(vertex_graph, node.t.y, node.t.z );
		VistaIOGraphLinkNodesBi(vertex_graph, node.t.z, node.t.x );
	}

	return result;
}


bool CVistaMeshIO::do_save(string const &  filename, const CTriangleMesh& mesh)const
{
	bool result = false;

	COutputFile outf(filename);
	if (!outf)
		return false;

	VistaIOGraph vertex_graph;
	VistaIOGraph triangle_graph;
	int  type = mesh.get_available_data();

	cvdebug() << "available: " << type << "\n";

	VistaIOAttrList list = VistaIOCreateAttrList();

	vertexio_t *lookup =  vertexio_lookup;

	while (lookup->data_avail && lookup->data_avail != type)
		++lookup;

	cvdebug() << "will save " << lookup->type_name << "\n";

	if (!lookup->data_avail) {
		cverr() << "Fatal: unknown data in mesh\n";
		return false;
	}

	vertex_graph = lookup->create(mesh);

	if (!vertex_graph) {
		cverr() << "Fatal: unable to create vertex graph\n";
		return false;
	}



	VistaIOSetAttr(VistaIOGraphAttrList(vertex_graph), "component_interp", NULL, VistaIOStringRepn, "vertex");
	VistaIOSetAttr(list, lookup->type_name, NULL, VistaIOGraphRepn, vertex_graph);

	triangle_graph = create_triangle_graph_and_link(mesh, vertex_graph);
	if (!triangle_graph)
		VistaIODestroyAttrList(list);

	VistaIOSetAttr(list, "Triangles", NULL, VistaIOGraphRepn, triangle_graph);

	result = (VistaIOWriteFile(outf, list) == TRUE);
	VistaIODestroyAttrList(list);

	return result;
}

NS_END
