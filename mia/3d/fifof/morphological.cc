/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef __GCC__
#define PRIVATE
#else
#define PRIVATE __attribute__((visibility("hidden")))
#endif

#include <mia/core/type_traits.hh>
#include <mia/3d/fifof/morphological.hh>

NS_MIA_USE
using namespace std;
using namespace boost;

namespace morphological_fifof {

template <template <typename, bool> class Compare>
C2DMorphFifoFilter<Compare>::C2DMorphFifoFilter(P3DShape shape):
	C2DImageFifoFilter(shape->get_size().z, shape->get_size().z / 2 + 1 ,0),
	m_shape(shape)
{
	TRACE("C2DMorphFifoFilter<Compare>::C2DMorphFifoFilter(P3DShape shape)");
}

template <template <typename, bool> class Compare>
template <typename T>
C2DImage *C2DMorphFifoFilter<Compare>::operator()(const T2DImage<T>& input)
{
	TRACE("C2DMorphFifoFilter<Compare>::push internal");

	T3DImage<T> *buf = dynamic_cast<T3DImage<T> *> (m_buffer.get());
	assert(buf);

	copy(input.begin(), input.end(), buf->begin());
	return NULL;
}

template <template <typename, bool> class Compare>
template <typename T>
C2DImage *C2DMorphFifoFilter<Compare>::operator()(const T3DImage<T>& input) const
{
	TRACE("C2DMorphFifoFilter<Compare>::operator()(const T3DImage<T>& input) const");
	T2DImage<T> *result = new T2DImage<T>(m_slice_size);
	size_t read_slice = m_shape->get_size().z / 2;
	const bool is_float = is_floating_point<T>::value;
	typedef Compare<T, is_float> cmp;

	typename T2DImage<T>::iterator i = result->begin();

	C3DShape::const_iterator shape_end = m_shape->end();

	for (size_t y = 0; y < result->get_size().y; ++y)
		for (size_t x = 0; x < result->get_size().x; ++x, ++i) {
			C3DShape::const_iterator shape_i = m_shape->begin();
			T value = cmp::start_value();

			while (shape_i != shape_end) {

				if (shape_i->z + read_slice >= get_start() &&
				    shape_i->z + read_slice < get_end()) {
					if (size_t(shape_i->y + y) < result->get_size().y)
						if (size_t(shape_i->x + x) < result->get_size().x) {
							T v = input(shape_i->x + x,
									shape_i->y + y,
									shape_i->z + read_slice);
							if (cmp::apply(value, v))
								 value = v;
						}
				}
				++shape_i;
			}
			*i = value;
		}
	return result;
}

template <template <typename, bool> class Compare>
void C2DMorphFifoFilter<Compare>::do_push(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMorphFifoFilter<Compare>::do_push");
	mia::accumulate(*this, *x);
}

template <template <typename, bool> class Compare>
void C2DMorphFifoFilter<Compare>::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMorphFifoFilter<Compare>::do_initialize");
	m_slice_size = x->get_size();
	m_buffer.reset(create_buffer(m_slice_size, m_shape->get_size().z, x->get_pixel_type()));

}

template <template <typename, bool> class Compare>
P2DImage C2DMorphFifoFilter<Compare>::do_filter()
{
	TRACE("C2DMorphFifoFilter<Compare>::do_filter");


	return P2DImage(mia::filter(*this, *m_buffer));
}

template <template <typename, bool> class Compare>
void C2DMorphFifoFilter<Compare>::shift_buffer()
{
	TRACE("C2DMorphFifoFilter<Compare>::shift_buffer");

	mia::filter_inplace(m_shifter, *m_buffer);
}


C2DMorphFifoFilterPluginBase::C2DMorphFifoFilterPluginBase(const char *name):
	C2DFifoFilterPlugin(name)
{
	add_parameter("shape", make_param(m_shape, "6n", false, "structuring element"));
}


const string C2DMorphFifoFilterPluginBase::do_get_descr() const
{
	stringstream descr;
	descr << "2D Stack morphological filter '" << get_name() << "'";
	return descr.str();
}

mia::P3DShape C2DMorphFifoFilterPluginBase::get_shape() const
{
	return m_shape;
}


template <template <typename, bool> class Compare>
C2DMorphFifoFilterPlugin<Compare>::C2DMorphFifoFilterPlugin(const char *name):
	C2DMorphFifoFilterPluginBase(name)
{
}


template <template <typename, bool> class Compare>
C2DImageFifoFilter *C2DMorphFifoFilterPlugin<Compare>::do_create()const
{
	TRACE("C2DMorphFifoFilterPlugin<Compare>::do_create()");
	return new C2DMorphFifoFilter<Compare>(get_shape());
}

C2DDilateFifoFilterPlugin::C2DDilateFifoFilterPlugin():
	C2DMorphFifoFilterPlugin<DilateCompare>("dilate")
{
}

C2DErodeFifoFilterPlugin::C2DErodeFifoFilterPlugin():
	C2DMorphFifoFilterPlugin<ErodeCompare>("erode")
{
}


C2DOpenFifoFilterPlugin::C2DOpenFifoFilterPlugin():
	C2DMorphFifoFilterPluginBase("open")
{
}


C2DImageFifoFilter *C2DOpenFifoFilterPlugin::do_create()const
{
	TRACE("C2DOpenFifoFilterPlugin::do_create()");
	P2DImageFifoFilter inm(new C2DMorphFifoFilter<DilateCompare>(get_shape()));
	auto result = new C2DMorphFifoFilter<ErodeCompare>(get_shape());
	result->append_filter(inm);
	return result;
}


C2DCloseFifoFilterPlugin::C2DCloseFifoFilterPlugin():
	C2DMorphFifoFilterPluginBase("close")
{
}

C2DImageFifoFilter *C2DCloseFifoFilterPlugin::do_create()const
{
	TRACE("C2DCloseFifoFilterPlugin::do_create()");
	P2DImageFifoFilter inm(new C2DMorphFifoFilter<ErodeCompare>(get_shape()));
	auto result = new C2DMorphFifoFilter<DilateCompare>(get_shape());
	result->append_filter(inm);
	return result;
}



extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *p = new C2DDilateFifoFilterPlugin();
	p->append_interface(new C2DErodeFifoFilterPlugin());
	p->append_interface(new C2DOpenFifoFilterPlugin());
	p->append_interface(new C2DCloseFifoFilterPlugin());

	return p;
}

template class C2DMorphFifoFilter<ErodeCompare>; 
template class C2DMorphFifoFilter<DilateCompare>; 
}
