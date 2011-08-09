/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
	virtual void do_add_suffixes(std::multimap<std::string, std::string>& map) const;
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
}

const string  CVistaMeshIO::do_get_descr()const
{
	return string("Vista/Simbio triangle mesh input/output support");
}

template <class T>
struct TVNodeType: public VNodeBaseRec {
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
	VNodeBaseRec base;
	int type;
	T3DVector<unsigned int> t;
};


static  CTriangleMesh::PTrianglefield  read_triangles(VGraph graph, size_t v_size)
{
	cvdebug() << "vista_meshio::read_triangles()\n";

	long node_size = graph->nfields;
	if ( node_size != 4 ) {
		cverr() << "only triangles supported\n";
		return CTriangleMesh::PTrianglefield();
	}

	if (VNodeRepn(graph) != VLongRepn) {
		cverr() <<"only long indices supported\n";
		return CTriangleMesh::PTrianglefield();
	}
	TTriangelType *node = (TTriangelType*)VGraphFirstNode(graph);
	CTriangleMesh::PTrianglefield triangles(new CTriangleMesh::CTrianglefield(VGraphNNodes(graph)));

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
		node = (TTriangelType*)VGraphNextNode(graph);
	}
	return triangles;
}

static  CTriangleMesh *read_vgraph(VGraph vgraph, VGraph tgraph)
{
	TVNodeType<float> *node = (TVNodeType<float> *)VGraphFirstNode(vgraph);
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	while (node) {
		if (v == e)
			return NULL;
		*v++ = node->vertex;
		node = (TVNodeType<float> *)VGraphNextNode(vgraph);
	}
	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices);
}

static CTriangleMesh *read_vsgraph(VGraph vgraph, VGraph tgraph)
{
	TVSNodeType<float> *node =
		(TVSNodeType<float> *)VGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield scales(new CTriangleMesh::CScalefield(VGraphNNodes(vgraph)));

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::scale_iterator s =  scales->begin();


	while (node && v != e) {
		*v++ = node->vertex;
		*s++ = node->scale;
		node = (TVSNodeType<float> *)VGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices,
				 CTriangleMesh::PNormalfield(),
				 CTriangleMesh::PColorfield(), scales);

}

static CTriangleMesh *read_vcgraph(VGraph vgraph, VGraph tgraph)
{
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield color(new CTriangleMesh::CColorfield(VGraphNNodes(vgraph)));

	TVCNodeType<float> *node =
		(TVCNodeType<float> *)VGraphFirstNode(vgraph);

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::color_iterator c = color->begin();

	while (node && v != e) {
		*v++ = node->vertex;
		*c++ = node->color;
		node = (TVCNodeType<float> *)VGraphNextNode(vgraph);
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


static CTriangleMesh *read_vcsgraph(VGraph vgraph, VGraph tgraph)
{
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield  color(new CTriangleMesh::CColorfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield  scales(new CTriangleMesh::CScalefield(VGraphNNodes(vgraph)));

	TVCSNodeType<float> *node =
		(TVCSNodeType<float> *)VGraphFirstNode(vgraph);

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::color_iterator c = color->begin();
	CTriangleMesh::scale_iterator s = scales->begin();


	while (node && v != e) {
		*v++ = node->vertex;
		*c++ = node->color;
		*s++ = node->scale;
		node = (TVCSNodeType<float> *)VGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, CTriangleMesh::PNormalfield(), color, scales);
}

static CTriangleMesh *read_vngraph(VGraph vgraph, VGraph tgraph)
{
	TVNNodeType<float> *node =
		(TVNNodeType<float> *)VGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VGraphNNodes(vgraph)));

	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::normal_iterator n = normals->begin();

	while (node && v != e) {
		*v++ =  node->vertex;
		*n++ =  node->normal;
		node = (TVNNodeType<float> *)VGraphNextNode(vgraph);

	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, normals,
				 CTriangleMesh::PColorfield(),
				 CTriangleMesh::PScalefield());
}

static CTriangleMesh *read_vnsgraph(VGraph vgraph, VGraph tgraph)
{
	TVNSNodeType<float> *node =
		(TVNSNodeType<float> *)VGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield  scales(new CTriangleMesh::CScalefield(VGraphNNodes(vgraph)));


	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::normal_iterator n = normals->begin();
	CTriangleMesh::scale_iterator s =  scales->begin();

	while (node && v != e) {
		*n++ = node->normal;
		*v++ = node->vertex;
		*s++ = node->scale;
		node = (TVNSNodeType<float> *)VGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices,
				 normals, CTriangleMesh::PColorfield(), scales);
}


static CTriangleMesh *read_vncgraph(VGraph vgraph, VGraph tgraph)
{
	TVNCNodeType<float> *node =
		(TVNCNodeType<float> *)VGraphFirstNode(vgraph);

	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield  color(new CTriangleMesh::CColorfield(VGraphNNodes(vgraph)));


	CTriangleMesh::vertex_iterator v = vertices->begin();
	CTriangleMesh::vertex_iterator e = vertices->end();
	CTriangleMesh::normal_iterator n = normals->begin();
	CTriangleMesh::color_iterator c = color->begin();

	while (node && v != e) {
		*n++ = node->normal;
		*v++ = node->vertex;
		*c++ = node->color;
		node = (TVNCNodeType<float> *)VGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, normals, color, CTriangleMesh::PScalefield());

}

static CTriangleMesh *read_vncsgraph(VGraph vgraph, VGraph tgraph)
{
	TVNCSNodeType<float> *node =
		(TVNCSNodeType<float> *)VGraphFirstNode(vgraph);
	CTriangleMesh::PVertexfield vertices(new CTriangleMesh::CVertexfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PNormalfield normals(new CTriangleMesh::CNormalfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PColorfield  color(new CTriangleMesh::CColorfield(VGraphNNodes(vgraph)));
	CTriangleMesh::PScalefield  scales(new CTriangleMesh::CScalefield(VGraphNNodes(vgraph)));


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
		node = (TVNCSNodeType<float> *)VGraphNextNode(vgraph);
	}

	CTriangleMesh::PTrianglefield triangles = read_triangles(tgraph, vertices->size());
	if (!triangles) {
		return NULL;
	}
	return new CTriangleMesh(triangles, vertices, normals, color, scales);
}


static VGraph create_vgraph(const CTriangleMesh& mesh)
{
	VGraph result = VCreateGraph(mesh.vertices_size(), 4, VFloatRepn, 0);


	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();

	TVNodeType<float> node;
	node.type = 1;

	int pos = 1;

	while (vi != ve) {
		node.vertex = *vi++;
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}

static VGraph create_vngraph(const CTriangleMesh& mesh)
{
	VGraph result = VCreateGraph(mesh.vertices_size(), 7, VFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_normal_iterator ni = mesh.normals_begin();

	TVNNodeType<float> node;
	node.type = 2;
	int pos = 1;

	while (vi != ve) {
		node.vertex = *vi++;
		node.normal = *ni++;
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}

static VGraph create_vsgraph(const CTriangleMesh& mesh)
{
	VGraph result = VCreateGraph(mesh.vertices_size(), 11, VFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_scale_iterator si = mesh.scale_begin();

	TVSNodeType<float> node;
	node.type = 4;
	int pos = 1;

	while (vi != ve) {
		node.vertex = *vi++;
		node.scale  = *si++;
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}


static VGraph create_vnsgraph(const CTriangleMesh& mesh)
{

	VGraph result = VCreateGraph(mesh.vertices_size(), 8, VFloatRepn, 0);
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
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}

static VGraph create_vcgraph(const CTriangleMesh& mesh)
{

	VGraph result = VCreateGraph(mesh.vertices_size(), 7, VFloatRepn, 0);
	CTriangleMesh::const_vertex_iterator vi = mesh.vertices_begin();
	CTriangleMesh::const_vertex_iterator ve = mesh.vertices_end();
	CTriangleMesh::const_color_iterator  ci  = mesh.color_begin();

	int pos = 1;
	TVNCNodeType<float> node;
	node.type = 7;

	while (vi != ve) {
		node.vertex = *vi++;
		node.color  = *ci++;
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}

static VGraph create_vncgraph(const CTriangleMesh& mesh)
{

	VGraph result = VCreateGraph(mesh.vertices_size(), 10, VFloatRepn, 0);
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

		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}

static VGraph create_vcsgraph(const CTriangleMesh& mesh)
{

	VGraph result = VCreateGraph(mesh.vertices_size(), 8, VFloatRepn, 0);
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
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}
static VGraph create_vncsgraph(const CTriangleMesh& mesh)
{

	VGraph result = VCreateGraph(mesh.vertices_size(), 11, VFloatRepn, 0);
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
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
	}
	return result;
}

typedef VGraph (*vertexgraph_create)(const CTriangleMesh& mesh);
typedef CTriangleMesh *   (*vertexgraph_read)(VGraph graph1, VGraph graph2);

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
	VGraph graph;
	vertexgraph_read   read;
} vertex_reader_t;

static bool  lookup_vertex_graph(VAttrList list, VAttrListPosn *pos, vertex_reader_t *result)
{
	vertexio_t *lookup =  vertexio_lookup;
	while (lookup->simbio_type != 0) {
		if (VLookupAttr(list,lookup->type_name,pos)) {
			VGetAttrValue(pos, NULL, VGraphRepn, &result->graph);
			if (result->graph) {
				result->read = lookup->read;
				cvdebug() <<"found "<<  lookup->type_name << "\n";
				VNextAttr(pos);
				return true;
			}
		}
		++lookup;
	}

	cvwarn() <<"I didn't find a known  graph attribute, will try generic graph\n";

	lookup =  vertexio_lookup;

	VFirstAttr(list, pos);
	while (VAttrExists(pos)) {
		if (VGetAttrRepn(pos) == VGraphRepn){
			cvdebug() <<"Got a graph\n";
			VGetAttrValue(pos, NULL, VGraphRepn, &result->graph);
			TVNodeType<float> *node = (TVNodeType<float> *)VGraphFirstNode(result->graph);
			while (lookup->simbio_type  && lookup->simbio_type != node->type)
				++lookup;

			cvdebug() <<"found "<<  lookup->type_name << "\n";

			result->read = lookup->read;

			VNextAttr(pos);
			break;
		}
		VNextAttr(pos);
	}
	return lookup->simbio_type != 0;
}


static VGraph lookup_triangle_graph(VAttrList list, VAttrListPosn *pos)
{
	VAttrListPosn save_pos = *pos;
	VGraph result = NULL;
	if (VLookupAttr(list,"Triangles",pos)) {
		VGetAttrValue(pos, NULL, VGraphRepn, &result);
		VNextAttr(pos);
	}else{
		*pos = save_pos;
		cvdebug() <<"Didn't find an attribute called 'triangles', will try generic graph\n";
		while (VAttrExists(pos)) {
			if (VGetAttrRepn(pos) == VGraphRepn){
				cvdebug() <<"Got a graph\n";
				VGetAttrValue(pos, NULL, VGraphRepn, &result);
				VNextAttr(pos);
				break;
			}
			VNextAttr(pos);
		}
	}
	return result;
}

static CTriangleMesh *do_load_mesh(VAttrList list)
{
	VAttrListPosn pos;
	vertex_reader_t vertex_source;

	VGraph triangle_graph = NULL;

	if ( ! lookup_vertex_graph(list,&pos,&vertex_source) ) {
		cvwarn() <<"Didn't find a graph type attribute\n";
		return NULL;
	}
	// pos is now behind the vertex - graph

	if ((triangle_graph = lookup_triangle_graph(list,&pos) ) == NULL) {
		cvwarn() <<"Unable to find triangles\n";
		return NULL;
	}

	int v_size = VGraphNNodes(vertex_source.graph);
	int num_faces = VGraphNNodes(triangle_graph);

	if (VNodeRepn(vertex_source.graph) != VFloatRepn) {
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

	VAttrList list= VReadFile(file,0);

	CTriangleMesh *result = NULL;
	if (list) {
		cvdebug() << "Got a valid vista file\n";
		VAttrList nested_list;
		VAttrListPosn pos;

		if (VLookupAttr(list,"mesh",&pos)) {
			VGetAttrValue(&pos,NULL,VAttrListRepn,&nested_list);
		}else{
			cvdebug() << "Let's put meshes in a nested list!\n";
			nested_list = list;
		}

		result = do_load_mesh(nested_list);

		VDestroyAttrList(list);
	}

	return PTriangleMesh(result);
}


static VGraph create_triangle_graph_and_link(const CTriangleMesh& mesh, VGraph vertex_graph)
{
	VGraph result = VCreateGraph(mesh.triangle_size(), 4, VLongRepn, 0);

	VSetAttr(VGraphAttrList(result), "primitive_interp",  NULL, VStringRepn, "surface");
	VSetAttr(VGraphAttrList(result), "implicit_links", NULL, VStringRepn, "true");
	VSetAttr(VGraphAttrList(result), "component_interp", NULL, VStringRepn, "primitive");

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
		VGraphAddNodeAt(result,(VNodestruct*)&node,pos++);
		VGraphLinkNodesBi(vertex_graph, node.t.x, node.t.y );
		VGraphLinkNodesBi(vertex_graph, node.t.y, node.t.z );
		VGraphLinkNodesBi(vertex_graph, node.t.z, node.t.x );
	}

	return result;
}


bool CVistaMeshIO::do_save(string const &  filename, const CTriangleMesh& mesh)const
{
	bool result = false;

	COutputFile outf(filename);
	if (!outf)
		return false;

	VGraph vertex_graph;
	VGraph triangle_graph;
	int  type = mesh.get_available_data();

	cvdebug() << "available: " << type << "\n";

	VAttrList list = VCreateAttrList();

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



	VSetAttr(VGraphAttrList(vertex_graph), "component_interp", NULL, VStringRepn, "vertex");
	VSetAttr(list, lookup->type_name, NULL, VGraphRepn, vertex_graph);

	triangle_graph = create_triangle_graph_and_link(mesh, vertex_graph);
	if (!triangle_graph)
		VDestroyAttrList(list);

	VSetAttr(list, "Triangles", NULL, VGraphRepn, triangle_graph);

	result = (VWriteFile(outf, list) == TRUE);
	VDestroyAttrList(list);

	return result;
}

void CVistaMeshIO::do_add_suffixes(std::multimap<std::string, std::string>& map) const
{
	map.insert(pair<string,string>(".v", get_name()));
	map.insert(pair<string,string>(".V", get_name()));
	map.insert(pair<string,string>(".vmesh", get_name()));
	map.insert(pair<string,string>(".VMESH", get_name()));
}

NS_END
