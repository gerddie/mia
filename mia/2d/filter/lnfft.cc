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

#include <cmath>
#include <functional>
#include <stdexcept>

#include <boost/type_traits.hpp>
#include <mia/2d/2dfilter.hh>
#include <libmia/fftkernel.hh>

// to avoid the indention
#define NSBEGIN namespace lnfft_2dimage_filter {
#define NSEND }

NSBEGIN

NS_MIA_USE;
using namespace std; 


static const char *plugin_name = "lnfft"; 
static const CStringOption param_kernel("kernel", "filter kernel", NULL); 


class C2DLnFFT: public C2DFilter {
public: 
	C2DLnFFT(C2DFFTKernelFactory::ProductPtr kernel); 
	~C2DLnFFT();
	
	void init(const C2DBounds& size); 
	
	template <typename T>
	C2DLnFFT::result_type operator()(const T2DImage<T>& image)const; 
		
	
private: 
	C2DFFTKernelFactory::ProductPtr m_kernel; 
	C2DBounds m_size; 
	union {
		fftw_complex *m_cbuffer; 
		double   *m_fbuffer; 
	}; 
	fftw_plan m_forward_plan; 
	fftw_plan m_backward_plan; 
	size_t m_realsize_x; 
};

class C2DImageLnFFTFilter: public C2DImageFilterBase {
	mutable C2DLnFFT m_filter; 
public:
	C2DImageLnFFTFilter(C2DFFTKernelFactory::ProductPtr kernel);

	virtual P2DImage do_filter(const C2DImage& image) const;
};

class C2DImageLnFFTFilterFactory: public C2DFilterPlugin {
public: 
	C2DImageLnFFTFilterFactory();
	virtual C2DFilter *create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
};


C2DLnFFT::C2DLnFFT(C2DFFTKernelFactory::ProductPtr kernel):
	m_kernel(kernel), 
	m_size(0,0), 
	m_cbuffer(NULL), 
	m_realsize_x(0)
{
}

C2DLnFFT::~C2DLnFFT()
{
	if (m_cbuffer) {
		fftw_free(m_cbuffer); 
		fftw_destroy_plan( m_forward_plan); 
		fftw_destroy_plan( m_backward_plan); 		
	}
}

void C2DLnFFT::init(const C2DBounds& size)
{
	if (m_size == size)
		return; 
	cvdebug() << "C2DLnFFT::init ...\n"; 
	
	m_size = size; 
	m_realsize_x = 2 * (m_size.x /2 + 1); 

	cvdebug() << "size = " << m_size.x << ", " << m_size.y << "\n"; 
	if (m_cbuffer) {
		delete[] m_cbuffer; 
		fftw_destroy_plan( m_forward_plan); 
		fftw_destroy_plan( m_backward_plan); 		
	}
	
	m_cbuffer = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_size.y * m_realsize_x);
	// create the fftw plans
	
	cvdebug() << "buffer at " << m_cbuffer << ":" << m_fbuffer << "\n"; 
	
	if (!m_cbuffer) {
		throw runtime_error("unable to allocate fftw buffers"); 
	}
	
	 m_forward_plan = fftw_plan_dft_r2c_2d(m_size.y, m_size.x,
						m_fbuffer, m_cbuffer, FFTW_ESTIMATE);

	 cvdebug() << "forward plan at " <<  m_forward_plan << "\n"; 
	 if (!m_forward_plan) {
		 //fftw_free(m_cbuffer); 
		 throw runtime_error("unable to create forward plans ..."); 
	 }
	 
	 m_backward_plan = fftw_plan_dft_c2r_2d(m_size.y, m_size.x,
						 m_cbuffer, m_fbuffer, FFTW_ESTIMATE);
	 
	 if (!m_backward_plan) {
		 //fftw_free(m_cbuffer); 
		 //fftw_destroy_plan( m_forward_plan); 		 
		 throw runtime_error("unable to create backward plans ..."); 
	 }
	 
	 cverb << "done\n"; 
}

template <typename T, bool is_integral>
struct FBackConvert {
	FBackConvert(double scale):
		m_scale(scale) 
		{
			cvdebug() << "scale = " << m_scale <<"\n"; 
		}
	
	T operator ()(double x) {
		return T(exp(x *m_scale) - 1.0); 
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
		double xc = exp(x *m_scale) - 1.0; 
		return xc < numeric_limits<T>::min() ? numeric_limits<T>::min() : 
			( xc < numeric_limits<T>::max() ?  T(xc) : numeric_limits<T>::max()); 
	}
private: 
	double m_scale; 
};

struct ForwardLogify {
	ForwardLogify(double f):m_f(f){}
	double operator ()(double x) {
		return m_f * log(x + 1.0); 
	}
private: 
	double m_f; 
};

template <typename T>
C2DLnFFT::result_type C2DLnFFT::operator()(const T2DImage<T>& image)const
{
	ForwardLogify flp(1.0); 
	ForwardLogify flm(-1.0); 
	
	cvdebug() << "C2DLnFFT::operator() begin\n";
	for (size_t y = 0; y < image.get_size().y; ++y) 
		transform(&image(0,y), &image(0,y) + image.get_size().x, &m_fbuffer[y * m_realsize_x], y & 1 ? flm : flp); 
		
	cvdebug() << "C2DLnFFT::operator() forward transform\n";
	fftw_execute( m_forward_plan); 

	cvdebug() << "C2DLnFFT::operator() call kernel\n";
	m_kernel->apply(m_size, m_cbuffer); 
	
	cvdebug() << "C2DLnFFT::operator() backward transform\n";
	fftw_execute( m_backward_plan); 

	cvdebug() << "C2DLnFFT::operator() back copy\n";
	T2DImage<T> *result = new T2DImage<T>(image.get_size(), image); 
	
	
	const bool is_integral = ::boost::is_integral<T>::value; 
	
	FBackConvert<T, is_integral> convertp(1.0/ image.size());
	FBackConvert<T, is_integral> convertm(-1.0/ image.size());
	// copy back and scale filtered output
	for (size_t y = 0; y < image.get_size().y; ++y)
		transform(&m_fbuffer[y * m_realsize_x], &m_fbuffer[y * m_realsize_x + m_size.x] , 
			  &(*result)(0,y), (y & 1) ? convertm : convertp);
	
	cvdebug() << "C2DLnFFT::operator() end\n";
	return P2DImage(result); 
	
}

C2DImageLnFFTFilter::C2DImageLnFFTFilter(C2DFFTKernelFactory::ProductPtr kernel):
	m_filter(kernel)
{
}
	

P2DImage C2DImageLnFFTFilter::do_filter(const C2DImage& image) const
{
	m_filter.init(image.get_size()); 
	return wrap_filter(m_filter, image); 
}


C2DImageLnFFTFilterFactory::C2DImageLnFFTFilterFactory():
	C2DFilterPlugin(plugin_name)
{
}

C2DFilter *C2DImageLnFFTFilterFactory::create(const CParsedOptions& options) const
{
	string kernel_descr = param_kernel.get_value(options); 
	if (kernel_descr.empty())
	    throw invalid_argument("fft: no kernel given"); 
	
	C2DFFTKernelPluginHandler fftkh; 
	C2DFFTKernelFactory::ProductPtr kernel(C2DFFTKernelFactory::produce(kernel_descr.c_str(), fftkh)); 
	if (!kernel) 
		throw invalid_argument(string("fft: unable to create cernel from '") + kernel_descr + "'");
	
	return ProductPtr(new C2DImageLnFFTFilter(kernel)); 
}

const string C2DImageLnFFTFilterFactory::do_get_descr()const
{
	return "2D image frequency domain filters based on LnFFT";
}


struct FCompare {
	bool operator () (float a, float b){
		return ::fabs(a-b) < 0.0001; 
	}
};


int C2DImageLnFFTFilterFactory::do_test() const 
{
	srand48(time(NULL)); 
	class CFFTKernelDummy : public CFFTKernel {
		virtual void do_apply(const C2DBounds& size, fftw_complex *cbuffer) const{};
	};
	
	CFFTKernel *k = new CFFTKernelDummy(); 
	C2DFFTKernelFactory::ProductPtr kernel(k); 
	
	C2DImageLnFFTFilter f(kernel); 
	
	C2DBounds size(16, 16); 
	C2DFImage *src = new C2DFImage(size); 
	P2DImage src_wrap(src); 

	for (C2DFImage::iterator i = src->begin(); i != src->end(); ++i)
		*i = 1; //drand48(); 
	
	P2DImage res_wrap = f.filter(src_wrap); 
	
	C2DFImage *res = res_wrap.getC2DFImage(); 
	if (!res) {
		cvfail() << get_name() << "image pixel type\n"; 
		return -1; 
	}
	
	if (res->get_size() != src->get_size()) {
		cvfail() << get_name() << "image size\n"; 
		return -1; 
	}
	
	if (equal(src->begin(), src->end(), res->begin(), FCompare()))
		return 1; 
	
	if (cverb.show_debug()) {
		for(C2DFImage::const_iterator s = src->begin(), r = res->begin(); s != src->end(); ++s, ++r)
			cvdebug() << *s << " : " << *r << "\n"; 
		    
	}
	
	return -1; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DImageLnFFTFilterFactory(); 
}



NSEND
