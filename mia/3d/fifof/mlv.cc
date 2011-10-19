/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

/* 
   LatexBeginPluginDescription{2D image stack filters}
   
   \subsection{Mean least variance}
   \label{fifof:mlv}
   
   \begin{description}
   
   \item [Plugin:] mlv 
   \item [Description:] Runs a mean least variance filter on the imput images. 
               The number of slices that are hold in the working memory is deducted from 
	       the filter size.
   \item [Input:] Gray scale or binary images, all of the same size and pixel type  
   \item [Output:] The filtered image(s) 
   
   \plugtabstart
   w & int & filter width parameter, the actual filter captures a neighborhood of 
                 $(4 * w + 1) \times (4 * w + 1) \times (4 * w + 1)$ voxels & 1 \\ 
   \plugtabend
   
   \end{description}

   LatexEnd  
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
	m_w(2 * hwidth + 1),
	m_read_start(2 * hwidth)
{
}

// This operator reads out the result

template <typename T>
C2DImage *C2DMLVnFifoFilter::operator()(const T3DImage<T>& /*dummy*/) const
{
	TRACE("C2DMLVnFifoFilter::operator() (pull)");
	C2DFImage mu_result(m_slice_size);
	C2DFImage sigma_buffer(m_slice_size);

	fill(sigma_buffer.begin(), sigma_buffer.end(), numeric_limits<float>::max());

	for (size_t z = get_start(); z < get_end(); ++z) {
		for (size_t iy = 0; iy < m_w; ++iy)
			for (size_t ix = 0; ix < m_w; ++ix) {
				for (size_t y = 0; y < m_slice_size.y; ++y) {
					C3DFImage::const_iterator mu_i = m_mu_buffer[z].begin_at(ix, y + iy);
					C3DFImage::const_iterator si_i = m_sigma_buffer[z].begin_at(ix, y + iy);
					C2DFImage::iterator omu_i = mu_result.begin_at(0, y);
					C2DFImage::iterator osi_i = sigma_buffer.begin_at(0, y);

					for (size_t x = 0; x < m_slice_size.x; ++x) {
						if (osi_i[x] > si_i[x]) {
							osi_i[x] = si_i[x];
							omu_i[x] = mu_i[x];
						}
					}
				}

			}

	}

	T2DImage<T> *result = new T2DImage<T>(m_slice_size);
	convert(mu_result.begin(), mu_result.end(), result->begin());

#if 0
				cout << "result:";
				copy(result->begin(),  result->end(),
				     ostream_iterator<float>(cout, " "));
				cout << "\n";

#endif


	return result;
}

static void do_evaluate(C2DFImage::const_iterator ni, C2DFImage::const_iterator ne,
		     C2DFImage::iterator mu, C2DFImage::iterator sigma)
{
	while (ni != ne) {
		const float n = *ni;
		const float muq = *mu / *ni;

		if (n > 1.0) {
			*sigma = (*sigma - muq * muq * n) / (n - 1.0f);
		}else {
			*sigma = 0.0f;
		}

		*mu++ = muq;
		++ni;
		++sigma;
	}
}

template <typename Iterator>
void rotate(Iterator begin, Iterator end)
{
	typename Iterator::value_type help = *(end - 1);
	copy_backward(begin, end - 1, end);
	*begin = help;
}

void C2DMLVnFifoFilter::shift_buffer()
{
	TRACE_FUNCTION; 
	rotate(m_mu_buffer.begin(), m_mu_buffer.end());
	rotate(m_sigma_buffer.begin(),m_sigma_buffer.end());
	rotate(m_n.begin(), m_n.end());
}

void C2DMLVnFifoFilter::evaluate(size_t slice)
{
	TRACE_FUNCTION; 
	do_evaluate(m_n[slice].begin(),
		    m_n[slice].end(),
		    m_mu_buffer[slice].begin(),
		    m_sigma_buffer[slice].begin());

}

// this operator add the new slice

template <typename T>
C2DImage *C2DMLVnFifoFilter::operator()(const T2DImage<T>& input)
{
	TRACE("C2DMLVnFifoFilter::operator()(const T2DImage<T>& input)(push)");

	fill(m_mu_buffer[0].begin(), m_mu_buffer[0].end(), 0.0);
	fill(m_sigma_buffer[0].begin(), m_sigma_buffer[0].end(), 0.0);
	fill(m_n[0].begin(), m_n[0].end(), 0.0);

	for (size_t y = 0; y < m_slice_size.y; ++y) {
		copy(input.begin_at(0,y), input.begin_at(0,y + 1), m_buf1.begin());
		transform(m_buf1.begin(), m_buf1.end(), m_buf2.begin(), 
			  [](float x) {return x * x;}); 

		for (size_t iz = 0; iz < m_w; ++iz)
			for (size_t iy = 0; iy < m_w; ++iy)
				for (size_t ix = 0; ix < m_w; ++ix) {
					transform(m_buf1.begin(), m_buf1.end(),
						  m_mu_buffer[iz].begin_at(ix, y + iy),
						  m_mu_buffer[iz].begin_at(ix, y + iy), 
						  [](float x, float y){return x+y;}); 
					transform(m_buf2.begin(), m_buf2.end(),
						  m_sigma_buffer[iz].begin_at(ix, y + iy),
						  m_sigma_buffer[iz].begin_at(ix, y + iy), 
						  [](float x, float y){return x+y;}
						);
				}
	}

	C2DFImage::const_iterator ntmpl_b = m_n_template.begin();
	C2DFImage::const_iterator ntmpl_e = m_n_template.end();

	for (size_t iz = 0; iz < m_w; ++iz) {
		transform(ntmpl_b, ntmpl_e,
			  m_n[iz].begin_at(0, 0),
			  m_n[iz].begin_at(0, 0), [](float x, float y){return x+y;});
	}
	return NULL;
}

void C2DMLVnFifoFilter::post_finalize()
{
	TRACE_FUNCTION; 
	m_mu_buffer.resize(0);
	m_sigma_buffer.resize(0);
	m_n.resize(0);
}

void C2DMLVnFifoFilter::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMLVnFifoFilter::do_initialize");


	m_slice_size = x->get_size();
	C2DBounds size(m_slice_size.x + 2 * m_hw,
		       m_slice_size.y + 2 * m_hw);

	size_t n_slices = get_buffer_size();

	m_mu_buffer = vector<C2DFImage>(n_slices);
	m_sigma_buffer = vector<C2DFImage>(n_slices);
	m_n = vector<C2DFImage>(n_slices);

	for (size_t i = 0; i < n_slices; ++i) {
		m_mu_buffer[i] = C2DFImage(size);
		m_sigma_buffer[i] = C2DFImage(size);
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
	return "mean least variance stack filter";
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

