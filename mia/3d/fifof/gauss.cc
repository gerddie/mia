/* -*- mia-c++  -*-
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*! \brief basic type of a plugin handler

A gauss filter for stacks of 2D images

\author Gert Wollny <gerddie at gmail.com>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include <iomanip>
#include <limits>
#include <numeric>

#include <boost/cast.hpp>
#include <boost/lambda/lambda.hpp>


#include <mia/3d/fifof/gauss.hh>



using namespace boost::lambda;
using namespace boost;

NS_BEGIN(gauss_2dstack_filter)

NS_MIA_USE;
using namespace std;

template <class InputIterator, class OutputIterator>
static void convert(InputIterator a, InputIterator e, OutputIterator o)
{
	while (a != e) {
		*o = numeric_cast<typename iterator_traits<OutputIterator>::value_type>(*a);
		++o;
		++a;
	}
}

class C2DGaussStackFilterFactory : public C2DFifoFilterPlugin {
public:
	C2DGaussStackFilterFactory();
private:

	virtual const string do_get_descr() const;
	virtual bool do_test() const;
	virtual C2DFifoFilterPlugin::ProductPtr do_create()const;

	mutable int _M_hw;
};


C2DGaussFifoFilter::C2DGaussFifoFilter(size_t hw):
	C2DImageFifoFilter(2*hw + 1, hw + 1, 0),
	_M_hw(hw)
{
	const C2DFilterPluginHandler::Instance& filter_plugins = C2DFilterPluginHandler::instance();
	stringstream filter_descr;
	filter_descr << "gauss:w=" << hw;

	_M_gauss2d = filter_plugins.produce(filter_descr.str().c_str());
	if (!_M_gauss2d) {
		filter_descr << " is not a supported filter description";
		throw runtime_error(filter_descr.str());
	}
	_M_1dfilter = C1DSpacialKernelPluginHandler::instance().produce(filter_descr.str().c_str());
}

template <typename T>
C2DImage *C2DGaussFifoFilter::operator()(const T3DImage<T>& /*buffer*/) const
{
	TRACE("C2DGaussFifoFilter::pull");

	C2DFImage help(_M_slice_size);

	if ((int)get_start() == 0 && get_end() == _M_1dfilter->size()) {
		const float k = (*_M_1dfilter)[0];
		transform(_M_buffer->begin(), _M_buffer->begin() + help.size(), help.begin(), _1 * k);

		for (size_t i =  1; i < get_end(); ++i) {
			C2DFImage::iterator s_i = _M_buffer->begin_at(0,0,i);
			const float k = (*_M_1dfilter)[i];
			transform(help.begin(), help.end(), s_i, help.begin(), _1 + k * _2);
		}
	}else{
		const float k = (*_M_1dfilter)[0];
		transform(_M_buffer->begin_at(0,0,get_end() - 1),
			  _M_buffer->begin_at(0,0,get_end()),
			  help.begin(), _1 * k);

		for (size_t i =  1; i < get_end(); ++i) {
			C2DFImage::iterator s_i = _M_buffer->begin_at(0,0, get_end() - 1 - i);
			const float k = (*_M_1dfilter)[i];
			transform(help.begin(), help.end(), s_i, help.begin(), _1 + k * _2);
		}

	}
	T2DImage<T> *retval = new T2DImage<T>(_M_slice_size);
	convert(help.begin(), help.end(), retval->begin());
	return retval;
}

// adds new data at the beginning of the input
template <typename T>
C2DImage *C2DGaussFifoFilter::operator()(const T2DImage<T>& input)
{
	TRACE("C2DGaussFifoFilter::push internal");
	copy(input.begin(), input.end(), _M_buffer->begin());
	return NULL;
}

void C2DGaussFifoFilter::shift_buffer()
{
	copy_backward(_M_buffer->begin(),
		      _M_buffer->end() - _M_buffer->get_size().x * _M_buffer->get_size().y ,
		      _M_buffer->end());
}

void C2DGaussFifoFilter::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
	_M_slice_size = x->get_size();
	_M_buffer.reset(new C3DFImage(C3DBounds(_M_slice_size.x, _M_slice_size.y, 2 * _M_hw + 1)));
	_M_dummy.reset(create_buffer(C2DBounds(1,1), 1, x->get_pixel_type()));
}

void C2DGaussFifoFilter::do_push(::boost::call_traits<P2DImage>::param_type x)
{
	P2DImage tmp = _M_gauss2d->filter(*x);
	mia::accumulate(*this, *tmp);

}

P2DImage C2DGaussFifoFilter::do_filter()
{
	return 	P2DImage(mia::filter(*this, *_M_dummy));
}

void C2DGaussFifoFilter::post_finalize()
{
	_M_buffer.reset(NULL);
	_M_dummy.reset(NULL);
}

C2DGaussStackFilterFactory::C2DGaussStackFilterFactory():
	C2DFifoFilterPlugin("gauss"),
	_M_hw(1)
{
	add_parameter("w", new CIntParameter(_M_hw, 0, numeric_limits<int>::max(),
					     false, "filter width parameter (2*w+1)"));
}

const string C2DGaussStackFilterFactory::do_get_descr() const
{
	return "Gauss 2D fifo filter";
}


bool C2DGaussStackFilterFactory::do_test() const
{
	return true;
}

C2DFifoFilterPlugin::ProductPtr C2DGaussStackFilterFactory::do_create()const
{
	return ProductPtr(new C2DGaussFifoFilter(_M_hw));
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DGaussStackFilterFactory();
}

NS_END // end namespace median_2dstack_filter
