/* -*- mia-c++ -*- 
 *
 * Copyright (c) 2004-2011 
 *
 * Max-Planck-Institute of Evolutionary Anthropology, Leipzg 
 * ETSI Telecomunicacion, UPM, Madrid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <limits>
#include <mia/3d/stackdisttrans.hh>


NS_MIA_BEGIN

using namespace std; 

const float g_far = numeric_limits<float>::max(); 

C2DStackDistanceTransform::C2DStackDistanceTransform()
{
}

C2DStackDistanceTransform::C2DStackDistanceTransform(const C2DBounds& size):
	_M_size(size), 
	_M_k(size.x * size.y, 0), 
	_M_zdt(size.x * size.y)
{
	TRACE_FUNCTION; 
	auto izdt = _M_zdt.begin(); 
	
	SParabola p = {0, 0, -g_far, g_far}; 
	
	for (size_t i = 0; i < _M_zdt.size(); ++i, ++izdt)
		izdt->push_back(p); 
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
	
	if (src->size() != _M_zdt.size()) 
		throw runtime_error("input image has different size!");
	
	auto si = src->begin(); 
	auto ei = src->end(); 
	auto k = _M_k.begin(); 
	auto p = _M_zdt.begin(); 
	
	// for each point of the input mask run the DT(f) algorithm 
	// this takes care of the z-diatance parts 
	while (si != ei) {
		float f = *si ? 0.0f : g_far; 
		
		SParabola& parabola = (*p)[*k];
		
		float s  = d (f, q, parabola.fv, parabola.v);
		while (s <= parabola.z) {
			--(*k);
			parabola = (*p)[*k]; 
			s  = d (f, q, parabola.fv, parabola.v);
		}
		++(*k);
		
		if (*k > p->size()) {
			cverr() << "k = " << *k << " but p->size() = " << p->size() <<"\n"; 
			assert(0 && "can't do");
		}
		
		SParabola new_p = {*k, q, s, f};
		if ( *k == p->size() ) {
			p->push_back(new_p);
		}else {
			(*p)[*k] = new_p; 
			if (*k < p->size() - 1)
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

	if (src.get_size() != _M_size) {
		THROW(invalid_argument, "input image " << s << "has a dffernt size then reference"); 
	}
	
	// evaluate the distance image for this slice
	C2DFImage slice_tmp(_M_size); 
	auto i = slice_tmp.begin(); 
	auto e = slice_tmp.end(); 
	auto p = _M_zdt.begin(); 

	while (i != e) {
		size_t k = 0; 
		
		while ( k < p->size() - 1 && (*p)[ k + 1 ].z  < s) {
			++k; 
		}
		
		float delta = float(s) - (*p)[k].v; 
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
	
	for (size_t y = 0; y < _M_size.y; ++y)
		for (size_t x = 0; x < _M_size.x; ++x, ++iref, ++isrc)
			if (*isrc)
				result.push_back(DistanceFromPoint(C3DBounds(x,y,s), sqrt(*iref))); 
	return result; 
}

bool C2DStackDistanceTransform::save(const string& filename)
{
	return false; 
}
	

float C2DStackDistanceTransform::d(float fq, float q, float fv, float v)const
{
	return ( fq  - fv + q * q - v * v) / (q - v) * 0.5; 
}
	
void C2DStackDistanceTransform::dt1d(vector<float>& r)const
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
		float delta = float(q) - v[k]; 
		r[q] = delta * delta + f[v[k]];
	}
}

void C2DStackDistanceTransform::dt2d(C2DFImage& image)const
{
	TRACE_FUNCTION; 
	vector<float> buffer(image.get_size().x); 
	for (size_t y = 0; y < image.get_size().y; ++y) {
		image.get_data_line_x(y, buffer);
		dt1d(buffer); 
		image.put_data_line_x(y, buffer);
	}
	buffer.resize(image.get_size().y); 
	for (size_t x = 0; x < image.get_size().x; ++x) {
		image.get_data_line_y(x, buffer);
		dt1d(buffer); 
		image.put_data_line_y(x, buffer);
	}
}

NS_MIA_END
