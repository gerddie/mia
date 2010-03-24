/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
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

/*! \brief fft filter, required the definition of a kernel


\file fft.cc
\author Gert Wollny <gert.wollny at die.upm.es>

*/

#include <limits>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter/fft.hh>

NS_BEGIN(fft_2dimage_filter)
NS_MIA_USE;
using namespace std; 

C2DFft::C2DFft(const PFFT2DKernel&  kernel):
	_M_kernel(kernel)
{
}

template <typename T, bool is_integral>
struct FBackConvert {
	FBackConvert(float scale):
		_M_scale(scale) 
		{
			cvdebug() << "scale = " << _M_scale <<"\n"; 
		}
	
	T operator ()(float x) {
		return T(x *_M_scale); 
	}
private: 
	float _M_scale; 
};

template <typename T>
struct FBackConvert<T, true> {
	FBackConvert(float scale):
		_M_scale(scale) 
		{
			cvdebug() << "scale = " << _M_scale <<"\n"; 
		}
	
	T operator ()(float x) {
		float xc = x *_M_scale; 
		return xc < numeric_limits<T>::min() ? numeric_limits<T>::min() : 
			( xc < numeric_limits<T>::max() ?  T(xc) : numeric_limits<T>::max()); 
	}
private: 
	float _M_scale; 
};


template <typename T>
typename C2DFft::result_type C2DFft::operator () (const T2DImage<T>& image) const
{
	cvdebug() << "C2DFFT::operator() begin\n";
	
	float *buffer = _M_kernel->prepare(image.get_size()); 
	size_t realsize_x = image.get_size().x; 

	// fill buffer with image data
	for (size_t y = 0; y < image.get_size().y; ++y) 
		if (y & 1) 
			transform(image.begin_at(0,y), image.begin_at(0,y) + image.get_size().x, &buffer[y * realsize_x], 
				  bind2nd(multiplies<float>(), -1)); 
		else
			copy(image.begin_at(0,y), image.begin_at(0,y) + image.get_size().x, &buffer[y *  realsize_x]); 
	
	_M_kernel->apply(); 

	T2DImage<T> *result = new T2DImage<T>(image.get_size()); 
	
	const bool is_integral = ::boost::is_integral<T>::value; 
	
	float scale = 1.0/ image.size(); 

	FBackConvert<T, is_integral> convertp(scale);
	FBackConvert<T, is_integral> convertm(-scale);

	for (size_t y = 0; y < image.get_size().y; ++y)
		if (y & 1)
			transform(&buffer[y * realsize_x], &buffer[y * realsize_x + image.get_size().x], result->begin_at(0,y), convertm);
		else
			transform(&buffer[y * realsize_x], &buffer[y * realsize_x + image.get_size().x], result->begin_at(0,y), convertp);
	
	return P2DImage(result);
}

P2DImage C2DFft::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DFilterPluginFactory::C2DFilterPluginFactory():
	C2DFilterPlugin("fft")
{
	add_parameter("k", new CStringParameter(_M_kernel, true, "filter kernel")); 
}

C2DFilterPluginFactory::ProductPtr C2DFilterPluginFactory::do_create()const
{
	PFFT2DKernel k = CFFT2DKernelPluginHandler::instance().produce(_M_kernel.c_str()); 
	return C2DFilterPluginFactory::ProductPtr(new C2DFft(k)); 
}

	const string C2DFilterPluginFactory::do_get_descr()const
	{
	return "2D image fft filter"; 
}
	
extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DFilterPluginFactory(); 
}

NS_END
