/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010, Gert Wollny
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

#include <mia/internal/autotest.hh>
#include <mia/core/shared_ptr.hh>


#include <mia/2d/costbase.hh>

NS_MIA_USE; 

struct C2DTransformMock: public C2DTransformation {

	C2DTransformMock(const C2DBounds& size); 
	virtual C2DTransformation *clone() const;
	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual P2DTransformation upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual void update(float step, const C2DFVectorfield& a);
	virtual size_t degrees_of_freedom() const;
	virtual void set_identity();
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual C2DFVectorfield translate(const C2DFVectorfield& gradient) const;
	virtual float get_max_transform() const;
	virtual const C2DBounds& get_size() const;
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual C2DFVector apply(const C2DFVector& x) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
private: 
        virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	C2DBounds m_size; 

}; 

struct C2DCostTest: public C2DCostBase {

private: 
	double do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const; 
	
}; 


BOOST_AUTO_TEST_CASE (test_costbase ) 
{
	const C2DBounds size(1,1); 
	C2DCostTest cost; 
	
	C2DFVectorfield force(size); 
	
	C2DTransformMock t(size); 
	
	BOOST_CHECK_EQUAL(cost.evaluate(t, force), 1.0);
	BOOST_CHECK_EQUAL(force(0,0), C2DFVector(1.2, 2.3));

}


double C2DCostTest::do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const
{

	force(0,0) = t(C2DFVector(0.0, 0.0)); 
	return 1.0; 
}


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
