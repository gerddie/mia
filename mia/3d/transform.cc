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
  LatexBeginPluginSection{3D Transformations}
  \label{sec:3dtransforms}
  
  3D transformations provide to means to transform images and point sets. They are mostly used 
  in image registration. 
  
  LatexEnd
*/


#define VSTREAM_DOMAIN "3dtransform"
#include <mia/3d/transform.hh>
#include <mia/3d/deformer.hh>


NS_MIA_BEGIN

C3DTransformation::C3DTransformation(const C3DInterpolatorFactory& ipf):
	Transformation<C3DImage, C3DInterpolatorFactory>(ipf)
{

}

void C3DTransformation::reinit() const
{
}

void C3DTransformation::set_creator_string(const std::string& s)
{
	m_creator_string = s; 
}


const std::string& C3DTransformation::get_creator_string()const
{
	return m_creator_string; 
}

C3DTransformation *C3DTransformation::clone() const
{
	C3DTransformation *result = do_clone(); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result; 
}

P3DTransformation C3DTransformation::upscale(const C3DBounds& size) const
{
	P3DTransformation result = do_upscale(size); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result; 
}



C3DTransformation::iterator_impl::iterator_impl():
	m_pos(0,0,0), 
	m_size(0,0,0)
{
}

C3DTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size):
	m_pos(pos), 
	m_size(size)
{
}

void C3DTransformation::iterator_impl::increment()
{

	if (m_pos.x < m_size.x) {
		++m_pos.x;
		if (m_pos.x < m_size.x) {
			do_x_increment();
			return; 
		}
	} else {
		// if x is at the end, then everything is at the end
		cvwarn() << "C3DTransformation::iterator_impl::increment() past end\n"; 
		return; 
	}
	
	++m_pos.y;
	if (m_pos.y < m_size.y) {
		m_pos.x = 0;
		do_y_increment();
		return; 
	}
	
	++m_pos.z; 
	if (m_pos.z < m_size.z) {
		m_pos.y = 0;
		m_pos.x = 0;
		do_z_increment();
		return; 
	}
}

void C3DTransformation::set_debug()
{
	m_debug = true; 
}
bool C3DTransformation::get_debug()const
{
	return m_debug; 
}


const C3DBounds& C3DTransformation::iterator_impl::get_pos()const
{
	return m_pos; 
}

const C3DBounds& C3DTransformation::iterator_impl::get_size()const
{
	return m_size; 
}


const C3DFVector&  C3DTransformation::iterator_impl::get_value() const
{
	return do_get_value(); 
}

bool C3DTransformation::iterator_impl::operator == (const C3DTransformation::iterator_impl& b) const
{
	assert(m_size == b.m_size); 
	return m_pos == b.m_pos; 
}

EXPORT_3D bool operator == (const C3DTransformation::const_iterator& a, 
			    const C3DTransformation::const_iterator& b)
{
	return *a.m_holder == *b.m_holder; 
}


EXPORT_3D bool operator != (const C3DTransformation::const_iterator& a, 
			    const C3DTransformation::const_iterator& b)
{
	return !(a == b); 
}


C3DTransformation::const_iterator::const_iterator():
	m_holder(NULL)
{
}

C3DTransformation::const_iterator::const_iterator(iterator_impl * holder):
	m_holder(holder)
{
}

C3DTransformation::const_iterator::const_iterator(const const_iterator& other):
	m_holder(other.m_holder->clone())
{
}

C3DTransformation::const_iterator& 
C3DTransformation::const_iterator::operator = (const const_iterator& other)
{
	m_holder.reset(other.m_holder->clone()); 
	return *this; 
}

C3DTransformation::const_iterator& C3DTransformation::const_iterator::operator ++()
{
	m_holder->increment(); 
	return *this; 
}

C3DTransformation::const_iterator C3DTransformation::const_iterator::operator ++(int)
{
	auto old = m_holder->clone(); 
	++(*this); 
	return C3DTransformation::const_iterator(old); 
}

const C3DFVector&  C3DTransformation::const_iterator::operator *() const
{
	assert(m_holder); 
	return m_holder->get_value(); 
}

const C3DFVector  *C3DTransformation::const_iterator::operator ->() const
{
	assert(m_holder); 
	return &m_holder->get_value(); 
}

bool C3DTransformation::refine()
{
	return false; 
}

struct F3DTransform : public TFilter<P3DImage> {
	F3DTransform(const C3DInterpolatorFactory& ipf, const C3DTransformation& trans):
		m_ipf(ipf),
		m_trans(trans){
	}
	template <typename T>
	P3DImage operator ()(const T3DImage<T>& image) const {
		
		auto *timage = new T3DImage<T>(m_trans.get_size(), image);
		P3DImage result(timage);

		std::auto_ptr<T3DInterpolator<T> > interp(m_ipf.create(image.data()));

		auto r = timage->begin();
		auto v = m_trans.begin();
		
		while (r != timage->end()) {
			*r = (*interp)(*v);
			++r; ++v; 
		}
		return result;
	}
private:
	const C3DInterpolatorFactory& m_ipf;
	const C3DTransformation& m_trans;
};



P3DImage C3DTransformation::do_transform(const C3DImage& input, const C3DInterpolatorFactory& ipf) const
{
	return mia::filter(F3DTransform(ipf, *this), input);

}

const char *C3DTransformation::data_descr = "3dtransform";
const char *C3DTransformation::dim_descr = "3d";

NS_MIA_END
