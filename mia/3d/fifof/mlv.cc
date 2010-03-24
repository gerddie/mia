/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2008 - 2010
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

/*! \brief basic type of a plugin handler

A median filter for stacks of 2D images

\author Gert Wollny <wollny at eva.mpg.de>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iomanip>
#include <limits>

#include <boost/cast.hpp>
#include <boost/lambda/lambda.hpp>

#include <mia/3d/fifof/mlv.hh>

NS_BEGIN(mlv_2dstack_filter)

NS_MIA_USE
using namespace std;
using namespace boost::lambda;
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
	_M_hw(hwidth),
	_M_w(2 * hwidth + 1),
	_M_read_start(2 * hwidth)
{
}

// This operator reads out the result

template <typename T>
C2DImage *C2DMLVnFifoFilter::operator()(const T3DImage<T>& /*dummy*/) const
{
	TRACE("C2DMLVnFifoFilter::operator() (pull)");
	C2DFImage mu_result(_M_slice_size);
	C2DFImage sigma_buffer(_M_slice_size);

	fill(sigma_buffer.begin(), sigma_buffer.end(), numeric_limits<float>::max());

	for (size_t z = get_start(); z < get_end(); ++z) {
		for (size_t iy = 0; iy < _M_w; ++iy)
			for (size_t ix = 0; ix <= _M_w; ++ix) {
				for (size_t y = 0; y < _M_slice_size.y; ++y) {
					C3DFImage::const_iterator mu_i = _M_mu_buffer[z].begin_at(ix, y + iy);
					C3DFImage::const_iterator si_i = _M_sigma_buffer[z].begin_at(ix, y + iy);
					C2DFImage::iterator omu_i = mu_result.begin_at(0, y);
					C2DFImage::iterator osi_i = sigma_buffer.begin_at(0, y);

					for (size_t x = 0; x < _M_slice_size.x; ++x) {
						if (osi_i[x] > si_i[x]) {
							osi_i[x] = si_i[x];
							omu_i[x] = mu_i[x];
						}
					}
				}

			}

	}

	T2DImage<T> *result = new T2DImage<T>(_M_slice_size);
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
	TRACE("C2DMLVnFifoFilter::shift_buffer()");

	rotate(_M_mu_buffer.begin(), _M_mu_buffer.end());
	rotate(_M_sigma_buffer.begin(),_M_sigma_buffer.end());
	rotate(_M_n.begin(), _M_n.end());


}

void C2DMLVnFifoFilter::evaluate(size_t slice)
{
	cvdebug() << "Evaluate slice " << slice << "\n";
#if 0
	cout << "mu:";
	copy(_M_mu_buffer[slice].begin(),  _M_mu_buffer[slice].end(),
	     ostream_iterator<float>(cout, " "));
	cout << "\n";

	cout << "sigma:";
	copy(_M_sigma_buffer[slice].begin(),  _M_sigma_buffer[slice].end(),
	     ostream_iterator<float>(cout, " "));
	cout << "\n";
	cout << "N:";
	copy(_M_n[slice].begin(),  _M_n[slice].end(),
	     ostream_iterator<float>(cout, " "));
	cout << "\n";

	cout << "\n";
#endif
	do_evaluate(_M_n[slice].begin(),
		    _M_n[slice].end(),
		    _M_mu_buffer[slice].begin(),
		    _M_sigma_buffer[slice].begin());

}

// this operator add the new slice

template <typename T>
C2DImage *C2DMLVnFifoFilter::operator()(const T2DImage<T>& input)
{
	TRACE("C2DMLVnFifoFilter::operator()(const T2DImage<T>& input)(push)");

	fill(_M_mu_buffer[0].begin(), _M_mu_buffer[0].end(), 0.0);
	fill(_M_sigma_buffer[0].begin(), _M_sigma_buffer[0].end(), 0.0);
	fill(_M_n[0].begin(), _M_n[0].end(), 0.0);

	for (size_t y = 0; y < _M_slice_size.y; ++y) {
		copy(input.begin_at(0,y), input.begin_at(0,y + 1), _M_buf1.begin());
		transform(_M_buf1.begin(), _M_buf1.end(), _M_buf2.begin(), _1 * _1);

		typename T2DImage<T>::const_iterator is = input.begin_at(0,y);
		typename T2DImage<T>::const_iterator ie = input.begin_at(0,y + 1);

		for (size_t iz = 0; iz < _M_w; ++iz)
			for (size_t iy = 0; iy < _M_w; ++iy)
				for (size_t ix = 0; ix < _M_w; ++ix) {
					transform(_M_buf1.begin(), _M_buf1.end(),
						  _M_mu_buffer[iz].begin_at(ix, y + iy),
						  _M_mu_buffer[iz].begin_at(ix, y + iy), _1 + _2);
					transform(_M_buf2.begin(), _M_buf2.end(),
						  _M_sigma_buffer[iz].begin_at(ix, y + iy),
						  _M_sigma_buffer[iz].begin_at(ix, y + iy), _1 + _2);
				}
	}

	C2DFImage::const_iterator ntmpl_b = _M_n_template.begin();
	C2DFImage::const_iterator ntmpl_e = _M_n_template.end();

	for (size_t iz = 0; iz < _M_w; ++iz) {
		transform(ntmpl_b, ntmpl_e,
			  _M_n[iz].begin_at(0, 0),
			  _M_n[iz].begin_at(0, 0), _1 + _2);
	}
#if 0
	cout << "sum:";
	copy(_M_mu_buffer[0].begin(),  _M_mu_buffer[1].end(),
	     ostream_iterator<float>(cout, " "));
	cout << "\n";

	cout << "sum2:";
	copy(_M_sigma_buffer[0].begin(),  _M_sigma_buffer[0].end(),
	     ostream_iterator<float>(cout, " "));
	cout << "\n";
	cout << "N:";
	copy(_M_n[0].begin(),  _M_n[0].end(),
	     ostream_iterator<float>(cout, " "));
	cout << "\n";

	cout << "\n";
#endif
	return NULL;
}

void C2DMLVnFifoFilter::post_finalize()
{
	TRACE("C2DMLVnFifoFilter::post_finalize()");

	_M_mu_buffer.resize(0);
	_M_sigma_buffer.resize(0);
	_M_n.resize(0);
}

void C2DMLVnFifoFilter::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMLVnFifoFilter::do_initialize");


	_M_slice_size = x->get_size();
	C2DBounds size(_M_slice_size.x + 2 * _M_hw,
		       _M_slice_size.y + 2 * _M_hw);

	size_t n_slices = get_buffer_size();

	_M_mu_buffer = vector<C2DFImage>(n_slices);
	_M_sigma_buffer = vector<C2DFImage>(n_slices);
	_M_n = vector<C2DFImage>(n_slices);

	for (size_t i = 0; i < n_slices; ++i) {
		_M_mu_buffer[i] = C2DFImage(size);
		_M_sigma_buffer[i] = C2DFImage(size);
		_M_n[i] = C2DFImage(size);
	}

	_M_buf_slice_size = size.x * size.y;

	_M_buf1.resize(_M_slice_size.x);
	_M_buf2.resize(_M_slice_size.x);
	_M_prototype.reset(create_buffer(C2DBounds(1,1), 1, x->get_pixel_type()));


	_M_n_template = C2DFImage(C2DBounds(_M_slice_size.x + 2 * _M_hw, _M_slice_size.y + 2 * _M_hw));
	// create the pattern that is used to update the count
	// this is the slow version, but we have to do it only once
	for (size_t y = 0; y < _M_slice_size.y; ++y)
		for (size_t x = 0; x < _M_slice_size.x; ++x)
			for (size_t iy = 0; iy < 2 * _M_hw + 1; ++iy)
				for (size_t ix = 0; ix < 2 * _M_hw + 1; ++ix)
					_M_n_template(x + ix, y + iy) += 1.0;
}

void C2DMLVnFifoFilter::do_push(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMLVnFifoFilter::do_push");
	mia::accumulate(*this, *x);
}

P2DImage C2DMLVnFifoFilter::do_filter()
{
	TRACE("C2DMLVnFifoFilter::do_filter");
	return P2DImage(mia::filter(*this, *_M_prototype));
}


class C2DMLVnFifoFilterPlugin : public C2DFifoFilterPlugin {
public:
	C2DMLVnFifoFilterPlugin();
private:

	virtual const string do_get_descr() const;
	virtual bool do_test() const;
	virtual C2DFifoFilterPlugin::ProductPtr do_create()const;

	mutable int _M_hw;
};

C2DMLVnFifoFilterPlugin::C2DMLVnFifoFilterPlugin():
	C2DFifoFilterPlugin("mlv"),
	_M_hw(1)
{
	add_parameter("w", new CIntParameter(_M_hw, 0, numeric_limits<int>::max(),
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

C2DFifoFilterPlugin::ProductPtr C2DMLVnFifoFilterPlugin::do_create()const
{
	return ProductPtr(new C2DMLVnFifoFilter(_M_hw));
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{

	return new C2DMLVnFifoFilterPlugin();
}

NS_END

