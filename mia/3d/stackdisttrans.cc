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

#define VSTREAM_DOMAIN "StackDT"

#include <limits>
#include <mia/3d/stackdisttrans.hh>

#include <mia/core/export_handler.hh>

#include <mia/core/errormacro.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>


NS_MIA_BEGIN

using namespace std; 

const float g_far = numeric_limits<float>::max(); 

const char * const C2DStackDistanceTransform::type_descr = "dt"; 
const char * const C2DStackDistanceTransform::data_descr = "stack2d"; 


C2DStackDistanceTransform *C2DStackDistanceTransform::clone() const 
{
	return new C2DStackDistanceTransform(*this); 
}

C2DStackDistanceTransform::C2DStackDistanceTransform(const C2DImage& slice, const C3DFVector& voxel_size):
	m_size(slice.get_size()), 
	m_voxel_size(voxel_size),
	m_k(m_size.x * m_size.y, 0), 
	m_zdt(m_size.x * m_size.y)
{
	TRACE_FUNCTION; 

	const C2DBitImage *src = dynamic_cast<const C2DBitImage*>(&slice); 
	if (!src) 
		throw runtime_error("input image is not a bit image!");
	
	auto izdt = m_zdt.begin(); 
	
	SParabola p = {0, 0, -g_far, g_far}; 
	auto pixel = src->begin(); 
	for (size_t i = 0; i < m_zdt.size(); ++i, ++izdt, ++pixel) {
		p.fv = *pixel ? 0.0f : g_far; 
		izdt->push_back(p); 
	}
}

/*
  Implements Algorithm DT(f) of the paper 
*/ 
void C2DStackDistanceTransform::read( const C2DImage& slice, int q)
{
	TRACE_FUNCTION; 
	const C2DBitImage *src = dynamic_cast<const C2DBitImage*>(&slice); 
	if (!src) 
		throw runtime_error("input image is not a bit image!");
	
	if (src->size() != m_zdt.size()) 
		throw runtime_error("input image has different size!");
	
	C2DFVector pixel_size = slice.get_pixel_size(); 
	C2DFVector predev_pixel(m_voxel_size.x,  m_voxel_size.y); 
	if ( pixel_size != predev_pixel) 
		cvwarn() << "Input image voxel size and predefined voxel size differ: " 
			 << pixel_size  <<  " != " << predev_pixel
			 << " the latter takes precedence \n"; 
	
	auto si = src->begin(); 
	auto ei = src->end(); 
	auto k = m_k.begin(); 
	auto p = m_zdt.begin(); 
	
	// for each point of the input mask run the DT(f) algorithm 
	// this takes care of the z-diatance parts 
	while (si != ei) {
		float f = *si ? 0.0f : g_far; 
		
		SParabola& parabola = (*p)[*k];
		
		float s  = d (f, q, parabola.fv, parabola.v);
		while (s <= parabola.z) {
			--(*k);
			assert(*k >= 0); 
			parabola = (*p)[*k]; 
			s  = d (f, q, parabola.fv, parabola.v);
		}
		
		++(*k);
		
		const int psize = p->size(); 
		if ( *k > psize ) {
			cverr() << "k = " << *k << " but p->size() = " << p->size() <<"\n"; 
			assert(0 && "can't do");
		}
		
		SParabola new_p = {*k, q, s, f};
		if ( *k == psize ) {
			p->push_back(new_p);
		}else {
			(*p)[*k] = new_p; 
			if ( *k < psize - 1 )
				p->resize(*k + 1); 
		}
		++si; 
		++k; 
		++p; 
	}

}
	

vector<C2DStackDistanceTransform::DistanceFromPoint> 
C2DStackDistanceTransform::get_slice(size_t s, const C2DImage& image) const
{
	TRACE_FUNCTION; 
	
	vector<C2DStackDistanceTransform::DistanceFromPoint>  result; 
	auto src = dynamic_cast<const C2DBitImage&>(image); 

	if (src.get_size() != m_size) {
		throw create_exception<invalid_argument>( "input image nr ", s, "has a dffernt size ", src.get_size(), 
						" then reference ", m_size); 
	}

	C2DFVector pixel_size = image.get_pixel_size(); 
	C2DFVector predev_pixel(m_voxel_size.x,  m_voxel_size.y); 
	if ( pixel_size != predev_pixel) 
		cvwarn() << "Input image voxel size and predefined voxel size differ: " 
			 << pixel_size  <<  " != " << predev_pixel
			 << " the latter takes precedence \n"; 
	
	// evaluate the distance image for this slice
	C2DFImage slice_tmp(m_size); 
	auto i = slice_tmp.begin(); 
	auto e = slice_tmp.end(); 
	auto p = m_zdt.begin(); 

	while (i != e) {
		size_t k = 0; 
		
		while ( k < p->size() - 1 && (*p)[ k + 1 ].z  < s) {
			++k; 
		}
		
		float delta = (float(s) - (*p)[k].v) * m_voxel_size.z; 
		*i = delta * delta + (*p)[k].fv;
		++i; 
		++p; 
	}
	
	// run the 2D distance transform folding operation 
	dt2d(slice_tmp); 

	// now for each point in the input that is not zero, evaluate the 
	// distance to the original mask 
	auto isrc = src.begin(); 
	auto iref = slice_tmp.begin(); 
	
	for (size_t y = 0; y < m_size.y; ++y)
		for (size_t x = 0; x < m_size.x; ++x, ++iref, ++isrc)
			if (*isrc)
				result.push_back(DistanceFromPoint(C3DBounds(x,y,s), sqrt(*iref))); 
	return result; 
}

bool C2DStackDistanceTransform::save(const string& /*filename*/)
{
	return false; 
}
	

float C2DStackDistanceTransform::d(float fq, float q, float fv, float v)const
{
	return ( fq  - fv + q * q - v * v) / (q - v) * 0.5; 
}
	
void C2DStackDistanceTransform::dt1d(vector<float>& r, float scale)const
{
	TRACE_FUNCTION; 
	vector<float> f(r); 
	vector<int> v(f.size()); 
	vector<float> z(f.size() + 1); 

	int k = 0;
	
	v[0] = 0;
	z[0] = -numeric_limits<float>::max();
	z[1] = +numeric_limits<float>::max();
	
	for (size_t q = 1; q < f.size(); q++) {
		float s  = d(f[q], q, f[v[k]], v[k]);
		while (s <= z[k]) {
			--k;
			s  = d(f[q], q, f[v[k]], v[k]);
		}
		++k;
		v[k] = q;
		z[k] = s;
		z[k+1] = numeric_limits<float>::max();
	}

	k = 0;
	for (size_t q = 0; q < f.size(); ++q) {
		while (z[k+1] < q)
			++k;
		float delta = (float(q) - v[k]) * scale; 
		r[q] = delta * delta + f[v[k]];
	}
}

void C2DStackDistanceTransform::dt2d(C2DFImage& image)const
{
	TRACE_FUNCTION; 
	vector<float> buffer(image.get_size().x); 
	for (size_t y = 0; y < image.get_size().y; ++y) {
		image.get_data_line_x(y, buffer);
		dt1d(buffer, m_voxel_size.x); 
		image.put_data_line_x(y, buffer);
	}
	buffer.resize(image.get_size().y); 
	for (size_t x = 0; x < image.get_size().x; ++x) {
		image.get_data_line_y(x, buffer);
		dt1d(buffer, m_voxel_size.y); 
		image.put_data_line_y(x, buffer);
	}
}

template<> const  char * const 
TPluginHandler<C2DStackDistanceTransformIO>::m_help = 
   "These plug-ins provide support for loading and storing the results of a distance transform of a stack of 2D images." 
; 

template class TIOPlugin<C2DStackDistanceTransform>;
template class THandlerSingleton<TIOPluginHandler<C2DStackDistanceTransformIO> >;
template class TIOHandlerSingleton<TIOPluginHandler<C2DStackDistanceTransformIO> >;
template class TIOPluginHandler<C2DStackDistanceTransformIO>;
template class TPluginHandler<C2DStackDistanceTransformIO>;

NS_MIA_END
