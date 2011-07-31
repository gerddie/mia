/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

/*
  LatexBeginPluginSection{2D Transformations}
  \label{sec:2dtransforms}
  
  2D transformations provide to means to transform images and point sets. They are mostly used 
  in image registration. 
  
  LatexEnd
*/


#include <mia/2d/transform.hh>
#include <mia/2d/deformer.hh>

NS_MIA_BEGIN

C2DTransformation::C2DTransformation(const C2DInterpolatorFactory& ipf):
	Transformation<C2DImage, C2DInterpolatorFactory>(ipf)
{

}

void C2DTransformation::reinit() const
{
}

void C2DTransformation::set_creator_string(const std::string& s)
{
	m_creator_string = s; 
}


const std::string& C2DTransformation::get_creator_string()const
{
	return m_creator_string; 
}

C2DTransformation *C2DTransformation::clone() const
{
	C2DTransformation *result = do_clone(); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result; 
}

C2DTransformation::Pointer C2DTransformation::upscale(const C2DBounds& size) const
{
	Pointer result = do_upscale(size); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result;	
}


C2DTransformation::iterator_impl::iterator_impl():
	m_pos(0,0), 
	m_size(0,0)
{
}

C2DTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size):
	m_pos(pos), 
	m_size(size)
{
}

void C2DTransformation::iterator_impl::increment()
{
	++m_pos.x;
	if (m_pos.x < m_size.x)
		do_x_increment();
	else {
		if (m_pos.y < m_size.y) {
			++m_pos.y;
			if (m_pos.y < m_size.y)
				m_pos.x = 0;
			else 
				return ; 
			do_y_increment();
		}
	}
}

void C2DTransformation::iterator_impl::advance(unsigned int delta)
{
	unsigned int delta_x_max = m_size.x - m_pos.x; 
	if (delta < delta_x_max) {
		m_pos.x += delta; 
		do_x_increment();
		return; 
	}
	++m_pos.y; 
	delta -= delta_x_max; 
	m_pos.x = 0; 
	
	if (delta) {
		m_pos.x += delta % m_size.x; 
		m_pos.y += delta / m_size.x;
	}
	if (m_pos.y < m_size.y) 
		do_y_increment();
#ifndef NDEBUG
	else if (m_pos.y == m_size.y) 
		cvdebug() << "C2DTransformation::iterator_impl::advance: iterator at end\n"; 
	else 
		cvwarn() << "C2DTransformation::iterator_impl::advance: iterator past end\n"; 
#endif	
}


C2DTransformation::const_iterator& C2DTransformation::const_iterator::operator += (unsigned int delta)
{
	m_holder->advance(delta); 
	return *this; 
}

void C2DTransformation::const_iterator::print(ostream& os) const 
{
	m_holder->print(os); 
}

const C2DBounds& C2DTransformation::iterator_impl::get_pos()const
{
	return m_pos; 
}

void C2DTransformation::iterator_impl::print(ostream& os) const 
{
	os << "Iterator[pos=" << m_pos << " of " << m_size <<"]"; 
}

const C2DBounds& C2DTransformation::iterator_impl::get_size()const
{
	return m_size; 
}


const C2DFVector&  C2DTransformation::iterator_impl::get_value() const
{
	return do_get_value(); 
}

bool C2DTransformation::iterator_impl::operator == (const C2DTransformation::iterator_impl& b) const
{
	assert(m_size == b.m_size); 
	return m_pos == b.m_pos || (m_pos.y == m_size.y && b.m_pos.y == b.m_size.y); 
}

EXPORT_2D bool operator == (const C2DTransformation::const_iterator& a, 
			    const C2DTransformation::const_iterator& b)
{
	return *a.m_holder == *b.m_holder; 
}


EXPORT_2D bool operator != (const C2DTransformation::const_iterator& a, 
			    const C2DTransformation::const_iterator& b)
{
	return !(a == b); 
}


C2DTransformation::const_iterator::const_iterator():
	m_holder(NULL)
{
}

C2DTransformation::const_iterator::const_iterator(iterator_impl * holder):
	m_holder(holder)
{
}

C2DTransformation::const_iterator::const_iterator(const const_iterator& other):
	m_holder(other.m_holder->clone())
{
}

C2DTransformation::const_iterator& 
C2DTransformation::const_iterator::operator = (const const_iterator& other)
{
	m_holder.reset(other.m_holder->clone()); 
	return *this; 
}

C2DTransformation::const_iterator& C2DTransformation::const_iterator::operator ++()
{
	m_holder->increment(); 
	return *this; 
}

C2DTransformation::const_iterator C2DTransformation::const_iterator::operator ++(int)
{
	auto old = m_holder->clone(); 
	++(*this); 
	return C2DTransformation::const_iterator(old); 
}

const C2DFVector&  C2DTransformation::const_iterator::operator *() const
{
	assert(m_holder); 
	return m_holder->get_value(); 
}

const C2DFVector  *C2DTransformation::const_iterator::operator ->() const
{
	assert(m_holder); 
	return &m_holder->get_value(); 
}

bool C2DTransformation::refine()
{
	return false; 
}


/**
   @brief Helper functor for 2D image transformations 
   
   Helper Functor to evaluate a transformed image by applying a given 
   transformation and using the provided interpolator type
*/

struct F2DTransform : public TFilter<P2DImage> {

	/**
	   Construtor 
	   @param ipf interpolation factory to use 
	   @param trans tranformation to be applied 
	 */
	F2DTransform(const C2DInterpolatorFactory& ipf, const C2DTransformation& trans):
		m_ipf(ipf),
		m_trans(trans)
		{
		}
	
	template <typename T>
	P2DImage operator ()(const T2DImage<T>& image) const {
		T2DImage<T> *timage = new T2DImage<T>(m_trans.get_size(), image);
		
		unique_ptr<T2DInterpolator<T>> interp(m_ipf.create(image.data()));
		
		auto r = timage->begin();
		auto v = m_trans.begin();
		
		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
				*r = (*interp)(*v);
			}
		return P2DImage(timage);
	}
private:
	const C2DInterpolatorFactory& m_ipf;
	const C2DTransformation& m_trans;
};

P2DImage C2DTransformation::do_transform(const C2DImage& image, const C2DInterpolatorFactory& ipf) const
{
	return mia::filter(F2DTransform(ipf, *this), image);
}


const char *C2DTransformation::data_descr = "2dtransform";
const char *C2DTransformation::dim_descr = "2d"; 

NS_MIA_END
