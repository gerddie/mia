/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/core/parallel.hh>

#define VSTREAM_DOMAIN "3dtransform"
#include <mia/3d/transform.hh>
#include <mia/3d/deformer.hh>
#include <mia/core/threadedmsg.hh>


NS_MIA_BEGIN

C3DTransformation::C3DTransformation(const C3DInterpolatorFactory& ipf):
	Transformation<C3DImage, C3DInterpolatorFactory>(ipf), 
	m_debug(false)
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
	m_size(0,0,0), 
	m_start(0,0,0), 
	m_end(0,0,0)

{
}

C3DTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size):
	m_pos(pos), 
	m_size(size), 
	m_start(0,0,0), 
	m_end(size)
{
}

C3DTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
						const C3DBounds& end, const C3DBounds& size):
	m_pos(pos), 
	m_size(size), 
	m_start(begin), 
	m_end(end)
{
	assert(begin.x <= end.x); 
	assert(begin.y <= end.y); 
	assert(begin.z <= end.z);

	assert(pos.x <= end.x); 
	assert(pos.y <= end.y); 
	assert(pos.z <= end.z);

	assert(begin.x <= pos.x); 
	assert(begin.y <= pos.y); 
	assert(begin.z <= pos.z);

	assert(size.x >= end.x); 
	assert(size.y >= end.y); 
	assert(size.z >= end.z);
}

void C3DTransformation::iterator_impl::increment()
{

	if (m_pos.x < m_end.x) {
		++m_pos.x;
		if (m_pos.x < m_end.x) {
			do_x_increment();
			return; 
		}
	} else {
		// if x is at the end, then everything is at the end
		cvwarn() << "C3DTransformation::iterator_impl::increment() past end\n"; 
		return; 
	}
	
	++m_pos.y;
	if (m_pos.y < m_end.y) {
		m_pos.x = m_start.x;
		do_y_increment();
		return; 
	}
	
	++m_pos.z; 
	if (m_pos.z < m_end.z) {
		m_pos.y = m_start.y;
		m_pos.x = m_start.x;
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
	m_holder((iterator_impl *)NULL)
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

const C3DBounds& C3DTransformation::const_iterator::pos() const
{
	assert(m_holder); 
	return m_holder->get_pos(); 
}

const C3DBounds& C3DTransformation::const_iterator::get_size()const
{
	assert(m_holder); 
	return m_holder->get_size(); 
}

C3DFVector C3DTransformation::apply(const C3DFVector& x) const
{
	return get_displacement_at(x); 
}

bool C3DTransformation::refine()
{
	return false; 
}

C3DBounds C3DTransformation::get_minimal_supported_image_size() const
{
	return C3DBounds::_1; 
}

template <typename T> 
struct F3DTransformer {
	F3DTransformer(const C3DTransformation& _trans, 
		       const T3DConvoluteInterpolator<T>& _interp, 
		       T3DImage<T>& _result); 

	void operator() ( const C1DParallelRange& range ) const; 

	const C3DTransformation& trans; 
	T3DImage<T>& result; 
	const T3DConvoluteInterpolator<T>& interp; 
	
}; 


struct F3DTransform : public TFilter<P3DImage> {
	F3DTransform(const C3DInterpolatorFactory& ipf, const C3DTransformation& trans):
		m_ipf(ipf),
		m_trans(trans){
	}
	template <typename T>
	P3DImage operator ()(const T3DImage<T>& image) const {
		
		auto *timage = new T3DImage<T>(m_trans.get_size(), image);
		P3DImage result(timage);
		
		if (m_trans.has_attribute(C3DTransformation::input_spacing_attr) && 
		    m_trans.has_attribute(C3DTransformation::output_spacing_attr)) {
			C3DFVector in_voxel = m_trans.get_attribute_as<C3DFVector>(C3DTransformation::input_spacing_attr);
			C3DFVector out_voxel = m_trans.get_attribute_as<C3DFVector>(C3DTransformation::output_spacing_attr);
			if (image.get_voxel_size() != in_voxel) {
				cvwarn() << "C3DTransform: your input image voxel spacing [" 
					 << image.get_voxel_size() << "] differs from voxel spacing, this transformation expects [" 
					 << in_voxel << "], output voxel spacing not reliable\n"; 
			}
			timage->set_voxel_size(out_voxel); 
		}

		std::unique_ptr<T3DConvoluteInterpolator<T> > interp(m_ipf.create(image.data()));

		F3DTransformer<T> worker(m_trans, *interp, *timage); 
		pfor(C1DParallelRange( 0, timage->get_size().z), worker);

		// if the transformation provides a forced output pixel spacing, 
		// set it here 
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


template <typename T> 
F3DTransformer<T>::F3DTransformer(const C3DTransformation& _trans, 
			       const T3DConvoluteInterpolator<T>& _interp, 
			       T3DImage<T>& _result): 
	trans(_trans), 
	result(_result), 
	interp(_interp)
{

}

template <typename T> 
void F3DTransformer<T>::operator() ( const C1DParallelRange& range ) const
{
	CThreadMsgStream thread_stream;
	auto cache = interp.create_cache(); 
	
	C3DBounds begin(0,0,range.begin()); 
	C3DBounds end(result.get_size().x,result.get_size().y, range.end());

	auto r = result.begin_at(0,0,range.begin()); 
	
	cvdebug() << "range = " << begin << " - " << end << "\n"; 

	auto ti = trans.begin_range(begin, end); 
	auto te = trans.end_range(begin, end); 

	while (ti != te) {
		*r = interp(*ti, cache); 
		++ti; 
		++r; 
	}
}

const char *C3DTransformation::data_descr = "3dtransform";
const char *C3DTransformation::dim_descr = "3dimage";

NS_MIA_END
