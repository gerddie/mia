/* -*- mona-c++  -*-
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "TRANSFORMS"

#include <mia/2d/transform.hh>
#include <mia/internal/autotest.hh>

NS_MIA_USE; 

class Cost2DMock {
public: 
	Cost2DMock(const C2DBounds& size); 
	double value(const C2DTransformation& t) const;  
	double value_and_gradient(const C2DTransformation& t, C2DFVectorfield& gradient) const;
	
	double src_value(const C2DFVector& x)const; 
	double ref_value(const C2DFVector& x)const; 
	C2DFVector src_grad(const C2DFVector& x)const; 
	C2DBounds _M_size; 
	C2DFVector _M_center; 
	float _M_r; 
}; 



BOOST_AUTO_TEST_CASE (selftest_Cost2DMock) 
{
	C2DBounds size(20,20); 
	Cost2DMock cm(size); 
	
	C2DFVector x(11.0,11.0); 
	C2DFVector dx(.001,0.0); 
	C2DFVector dy(.0,0.001); 
	
	C2DFVector grad = cm.src_grad(x);
	
	C2DFVector fdgrad( (cm.src_value(x + dx) - cm.src_value(x - dx)) / 0.002, 
			   (cm.src_value(x + dy) - cm.src_value(x - dy)) / 0.002); 
	
	BOOST_CHECK_CLOSE(grad.x , fdgrad.x, 0.1); 
	BOOST_CHECK_CLOSE(grad.y , fdgrad.y, 0.1); 
	
}




Cost2DMock::Cost2DMock(const C2DBounds& size):
	_M_size(size), 
	_M_center(0.5 * size.x, 0.5 * size.y),
	_M_r(sqrt(_M_center.x * _M_center.x + _M_center.y * _M_center.y))
{
}
	
double Cost2DMock::value(const C2DTransformation& t) const
{
	assert(_M_size == t.get_size()); 
	double result = 0.0; 
	auto it = t.begin(); 
	for (size_t y = 0; y < _M_size.y; ++y) 
		for (size_t x = 0; x < _M_size.x; ++x, ++it) {
			double v = src_value(*it) - ref_value(C2DFVector(x,y)); 
			result += v * v; 
		}
	return result; 
		
}

double Cost2DMock::value_and_gradient(const C2DTransformation& t, C2DFVectorfield& gradient) const
{
	assert(_M_size == t.get_size()); 
	assert(gradient.get_size() == _M_size); 
	
	double result = 0.0; 
	auto it = t.begin(); 
	auto ig = gradient.begin(); 
	for (size_t y = 0; y < _M_size.y; ++y) 
		for (size_t x = 0; x < _M_size.x; ++x, ++it, ++ig) {
			double v = src_value(*it) - ref_value(C2DFVector(x,y)); 
			result += v * v; 
			*ig = 2.0 * v * src_grad(*it);  
		}
	return result; 
}

double Cost2DMock::src_value(const C2DFVector& x)const
{
	const C2DFVector p = x - _M_center; 
	return exp( - (p.x * p.x + p.y * p.y) / _M_r); 
}

C2DFVector Cost2DMock::src_grad(const C2DFVector& x)const
{
	
	return - 2.0f / _M_r * (x-_M_center) * src_value(x); 
}

double Cost2DMock::ref_value(const C2DFVector& x)const 
{
	const C2DFVector p = x - _M_center - C2DFVector(1.0,1.0); 
	return exp( - (p.x * p.x + p.y * p.y) / _M_r); 
}




