/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <mia/core/export_handler.hh>

#define _USE_MATH_DEFINES
#include <cmath>
#include <set>
#include <mia/mesh/triangularMesh.hh>

#include <mia/core/parallel.hh>
#include <mia/core/threadedmsg.hh>


NS_MIA_BEGIN

using namespace std;
using namespace boost;

struct CTriangleMeshData {

	CTriangleMesh::PTrianglefield  m_triangles ;
	CTriangleMesh::PVertexfield    m_vertices;
	CTriangleMesh::PNormalfield    m_normals;
	CTriangleMesh::PScalefield     m_scales;
	CTriangleMesh::PColorfield     m_colors;

	CTriangleMeshData(int n_triangles,int n_vertices);

	CTriangleMeshData(CTriangleMesh::PTrianglefield  triangles, int n_vertices);

	CTriangleMeshData(CTriangleMesh::PTrianglefield triangles,
			  CTriangleMesh::PVertexfield vertices,
			  CTriangleMesh::PNormalfield normals =
			            CTriangleMesh::PNormalfield(),
			  CTriangleMesh::PColorfield colors =
			            CTriangleMesh::PColorfield(),
			  CTriangleMesh::PScalefield scales =
			            CTriangleMesh::PScalefield());

	CTriangleMeshData(CTriangleMesh::CTrianglefield triangles, int n_vertices);
	CTriangleMeshData(const CTriangleMeshData& org);
	~CTriangleMeshData();

	void evaluate_normals();
};


CTriangleMeshData::CTriangleMeshData(int n_triangles,int n_vertices):
	m_triangles(new CTriangleMesh::CTrianglefield(n_triangles)),
	m_vertices(new CTriangleMesh::CVertexfield(n_vertices))
{
}

CTriangleMeshData::CTriangleMeshData(CTriangleMesh::PTrianglefield triangles,
				     CTriangleMesh::PVertexfield vertices,
				     CTriangleMesh::PNormalfield normals,
				     CTriangleMesh::PColorfield colors,
				     CTriangleMesh::PScalefield scales):
	m_triangles(triangles),
	m_vertices(vertices),
	m_normals(normals),
	m_scales(scales),
	m_colors(colors)
{
	assert(m_triangles && m_vertices);
}

CTriangleMeshData::CTriangleMeshData(CTriangleMesh::PTrianglefield  triangles, int n_vertices):
	m_triangles(triangles),
	m_vertices(new CTriangleMesh::CVertexfield(n_vertices))
{
}

CTriangleMeshData::CTriangleMeshData(const CTriangleMeshData& orig):
	m_triangles(orig.m_triangles),
	m_vertices(orig.m_vertices),
	m_normals(orig.m_normals),
	m_scales(orig.m_scales),
	m_colors(orig.m_colors)
{
	assert(m_triangles && m_vertices);
}

CTriangleMeshData::~CTriangleMeshData()
{
}

const std::string CTriangleMesh::get_type_description()
{
	return std::string("triangle_mesh");
}


template <typename T>
void ensure_single_refered(std::shared_ptr<T >& data)
{
	if (!data.unique())
		data.reset(new T(*data));
}

/* evaluate the normalized normal at a vertex.  The value of the normal is
   area-weighted
*/
void CTriangleMeshData::evaluate_normals()
{
	assert(m_vertices && m_triangles);
	cvdebug() << "CTriangleMeshData::evaluate_normals() from " << m_vertices->size() << " vertices\n";

	if (!m_normals || m_normals->empty() )
		// if no normals are available yet, created them
		m_normals.reset(new CTriangleMesh::CNormalfield(m_vertices->size()));
	else
		// before overwriting the normales, make sure we work on this set
		ensure_single_refered(m_normals);
	// zero out normals

	// the writable iterators, index operators  do check against multiple referencing of the data.
	// We only need read access, therefore a const reference makes sure,
	// the iterators are created from the const function
	const CTriangleMesh::CTrianglefield&  ctriangles = *m_triangles;
	//const CTriangleMesh::vertexfield&    cvertices = *m_vertices;

	cvdebug() << "Mesh has " << ctriangles.size() << " triangles\n";


	auto normalize = [](C3DFVector& x) {
		float xn = x.norm(); 
		if (xn > 0) 
			x /= xn; 
	}; 

	typedef CTriangleMesh::normal_type normal_type; 
	typedef CTriangleMesh::triangle_type triangle_type; 

	auto run_triangles = [this, &ctriangles, &normalize](const C1DParallelRange& range, 
						 const vector<normal_type>& cnormals) {
		vector<normal_type> normals(cnormals); 
		CThreadMsgStream thread_stream;
		for (auto i = range.begin(); i != range.end(); ++i) {
			triangle_type t = ctriangles[i]; 
			C3DFVector e1 = (*m_vertices)[t.x] - (*m_vertices)[t.y];
			C3DFVector e2 = (*m_vertices)[t.z] - (*m_vertices)[t.y];
			C3DFVector e3 = (*m_vertices)[t.z] - (*m_vertices)[t.x];
			C3DFVector help_normal = cross(e2, e1);
			if (help_normal.norm2() > 0) {
				normalize(e1); 
				normalize(e2); 
				normalize(e3); 
				float weight1 = acos(dot(e1,e2));
				float weight2 = acos(dot(e3,e2));
				
				normals[t.y] += weight1 * help_normal;
				normals[t.z] += weight2 * help_normal;
				
				float weight3 = M_PI - weight1  - weight2;
				
				normals[t.x] += weight3 * help_normal;
			}else {
				cverr() <<"CTriangleMeshData::evaluate_normals(): triangle " << i 
					<< ":" << t << " with corners [" 
					<< e1 << e2 << e3 << "] has zero normal\n";
			}
		}
		return normals; 
	}; 
	
	auto reduce_normals = [](const vector<normal_type>& lhs, const vector<normal_type>& rhs) -> vector<normal_type> {
		vector<normal_type> result(lhs); 
		pfor(C1DParallelRange(0,result.size()), 
		     [&result, &rhs](const C1DParallelRange& range) {
			     for (auto i= range.begin(); i != range.end(); ++i) 
				     result[i] += rhs[i]; 
		     }); 
		return result; 
	}; 
	
	vector<normal_type> normals_accumulator(m_normals->size());
	
	normals_accumulator = preduce(C1DParallelRange(0, ctriangles.size(), 100), normals_accumulator, 
				      run_triangles, reduce_normals);
	
	pfor(C1DParallelRange(0, normals_accumulator.size()),
	     [this, &normals_accumulator](const C1DParallelRange& range){
		     for (auto i= range.begin(); i != range.end(); ++i) {
			     C3DFVector n = normals_accumulator[i];
			     auto nn = n.norm2();
			     (*m_normals)[i] = (nn > 0) ? n / sqrt(nn) : C3DFVector::_0; 
		     }
	     });
}

CTriangleMesh::CTriangleMesh(const CTriangleMesh& orig):
	data(new CTriangleMeshData(*orig.data))
{
}


CTriangleMesh::CTriangleMesh( PTrianglefield triangles,  PVertexfield vertices,
			      PNormalfield normals,
			      PColorfield colors,
			      PScalefield scale):
	data(new CTriangleMeshData(triangles, vertices, normals, colors, scale))
{
}

CTriangleMesh::CTriangleMesh(int n_triangles, int n_vertices):
	data(new CTriangleMeshData(n_triangles, n_vertices))
{
}

CTriangleMesh::CTriangleMesh(PTrianglefield triangles, PVertexfield vertices):
	data(new CTriangleMeshData(triangles, vertices, PNormalfield(), PColorfield(), PScalefield()))
{
}

CTriangleMesh CTriangleMesh::clone_connectivity()const
{
	PVertexfield vertices(new CVertexfield(vertices_size()));
	CTriangleMesh result(data->m_triangles, vertices);
	return result;
}

CTriangleMesh *CTriangleMesh::clone() const
{
	return new CTriangleMesh(*this);
}

CTriangleMesh::~CTriangleMesh()
{
	delete data;
}


const void *CTriangleMesh::get_vertex_pointer()const
{
	return data->m_vertices ? &(*data->m_vertices)[0].x : NULL;
}

const void *CTriangleMesh::get_normal_pointer()const
{
	return data->m_normals ? &(*data->m_normals)[0].x : NULL;
}

const void *CTriangleMesh::get_color_pointer()const
{
	return data->m_colors ? &(*data->m_colors)[0].x : NULL;
}


const void *CTriangleMesh::get_triangle_pointer()const
{
	return  data->m_triangles ? &(*data->m_triangles)[0].x : NULL;
}

 unsigned int CTriangleMesh::triangle_size()const
{
	return data->m_triangles->size();
}

unsigned int CTriangleMesh::vertices_size()const
{
	return data->m_vertices->size();
}

const CTriangleMesh::triangle_type&    CTriangleMesh::triangle_at(unsigned int i)const
{
	assert(i < data->m_triangles->size());
	return (*data->m_triangles)[i];
}

const CTriangleMesh::vertex_type&      CTriangleMesh::vertex_at(unsigned int i)const
{
	assert(i < data->m_vertices->size());
	return (*data->m_vertices)[i];
}

const CTriangleMesh::normal_type&      CTriangleMesh::normal_at(unsigned int i)const
{
	assert(i < data->m_vertices->size() && data->m_normals);
	return (*data->m_normals)[i];
}

const CTriangleMesh::scale_type&      CTriangleMesh::scale_at(unsigned int i)const
{
	assert(i < data->m_vertices->size() && data->m_scales);
	return (*data->m_scales)[i];
}

const CTriangleMesh::color_type&       CTriangleMesh::color_at(unsigned int i)const
{
	assert(i < data->m_vertices->size() && data->m_colors);
	return (*data->m_colors)[i];

}

CTriangleMesh::const_triangle_iterator CTriangleMesh::triangles_begin() const
{
	const CTrianglefield& c = *data->m_triangles;
	return c.begin();
}

CTriangleMesh::const_triangle_iterator CTriangleMesh::triangles_end() const
{
	const CTrianglefield& c = *data->m_triangles;
	return c.end();
}


CTriangleMesh::triangle_iterator CTriangleMesh::triangles_begin()
{
	ensure_single_refered(data->m_triangles);
	return data->m_triangles->begin();
}

CTriangleMesh::triangle_iterator CTriangleMesh::triangles_end()
{
	ensure_single_refered(data->m_triangles);
	return data->m_triangles->end();
}

CTriangleMesh::const_vertex_iterator CTriangleMesh::vertices_begin()const
{
	const CVertexfield& c = *data->m_vertices;
	return c.begin();
}

CTriangleMesh::const_vertex_iterator CTriangleMesh::vertices_end()const
{
	const CVertexfield& c = *data->m_vertices;
	return c.end();
}

CTriangleMesh::vertex_iterator CTriangleMesh::vertices_begin()
{
	ensure_single_refered(data->m_vertices);
	return data->m_vertices->begin();
}

CTriangleMesh::vertex_iterator CTriangleMesh::vertices_end()
{
	ensure_single_refered(data->m_vertices);
	return data->m_vertices->end();
}

CTriangleMesh::const_normal_iterator CTriangleMesh::normals_begin()const
{
	if (!data->m_normals)
		data->evaluate_normals();

	const CNormalfield& c = *data->m_normals;
	return c.begin();
}

CTriangleMesh::const_normal_iterator CTriangleMesh::normals_end()const
{
	if (!data->m_normals)
		data->evaluate_normals();

	const CNormalfield& c = *data->m_normals;
	return c.end();
}

CTriangleMesh::normal_iterator CTriangleMesh::normals_begin()
{
	if (!data->m_normals)
		data->m_normals.reset(new CTriangleMesh::CNormalfield(data->m_vertices->size()));
	else
		ensure_single_refered(data->m_normals);


	return data->m_normals->begin();
}

CTriangleMesh::normal_iterator CTriangleMesh::normals_end()
{
	if (!data->m_normals)
		data->m_normals.reset(new CTriangleMesh::CNormalfield(data->m_vertices->size()));
	else
		ensure_single_refered(data->m_normals);

	return data->m_normals->begin();
}

CTriangleMesh::const_scale_iterator CTriangleMesh::scale_begin()const
{
	if (!data->m_scales)
		return const_scale_iterator();

	const CScalefield& c = *data->m_scales;
	return c.begin();
}

CTriangleMesh::const_scale_iterator CTriangleMesh::scale_end()const
{
	if (!data->m_scales)
		return const_scale_iterator();

	const CScalefield& c = *data->m_scales;
	return c.end();
}

CTriangleMesh::scale_iterator       CTriangleMesh::scale_begin()
{
	if (!data->m_scales)
		data->m_scales.reset(new CScalefield(data->m_vertices->size()));
	else
		ensure_single_refered(data->m_scales);
	return data->m_scales->begin();
}

CTriangleMesh::scale_iterator       CTriangleMesh::scale_end()
{
	if (!data->m_scales)
		data->m_scales.reset(new CScalefield(data->m_vertices->size()));
	else
		ensure_single_refered(data->m_scales);

	return data->m_scales->end();
}

CTriangleMesh::const_color_iterator CTriangleMesh::color_begin()const
{
	if (!data->m_colors)
		return const_color_iterator();

	const CColorfield& c = *data->m_colors;
	return c.begin();
}

CTriangleMesh::const_color_iterator CTriangleMesh::color_end()const
{
	if (!data->m_colors)
		return const_color_iterator();

	const CColorfield& c = *data->m_colors;
	return c.end();
}

CTriangleMesh::color_iterator       CTriangleMesh::color_begin()
{
	if (!data->m_colors)
		data->m_colors.reset(new CColorfield(data->m_vertices->size()));
	else
		ensure_single_refered(data->m_colors);

	return data->m_colors->begin();
}

CTriangleMesh::color_iterator       CTriangleMesh::color_end()
{
	if (!data->m_colors)
		data->m_colors.reset(new CColorfield(data->m_vertices->size()));
	else
		ensure_single_refered(data->m_colors);

	return data->m_colors->end();

}

int  CTriangleMesh::get_available_data() const
{
	return  (data->m_vertices ? ed_vertex : ed_none) |
		(data->m_normals  ? ed_normal : ed_none) |
		(data->m_scales   ? ed_scale : ed_none)  |
		(data->m_colors   ? ed_color : ed_none);
}

void CTriangleMesh::evaluate_normals()
{
	data->evaluate_normals();
}


struct VertexWithIndex {
	unsigned idx;
	C3DFVector v; 

	VertexWithIndex(const CTriangleMesh& mesh, unsigned i):
		idx(i), 
		v(mesh.vertex_at(i)){
		
	}
}; 

struct compare_vertex  {
	bool operator () (const VertexWithIndex& lhs, const VertexWithIndex& rhs) {
		return (lhs.v.z < rhs.v.z) || 
			((lhs.v.z == rhs.v.z) && ((lhs.v.y < rhs.v.y) ||
						  ((lhs.v.y == rhs.v.y) && (lhs.v.x < rhs.v.x)))); 
	}
}; 


PTriangleMesh EXPORT_MESH get_sub_mesh(const CTriangleMesh& mesh, const vector<unsigned>& triangle_indices) 
{

	auto new_triangles = make_shared<CTriangleMesh::CTrianglefield>(triangle_indices.size()); 
	transform(triangle_indices.begin(),  triangle_indices.end(),  
		  new_triangles->begin(), [mesh](unsigned idx){return mesh.triangle_at(idx);}); 
	
	// collect the vertices 
	set<VertexWithIndex, compare_vertex> vtxset;
	for (auto t = new_triangles->begin(); new_triangles->end() != t; ++t) {
		vtxset.insert(VertexWithIndex(mesh, t->x)); 
		vtxset.insert(VertexWithIndex(mesh, t->y)); 
		vtxset.insert(VertexWithIndex(mesh, t->z)); 
	}

	vector<unsigned> reindex(mesh.vertices_size(), mesh.vertices_size()); 
	unsigned out_index = 0; 

	auto new_vertices = make_shared<CTriangleMesh::CVertexfield>(); 
	new_vertices->reserve(vtxset.size()); 
	
	for (auto iv = vtxset.begin(); iv != vtxset.end(); ++iv) {
		new_vertices->push_back(iv->v); 
		cvdebug() << "vertex[" << out_index << "]=" <<iv->v << "\n"; 
		reindex[iv->idx] = out_index++; 
	}
	
	// re-index the triangles 
	for_each(new_triangles->begin(), new_triangles->end(), [&reindex](CTriangleMesh::triangle_type& t){
			t.x = reindex[t.x]; 
			t.y = reindex[t.y]; 
			t.z = reindex[t.z]; 
			cvdebug() << "triangle:" << t << "\n"; 
		}); 

	auto result = make_shared<CTriangleMesh>(new_triangles, new_vertices);

	if (mesh.get_available_data() & CTriangleMesh::ed_normal) {
		auto in = result->normals_begin(); 
		for (auto iv = vtxset.begin(); iv != vtxset.end(); ++iv, ++in) {
			*in = mesh.normal_at(iv->idx); 
		}
	}

	if (mesh.get_available_data() & CTriangleMesh::ed_scale) {
		auto is = result->scale_begin(); 
		for (auto iv = vtxset.begin(); iv != vtxset.end(); ++iv, ++is) {
			*is = mesh.scale_at(iv->idx); 
		}
	}

	if (mesh.get_available_data() & CTriangleMesh::ed_color) {
		auto ic = result->color_begin(); 
		for (auto iv = vtxset.begin(); iv != vtxset.end(); ++iv, ++ic) {
			*ic = mesh.color_at(iv->idx); 
		}
	}
	
	return result; 
}


const char *CTriangleMesh::data_descr = "mesh";
NS_MIA_END

#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN

template <> const char *  const 
TPluginHandler<CMeshIOPlugin>::m_help =  
   "These plug-ins implement loading and saving of simple triangular meshes from and to various file formats.";

template class TIOPlugin<CTriangleMesh>;
template class TPluginHandler<CMeshIOPlugin>;
template class TIOPluginHandler<CMeshIOPlugin>;
template class THandlerSingleton<TIOPluginHandler<CMeshIOPlugin> >;



NS_MIA_END




