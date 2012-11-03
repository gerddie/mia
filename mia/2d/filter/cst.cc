/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <limits>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter/cst.hh>


NS_BEGIN(filter_2dimage_cst)

NS_MIA_USE;
using namespace std;

C2DCst::C2DCst(const PCST2DImageKernel&  kernel):
	m_kernel(kernel)
{
}

template <typename T, bool is_integral>
struct FBackConvert {
	FBackConvert(double scale):
		m_scale(scale)
		{
			cvdebug() << "scale = " << m_scale <<"\n";
		}

	T operator ()(double x) {
		return T(x *m_scale);
	}
private:
	double m_scale;
};

template <typename T>
struct FBackConvert<T, true> {
	FBackConvert(double scale):
		m_scale(scale)
		{
			cvdebug() << "scale = " << m_scale <<"\n";
		}

	T operator ()(double x) {
		double xc = x *m_scale;
		return xc < numeric_limits<T>::min() ? numeric_limits<T>::min() :
			( xc < numeric_limits<T>::max() ?  T(xc) : numeric_limits<T>::max());
	}
private:
	double m_scale;
};


template <typename T>
struct __dispatch_fftw {
	static void apply(const CCST2DImageKernel& kernel, const T2DImage<T>& in, T2DImage<T>& out) {
		C2DFImage image(in.get_size());
		copy(in.begin(), in.end(), image.begin());
		kernel.apply(image, image);

		const bool is_integral = ::boost::is_integral<T>::value;
		FBackConvert<T, is_integral> convert(1.0);
		transform(image.begin(), image.end(), out.begin(), convert);
	}
};

template <>
struct __dispatch_fftw<float> {
	static void apply(const CCST2DImageKernel& kernel, const C2DFImage& in, C2DFImage& out) {
		kernel.apply(in, out);
	}
};

template <typename T>
typename C2DCst::result_type C2DCst::operator () (const T2DImage<T>& image) const
{
	cvdebug() << "C2DCST::operator() begin\n";

	m_kernel->prepare(image.get_size());
	T2DImage<T> *result = new T2DImage<T>(image.get_size(), image);

	__dispatch_fftw<T>::apply(*m_kernel, image, *result);

	return P2DImage(result);
}

P2DImage C2DCst::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DFilterPluginFactory::C2DFilterPluginFactory():
	C2DFilterPlugin("cst")
{
	add_parameter("k", make_param(m_kernel, "", true, "filter kernel"));
}

C2DFilter *C2DFilterPluginFactory::do_create()const
{
	return new C2DCst(m_kernel);
}

const string C2DFilterPluginFactory::do_get_descr()const
{
	return "Run a filter in the frequency domain by applying a cosinus or sinus transformation, "
		"running the filter, and applying the backward cosinus or sinus transformation. ";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DFilterPluginFactory();
}

NS_END // end namespace cst_2dimage_filter
