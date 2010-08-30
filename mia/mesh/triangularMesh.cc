/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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

// $Id: triangularMesh.cc 952 2006-07-13 12:36:59Z write1 $

/*! \brief implemetation of class for triangular meshes

\todo add a disance transform to evaluate the distance between two meshes
\file triangularMesh.cc
\author Gert Wollny <wollny@cbs.mpg.de>

*/

#include <mia/core/export_handler.hh>

#define _USE_MATH_DEFINES
#include <cmath>
#include <mia/mesh/triangularMesh.hh>


NS_MIA_BEGIN

using namespace std;
using namespace boost;

struct CTriangleMeshData {

	CTriangleMesh::PTrianglefield  _M_triangles ;
	CTriangleMesh::PVertexfield    _M_vertices;
	CTriangleMesh::PNormalfield    _M_normals;
	CTriangleMesh::PScalefield     _M_scales;
	CTriangleMesh::PColorfield     _M_colors;

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
	_M_triangles(new CTriangleMesh::CTrianglefield(n_triangles)),
	_M_vertices(new CTriangleMesh::CVertexfield(n_vertices))
{
}

CTriangleMeshData::CTriangleMeshData(CTriangleMesh::PTrianglefield triangles,
				     CTriangleMesh::PVertexfield vertices,
				     CTriangleMesh::PNormalfield normals,
				     CTriangleMesh::PColorfield colors,
				     CTriangleMesh::PScalefield scales):
	_M_triangles(triangles),
	_M_vertices(vertices),
	_M_normals(normals),
	_M_scales(scales),
	_M_colors(colors)
{
	assert(_M_triangles && _M_vertices);
}

CTriangleMeshData::CTriangleMeshData(CTriangleMesh::PTrianglefield  triangles, int n_vertices):
	_M_triangles(triangles),
	_M_vertices(new CTriangleMesh::CVertexfield(n_vertices))
{
}

CTriangleMeshData::CTriangleMeshData(const CTriangleMeshData& orig):
	_M_triangles(orig._M_triangles),
	_M_vertices(orig._M_vertices),
	_M_normals(orig._M_normals),
	_M_scales(orig._M_scales),
	_M_colors(orig._M_colors)
{
	assert(_M_triangles && _M_vertices);
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
	assert(_M_vertices && _M_triangles);
	cvdebug() << "CTriangleMeshData::evaluate_normals()\n";

	int errors = 0;
	if (!_M_normals)
		// if no normals are available yet, created them
		_M_normals.reset(new CTriangleMesh::CNormalfield(_M_vertices->size()));
	else
		// before overwriting the normales, make sure we work on this set
		ensure_single_refered(_M_normals);
	// zero out normals
	_M_normals->clear();

	// the writable iterators, index operators  do check against multiple referencing of the data.
	// We only need read access, therefore a const reference makes sure,
	// the iterators are created from the const function
	const CTriangleMesh::CTrianglefield&  ctriangles = *_M_triangles;
	//const CTriangleMesh::vertexfield&    cvertices = *_M_vertices;

	cvdebug() << "Mesh has " << ctriangles.size() << " triangles\n";
	// run overall triangles
	CTriangleMesh::const_triangle_iterator t = ctriangles.begin();
	CTriangleMesh::const_triangle_iterator et = ctriangles.end();

	while (t != et) {
		C3DFVector e1 = (*_M_vertices)[t->x] - (*_M_vertices)[t->y];
		C3DFVector e2 = (*_M_vertices)[t->z] - (*_M_vertices)[t->y];
		C3DFVector e3 = (*_M_vertices)[t->z] - (*_M_vertices)[t->x];
		C3DFVector help_normal = e2 ^ e1;
		if (help_normal.norm2() == 0 && errors < 10) {
			cverr() <<"CTriangleMeshData::evaluate_normals(): triangle "<< *t << " has zero normal\n";
			++errors;
			continue;
		}
		float weight1 = acos((e1 * e2) / (e1.norm() * e2.norm()));
		float weight2 = acos((e3 * e2) / (e3.norm() * e2.norm()));
		(*_M_normals)[t->y] += weight1 * help_normal;
		(*_M_normals)[t->z] += weight2 * help_normal;

		float weight3 = M_PI - weight1  - weight2;

		(*_M_normals)[t->x] += weight3 * help_normal;
		++t;
	}

	// normalize the normals
	CTriangleMesh::normal_iterator bn = _M_normals->begin();
	CTriangleMesh::normal_iterator en = _M_normals->end();

	while (bn != en) {
		float norm = (*bn).norm();
		if (norm > 0)
			*bn /= norm;
		++bn;
	}
}
// void CTriangleMeshData::evaluate_normals()
//


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

CTriangleMesh::CTriangleMesh(PTrianglefield triangles, PVertexfield vertices):
	data(new CTriangleMeshData(triangles, vertices, PNormalfield(), PColorfield(), PScalefield()))
{
}

CTriangleMesh CTriangleMesh::clone_connectivity()const
{
	PVertexfield vertices(new CVertexfield(vertices_size()));
	CTriangleMesh result(data->_M_triangles, vertices);
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
	return data->_M_vertices ? &(*data->_M_vertices)[0].x : NULL;
}

const void *CTriangleMesh::get_normal_pointer()const
{
	return data->_M_normals ? &(*data->_M_normals)[0].x : NULL;
}

const void *CTriangleMesh::get_color_pointer()const
{
	return data->_M_colors ? &(*data->_M_colors)[0].x : NULL;
}


const void *CTriangleMesh::get_triangle_pointer()const
{
	return  data->_M_triangles ? &(*data->_M_triangles)[0].x : NULL;
}

 unsigned int CTriangleMesh::triangle_size()const
{
	return data->_M_triangles->size();
}

unsigned int CTriangleMesh::vertices_size()const
{
	return data->_M_vertices->size();
}

const CTriangleMesh::triangle_type&    CTriangleMesh::triangle_at(unsigned int i)const
{
	assert(i < data->_M_triangles->size());
	return (*data->_M_triangles)[i];
}

const CTriangleMesh::vertex_type&      CTriangleMesh::vertex_at(unsigned int i)const
{
	assert(i < data->_M_vertices->size());
	return (*data->_M_vertices)[i];
}

const CTriangleMesh::normal_type&      CTriangleMesh::normal_at(unsigned int i)const
{
	assert(i < data->_M_vertices->size() && data->_M_normals);
	return (*data->_M_normals)[i];
}

const CTriangleMesh::scale_type&      CTriangleMesh::scale_at(unsigned int i)const
{
	assert(i < data->_M_vertices->size() && data->_M_scales);
	return (*data->_M_scales)[i];
}

const CTriangleMesh::color_type&       CTriangleMesh::color_at(unsigned int i)const
{
	assert(i < data->_M_vertices->size() && data->_M_colors);
	return (*data->_M_colors)[i];

}

CTriangleMesh::const_triangle_iterator CTriangleMesh::triangles_begin() const
{
	const CTrianglefield& c = *data->_M_triangles;
	return c.begin();
}

CTriangleMesh::const_triangle_iterator CTriangleMesh::triangles_end() const
{
	const CTrianglefield& c = *data->_M_triangles;
	return c.end();
}


CTriangleMesh::triangle_iterator CTriangleMesh::triangles_begin()
{
	ensure_single_refered(data->_M_triangles);
	return data->_M_triangles->begin();
}

CTriangleMesh::triangle_iterator CTriangleMesh::triangles_end()
{
	ensure_single_refered(data->_M_triangles);
	return data->_M_triangles->end();
}

CTriangleMesh::const_vertex_iterator CTriangleMesh::vertices_begin()const
{
	const CVertexfield& c = *data->_M_vertices;
	return c.begin();
}

CTriangleMesh::const_vertex_iterator CTriangleMesh::vertices_end()const
{
	const CVertexfield& c = *data->_M_vertices;
	return c.end();
}

CTriangleMesh::vertex_iterator CTriangleMesh::vertices_begin()
{
	ensure_single_refered(data->_M_vertices);
	return data->_M_vertices->begin();
}

CTriangleMesh::vertex_iterator CTriangleMesh::vertices_end()
{
	ensure_single_refered(data->_M_vertices);
	return data->_M_vertices->end();
}

CTriangleMesh::const_normal_iterator CTriangleMesh::normals_begin()const
{
	if (!data->_M_normals)
		data->evaluate_normals();

	const CNormalfield& c = *data->_M_normals;
	return c.begin();
}

CTriangleMesh::const_normal_iterator CTriangleMesh::normals_end()const
{
	if (!data->_M_normals)
		data->evaluate_normals();

	const CNormalfield& c = *data->_M_normals;
	return c.end();
}

CTriangleMesh::normal_iterator CTriangleMesh::normals_begin()
{
	if (!data->_M_normals)
		data->_M_normals.reset(new CTriangleMesh::CNormalfield(data->_M_vertices->size()));
	else
		ensure_single_refered(data->_M_normals);


	return data->_M_normals->begin();
}

CTriangleMesh::normal_iterator CTriangleMesh::normals_end()
{
	if (!data->_M_normals)
		data->_M_normals.reset(new CTriangleMesh::CNormalfield(data->_M_vertices->size()));
	else
		ensure_single_refered(data->_M_normals);

	return data->_M_normals->begin();
}

CTriangleMesh::const_scale_iterator CTriangleMesh::scale_begin()const
{
	if (!data->_M_scales)
		return const_scale_iterator();

	const CScalefield& c = *data->_M_scales;
	return c.begin();
}

CTriangleMesh::const_scale_iterator CTriangleMesh::scale_end()const
{
	if (!data->_M_scales)
		return const_scale_iterator();

	const CScalefield& c = *data->_M_scales;
	return c.end();
}

CTriangleMesh::scale_iterator       CTriangleMesh::scale_begin()
{
	if (!data->_M_scales)
		data->_M_scales.reset(new CScalefield(data->_M_vertices->size()));
	else
		ensure_single_refered(data->_M_scales);
	return data->_M_scales->begin();
}

CTriangleMesh::scale_iterator       CTriangleMesh::scale_end()
{
	if (!data->_M_scales)
		data->_M_scales.reset(new CScalefield(data->_M_vertices->size()));
	else
		ensure_single_refered(data->_M_scales);

	return data->_M_scales->end();
}

CTriangleMesh::const_color_iterator CTriangleMesh::color_begin()const
{
	if (!data->_M_colors)
		return const_color_iterator();

	const CColorfield& c = *data->_M_colors;
	return c.begin();
}

CTriangleMesh::const_color_iterator CTriangleMesh::color_end()const
{
	if (!data->_M_colors)
		return const_color_iterator();

	const CColorfield& c = *data->_M_colors;
	return c.end();
}

CTriangleMesh::color_iterator       CTriangleMesh::color_begin()
{
	if (!data->_M_colors)
		data->_M_colors.reset(new CColorfield(data->_M_vertices->size()));
	else
		ensure_single_refered(data->_M_colors);

	return data->_M_colors->begin();
}

CTriangleMesh::color_iterator       CTriangleMesh::color_end()
{
	if (!data->_M_colors)
		data->_M_colors.reset(new CColorfield(data->_M_vertices->size()));
	else
		ensure_single_refered(data->_M_colors);

	return data->_M_colors->end();

}

int  CTriangleMesh::get_available_data() const
{
	return  (data->_M_vertices ? ed_vertex : ed_none) |
		(data->_M_normals  ? ed_normal : ed_none) |
		(data->_M_scales   ? ed_scale : ed_none)  |
		(data->_M_colors   ? ed_color : ed_none);
}

void CTriangleMesh::evaluate_normals()
{
	data->evaluate_normals();
}

const char *io_mesh_type::type_descr = "mesh";
NS_MIA_END

#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN

template class TIOPlugin<io_mesh_type>;
template class TIOPluginHandler<CMeshIOPlugin>;
template class THandlerSingleton<TIOPluginHandler<CMeshIOPlugin> >;
template class TIOHandlerSingleton<TIOPluginHandler<CMeshIOPlugin> >;

NS_MIA_END




