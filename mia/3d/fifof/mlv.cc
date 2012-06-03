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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

extern "C" {
#include <cblas.h>
}

#include <iomanip>
#include <limits>

#include <boost/cast.hpp>

#include <mia/3d/fifof/mlv.hh>

NS_BEGIN(mlv_2dstack_filter)

NS_MIA_USE
using namespace std;
using namespace boost;


template <class InputIterator, class OutputIterator>
static void convert(InputIterator a, InputIterator e, OutputIterator o)
{
	while (a != e) {
		*o = numeric_cast<typename iterator_traits<OutputIterator>::value_type>(*a);
		++o;
		++a;
	}
}

C2DMLVnFifoFilter::C2DMLVnFifoFilter(size_t hwidth):
	C2DImageFifoFilter(4 * hwidth + 1, 2 * hwidth + 1, 2 * hwidth),
	m_hw(hwidth),
	m_w(2 * hwidth + 1)
{
}

// This operator reads out the result

template <typename T>
C2DImage *C2DMLVnFifoFilter::operator()(const T3DImage<T>& /*dummy*/) const
{
	TRACE("C2DMLVnFifoFilter::operator() (pull)");
	
	static int slice = 0; 
	
	int start = get_start() + m_w-1; 
	int end = min(get_end() + m_w, get_buffer_size()); 

	cvdebug() << "Evaluate "<< slice++ << " output " << start << " - " << end << "\n"; 
	C2DFImage mu_result(m_slice_size);
	C2DFImage sigma_buffer(m_slice_size);

	fill(sigma_buffer.begin(), sigma_buffer.end(), numeric_limits<float>::max());

	int min_n = 100; 
	
	for (size_t z = start; z <  end; ++z) {
		for (size_t y = 0; y < m_slice_size.y; ++y) {
			for (size_t iy = 0; iy < m_w; ++iy)
				for (size_t ix = 0; ix < m_w; ++ix) {
					
					C3DFImage::const_iterator mu_i = m_mu[z].begin_at(ix, y + iy);
					C3DFImage::const_iterator si_i = m_sigma[z].begin_at(ix, y + iy);
					C2DFImage::iterator omu_i = mu_result.begin_at(0, y);
					C2DFImage::iterator osi_i = sigma_buffer.begin_at(0, y);

					for (size_t x = 0; x < m_slice_size.x; ++x) {
						if (osi_i[x] > si_i[x]) {
							osi_i[x] = si_i[x];
							omu_i[x] = mu_i[x];
						}
						if (min_n > m_n[z](x+ix, y +iy)) 
							min_n = m_n[z](x+ix, y +iy);
					}
				}

			}

	}
	cvdebug() << "Minimal n = " << min_n << "\n"; 

	T2DImage<T> *result = new T2DImage<T>(m_slice_size);
	convert(mu_result.begin(), mu_result.end(), result->begin());

	return result;
}

static void do_evaluate(C2DFImage::const_iterator ni, C2DFImage::const_iterator ne,
		     C2DFImage::iterator mu, C2DFImage::range_iterator sigma)
{
	while (ni != ne) {
		const float n = *ni;
		const float muq = *mu / *ni;

		const float s = (n > 1.0) ? (*sigma - muq * muq * n) / (n - 1.0f) : 0.0f;
//		cvdebug() << "  mu(sigma) " << sigma.pos() << " " << n << ", " << *mu << ", " << *sigma 
//			  << "= " << muq << "(" << s << ")\n"; 

		*mu++ = muq;
		*sigma = s;
		++sigma; 
		++ni;
	}
}

template <typename Iterator>
void rotate(Iterator begin, Iterator end)
{
	typename Iterator::value_type help = *(end - 1);
	copy_backward(begin, end - 1, end);
	*begin = help;

	fill(begin->begin(), begin->end(), 0.0); 
}

void C2DMLVnFifoFilter::shift_buffer()
{
	TRACE_FUNCTION; 
	cvdebug() << "Evaluate shift\n"; 
	rotate(m_mu.begin(), m_mu.end());
	rotate(m_sigma.begin(),m_sigma.end());
	rotate(m_n.begin(), m_n.end());
	copy_backward(m_evaluated.begin(), m_evaluated.end() - 1, m_evaluated.end());
	m_evaluated[0] = false; 
}

void C2DMLVnFifoFilter::evaluate(size_t slice)
{
	TRACE_FUNCTION; 
	if (m_evaluated[slice]) 
		return; 

	cvdebug() << "Evaluate mu(sigma) for  " << slice << "\n"; 
	do_evaluate(m_n[slice].begin(),
		    m_n[slice].end(),
		    m_mu[slice].begin(),
		    m_sigma[slice].begin_range(C2DBounds::_0, m_sigma[slice].get_size()));
	m_evaluated[slice] = true; 
}

// this operator add the new slice

template <typename In, typename Out>
bool copy_was_zero(In b, In e, Out o)
{
	bool result = true; 
	while (b != e) {
		result &= (*b == 0); 
		*o = *b; 
		++o; 
		++b; 
	}
	return result; 
}


template <typename T>
C2DImage *C2DMLVnFifoFilter::operator()(const T2DImage<T>& src)
{
	TRACE("C2DMLVnFifoFilter::operator()(const T2DImage<T>& input)(push)");

	cvdebug() << "Evaluate input\n"; 

	
	C2DBounds temp_size = m_mu[0].get_size(); 

	vector<float> val(src.get_size().x); 
	vector<float> val2(src.get_size().x);
			
	vector<float> sum_mu_l1(temp_size.x); 
	vector<float> sum_sigma_l1(temp_size.x);

	vector<float> sum_mu_l2(m_mu[0].size(), 0.0); 
	vector<float> sum_sigma_l2(m_mu[0].size(), 0.0);

	
	for (size_t y = 0; y < src.get_size().y; ++y){
		
		// copy one input row and check whether it is all zero, if yes, shortcut
		if (copy_was_zero(src.begin_at(0,y), src.begin_at(0,y) + src.get_size().x, val.begin()))
			continue; 

		fill(sum_mu_l1.begin(), sum_mu_l1.end(), 0.0); 
		fill(sum_sigma_l1.begin(), sum_sigma_l1.end(), 0.0); 
		
		transform(val.begin(), val.end(), val2.begin(),[](float x) { return  x * x;});
		
		for(size_t x = 0; x < m_w; ++x) {
			cblas_saxpy(src.get_size().x, 1.0f, &val[0],  1, &sum_mu_l1[x], 1); 
			cblas_saxpy(src.get_size().x, 1.0f, &val2[0], 1, &sum_sigma_l1[x], 1);
		}
		
		for(size_t iy = 0; iy < m_w; ++iy) {
			cblas_saxpy(sum_mu_l1.size(), 1.0f, &sum_mu_l1[0],  1, &sum_mu_l2[(y + iy) * temp_size.x], 1); 
			cblas_saxpy(sum_sigma_l1.size(), 1.0f, &sum_sigma_l1[0],  1, &sum_sigma_l2[(y + iy) * temp_size.x], 1);
		}
	}
	// update the numbers
	for (size_t z = 0; z < m_w; ++z) {
		cblas_saxpy(sum_mu_l2.size(), 1.0f, &sum_mu_l2[0],  1, &m_mu[z](0,0) , 1); 
		cblas_saxpy(sum_sigma_l2.size(), 1.0f, &sum_sigma_l2[0],  1,&m_sigma[z](0,0) , 1);
		cblas_saxpy(m_n_template.size(), 1.0f, &m_n_template(0,0), 1, &m_n[z](0,0), 1); 
	}
	// the next shift will push this slice out if the input range 
	// ence we can evaluate the real values here. 
	evaluate(m_w-1); 
	

	return NULL;
}

void C2DMLVnFifoFilter::post_finalize()
{
	TRACE_FUNCTION; 
	m_mu.resize(0);
	m_sigma.resize(0);
	m_n.resize(0);
}

void C2DMLVnFifoFilter::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMLVnFifoFilter::do_initialize");

	m_slice_size = x->get_size();
	cvdebug() << "Input slices are of size " << m_slice_size << "\n"; 
	
	C2DBounds size(m_slice_size.x + 2 * m_hw,
		       m_slice_size.y + 2 * m_hw);

	size_t n_slices = get_buffer_size();

	m_mu = vector<C2DFImage>(n_slices);
	m_sigma = vector<C2DFImage>(n_slices);
	m_n = vector<C2DFImage>(n_slices);
	m_evaluated = vector<bool>(n_slices, false);
		
	for (size_t i = 0; i < n_slices; ++i) {
		m_mu[i] = C2DFImage(size);
		m_sigma[i] = C2DFImage(size);
		m_n[i] = C2DFImage(size);
	}

	m_buf_slice_size = size.x * size.y;

	m_buf1.resize(m_slice_size.x);
	m_buf2.resize(m_slice_size.x);
	m_prototype.reset(create_buffer(C2DBounds(1,1), 1, x->get_pixel_type()));


	m_n_template = C2DFImage(C2DBounds(m_slice_size.x + 2 * m_hw, m_slice_size.y + 2 * m_hw));
	// create the pattern that is used to update the count
	// this is the slow version, but we have to do it only once
	for (size_t y = 0; y < m_slice_size.y; ++y)
		for (size_t x = 0; x < m_slice_size.x; ++x)
			for (size_t iy = 0; iy < 2 * m_hw + 1; ++iy)
				for (size_t ix = 0; ix < 2 * m_hw + 1; ++ix)
					m_n_template(x + ix, y + iy) += 1.0;
}

void C2DMLVnFifoFilter::do_push(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMLVnFifoFilter::do_push");
	mia::accumulate(*this, *x);
}

P2DImage C2DMLVnFifoFilter::do_filter()
{
	TRACE("C2DMLVnFifoFilter::do_filter");
	return P2DImage(mia::filter(*this, *m_prototype));
}


class C2DMLVnFifoFilterPlugin : public C2DFifoFilterPlugin {
public:
	C2DMLVnFifoFilterPlugin();
private:

	virtual const string do_get_descr() const;
	virtual bool do_test() const;
	virtual C2DImageFifoFilter *do_create()const;

	mutable int m_hw;
};

C2DMLVnFifoFilterPlugin::C2DMLVnFifoFilterPlugin():
	C2DFifoFilterPlugin("mlv"),
	m_hw(1)
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(),
					     false, "filter width parameter"));
}

const string C2DMLVnFifoFilterPlugin::do_get_descr() const
{
	return "Mean of Least Variance 2D stack image filter";
}

bool C2DMLVnFifoFilterPlugin::do_test() const
{
	return true;
}

C2DImageFifoFilter *C2DMLVnFifoFilterPlugin::do_create()const
{
	return new C2DMLVnFifoFilter(m_hw);
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{

	return new C2DMLVnFifoFilterPlugin();
}

NS_END

