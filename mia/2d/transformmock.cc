/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010, Gert Wollny
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/2d/transformmock.hh>

NS_MIA_BEGIN

C2DTransformMock::C2DTransformMock(const C2DBounds& size):
	m_size(size)
{
	
}

C2DTransformation *C2DTransformMock::clone() const 
{
	return NULL; 
}

bool C2DTransformMock::save(const std::string& , const std::string& ) const 
{
	return false; 
}

P2DTransformation C2DTransformMock::upscale(const C2DBounds&) const 
{
	return P2DTransformation(); 
}

void C2DTransformMock::add(const C2DTransformation&)
{
}

void C2DTransformMock::update(float, const C2DFVectorfield&)
{
}

size_t C2DTransformMock::degrees_of_freedom() const 
{
	return m_size.x * m_size.y; 
}

void C2DTransformMock::set_identity()
{
}

C2DFMatrix C2DTransformMock::derivative_at(int, int) const 
{
	return C2DFMatrix(C2DFVector(1.0, 2.0), C2DFVector(3.0, 4.0)); 
}

C2DFVectorfield C2DTransformMock::translate(const C2DFVectorfield& gradient) const 
{
	return gradient; 
}

float C2DTransformMock::get_max_transform() const 
{
	return sqrt(1.2 * 1.2 +  2.3 * 2.3); 
}

const C2DBounds& C2DTransformMock::get_size() const 
{
	return m_size; 
}

float C2DTransformMock::pertuberate(C2DFVectorfield&) const 
{
	return 1.1; 
}

C2DFVector C2DTransformMock::apply(const C2DFVector& x) const 
{
	if (x.x == 0.0 && x.y == 0.0) 
		return C2DFVector(-1.2, -2.3);
	else 
		return C2DFVector(0.0, 0.0);
}

C2DFVector C2DTransformMock::operator () (const C2DFVector& x) const 
{
	return x - apply(x); 
}

float C2DTransformMock::get_jacobian(const C2DFVectorfield&, float delta) const 
{
	return delta;  
}

P2DImage C2DTransformMock::apply(const C2DImage&, const C2DInterpolatorFactory&) const
{
	return P2DImage(); 
}

NS_MIA_END
