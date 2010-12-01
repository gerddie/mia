/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#include <gsl++/vector.hh>

#include <mia/2d/transformmock.hh>

NS_MIA_BEGIN

C2DTransformMock::C2DTransformMock()
{
}

C2DTransformMock::C2DTransformMock(const C2DBounds& size):
	m_size(size)
{

}

C2DTransformation *C2DTransformMock::do_clone() const
{
	return NULL;
}

C2DTransformation *C2DTransformMock::invert() const
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
	return 2 * m_size.x * m_size.y;
}

void C2DTransformMock::set_identity()
{
}

C2DFMatrix C2DTransformMock::derivative_at(int, int) const
{
	return C2DFMatrix(C2DFVector(1.0, 2.0), C2DFVector(3.0, 4.0));
}

void C2DTransformMock::translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const
{
	for(auto f = gradient.begin(), r = params.begin(); f != gradient.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
	}
}

float C2DTransformMock::get_max_transform() const
{
	return sqrt(1.2 * 1.2 +  2.3 * 2.3);
}

const C2DBounds& C2DTransformMock::get_size() const
{
	return m_size;
}

gsl::DoubleVector  C2DTransformMock::get_parameters() const
{
	gsl::DoubleVector result(degrees_of_freedom());
	if (degrees_of_freedom() > 2) 
		result[1] = -2.3;
	if (degrees_of_freedom() > 1) 
		result[0] = -1.2;
	return result;
}

void C2DTransformMock::set_parameters(const gsl::DoubleVector& /*params*/)
{

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

P2DImage C2DTransformMock::apply(const C2DImage& src, const C2DInterpolatorFactory&) const
{
	return P2DImage(src.clone());
}

float C2DTransformMock::divergence() const
{
	return 1.0f;
}

float C2DTransformMock::curl() const
{
	return 2.0f;
}

double C2DTransformMock::get_divcurl_cost(double wd, double wr, gsl::DoubleVector& gradient) const
{
	gradient[0] = wd; 
	gradient[1] = wr; 
	return wd + wr; 
}

double C2DTransformMock::get_divcurl_cost(double wd, double wr) const
{
	return wd + wr; 
}


C2DTransformation::const_iterator C2DTransformMock::begin() const
{
	return C2DTransformation::const_iterator(new iterator_impl(C2DBounds(0,0), m_size)); 
}

C2DTransformation::const_iterator C2DTransformMock::end() const
{
	return C2DTransformation::const_iterator(new iterator_impl(m_size, m_size)); 
}

C2DTransformation::iterator_impl *C2DTransformMock::iterator_impl::clone()const
{
	return new C2DTransformMock::iterator_impl(get_pos(), get_size()); 
}

C2DTransformMock::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size):
	C2DTransformation::iterator_impl(pos, size), 
	_M_value(-1.2, -2.3)
{
}

const C2DFVector& C2DTransformMock::iterator_impl::do_get_value()const
{
	return _M_value; 
}

void C2DTransformMock::iterator_impl::do_x_increment()
{
}

void C2DTransformMock::iterator_impl::do_y_increment()
{
}




NS_MIA_END
