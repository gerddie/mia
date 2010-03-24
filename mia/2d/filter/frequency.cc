/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004 - 2008 
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */



/*! \brief fft 2D image filter 

A median filter 

\file median_2dimage_filter.cc
\author Gert Wollny <wollny at eva.mpg.de>

*/

#include <functional>
#include <stdexcept>

#include <boost/type_traits.hpp>

#include <mia/2d/2dfilter.hh>

#include <libmona/fftkernel.hh>

// to avoid the indention
#define NSBEGIN namespace frequency_2dimage_filter {
#define NSEND }

NSBEGIN

NS_MIA_USE;
using namespace std; 


static const char *plugin_name = "frequency"; 

class C2DFFT: public C2DFilter {
public: 
	C2DFFT(); 
	~C2DFFT();
	
	void init(const C2DBounds& size); 
	
	template <typename T>
	C2DFFT::result_type operator()(const T2DImage<T>& image)const; 
		
	
private: 
	C2DBounds _M_size; 
	fftw_complex *_M_cbuffer; 
	fftw_plan _M_forward_plan; 
};

class C2DImageFFTFilter: public C2DImageFilterBase {
	mutable C2DFFT _M_filter; 
public:
	C2DImageFFTFilter();

	virtual P2DImage do_filter(const C2DImage& image) const;
};

class C2DImageFFTFilterFactory: public C2DFilterPlugin {
public: 
	C2DImageFFTFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
	virtual int do_test() const; 
};


C2DFFT::C2DFFT():
	_M_size(0,0), 
	_M_cbuffer(NULL)
{
}

C2DFFT::~C2DFFT()
{
	if (_M_cbuffer) {
		fftw_free(_M_cbuffer); 
		fftw_destroy_plan( _M_forward_plan); 
	}
}

void C2DFFT::init(const C2DBounds& size)
{
	if (_M_size == size)
		return; 
	cvdebug() << "C2DFFT::init ...\n"; 
	
	_M_size = size; 

	cvdebug() << "size = " << _M_size.x << ", " << _M_size.y << "\n"; 
	if (_M_cbuffer) {
		delete[] _M_cbuffer; 
		fftw_destroy_plan( _M_forward_plan); 
	}
	
	_M_cbuffer = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * _M_size.y * _M_size.x);
	// create the fftw plans
	
	cvdebug() << "buffer at " << _M_cbuffer << "\n"; 
	
	if (!_M_cbuffer) {
		throw runtime_error("unable to allocate fftw buffers"); 
	}
	
	 _M_forward_plan = fftw_plan_dft_2d(_M_size.y, _M_size.x,
					    _M_cbuffer, _M_cbuffer, FFTW_FORWARD, FFTW_ESTIMATE);

	 cvdebug() << "forward plan at " <<  _M_forward_plan << "\n"; 
	 if (!_M_forward_plan) {
		 //fftw_free(_M_cbuffer); 
		 throw runtime_error("unable to create forward plans ..."); 
	 }
	 
	 cverb << "done\n"; 
}

template <typename T, bool is_integral>
struct FBackConvert {
	FBackConvert(double scale):
		_M_scale(scale) 
		{
			cvdebug() << "scale = " << _M_scale <<"\n"; 
		}
	
	T operator ()(double x) {
		return T(x *_M_scale); 
	}
private: 
	double _M_scale; 
};

template <typename T>
struct FBackConvert<T, true> {
	FBackConvert(double scale):
		_M_scale(scale) 
		{
			cvdebug() << "scale = " << _M_scale <<"\n"; 
		}
	
	T operator ()(double x) {
		double xc = x *_M_scale; 
		return xc < numeric_limits<T>::min() ? numeric_limits<T>::min() : 
			( xc < numeric_limits<T>::max() ?  T(xc) : numeric_limits<T>::max()); 
	}
private: 
	double _M_scale; 
};

template <typename T>
C2DFFT::result_type C2DFFT::operator()(const T2DImage<T>& image)const
{
	cvdebug() << "C2DFFT::operator() begin\n";
	typename T2DImage<T>::const_iterator i = image.begin(); 
	fftw_complex *t = _M_cbuffer; 
	for (size_t y = 0; y < image.get_size().y; ++y) {
		float sign = y & 1 ? -1 : 1; 
		for (size_t x = 0; x < image.get_size().x; ++x, sign *=-1, ++i, ++t) {
			(*t)[0] = sign * *i; 
			(*t)[1] = 0.0; 
		}
	}
	
	cvdebug() << "C2DFFT::operator() forward transform\n";
	fftw_execute( _M_forward_plan); 
	
	fftw_complex *cbuffer = _M_cbuffer;
	
	for (size_t i = 0; i < image.size(); ++i, ++cbuffer) {
		(*cbuffer)[0] = sqrt((*cbuffer)[0] * (*cbuffer)[0] + (*cbuffer)[1] * (*cbuffer)[1]); 
	}

	double factor =1.0/sqrt(image.size()); 
	cvdebug() << "factor = " << factor << "\n"; 
	
	T2DImage<T> *result = new T2DImage<T>(image.get_size());
	typename T2DImage<T>::iterator r = result->begin(); 
	cbuffer = _M_cbuffer;
	
	for (size_t y = 0; y < image.get_size().y; ++y)
		for (size_t x = 0; x < image.get_size().x; ++x, ++cbuffer, ++r) {
			double val = factor * (*cbuffer)[0]; 
			*r = val > numeric_limits<T>::max() ? numeric_limits<T>::max(): 
				val < numeric_limits<T>::min() ? val > numeric_limits<T>::min() : T(val); 
		}

	return P2DImage(result); 
}

C2DImageFFTFilter::C2DImageFFTFilter()
{
}
	

P2DImage C2DImageFFTFilter::do_filter(const C2DImage& image) const
{
	_M_filter.init(image.get_size()); 
	return wrap_filter(_M_filter, image); 
}


C2DImageFFTFilterFactory::C2DImageFFTFilterFactory():
	C2DFilterPlugin(plugin_name)
{
}

C2DFilterPlugin::ProductPtr C2DImageFFTFilterFactory::create(const CParsedOptions& options) const
{
	return ProductPtr(new C2DImageFFTFilter()); 
}

const string C2DImageFFTFilterFactory::do_get_descr()const
{
	return "2D image convert-to-frequency domain filters based on FFT";
}


struct FCompare {
	bool operator () (float a, float b){
		return ::fabs(a-b) < 0.0001; 
	}
};


int C2DImageFFTFilterFactory::do_test() const 
{
	return 0; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DImageFFTFilterFactory(); 
}



NSEND
