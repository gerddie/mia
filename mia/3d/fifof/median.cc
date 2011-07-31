/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

/* 
   LatexBeginPluginDescription{2D image stack filters}
   
   \subsection{Median}
   \label{fifof:median}
   
   \begin{description}
   
   \item [Plugin:] median
   \item [Description:] Runs a median filter on the imput images. 
   \item [Input:] Gray scale or binary images, all of the same size and pixel type  
   \item [Output:] The filtered image(s) 
   
   \plugtabstart
   w & int & filter width parameter, the actual filter captures a neighborhood of 
                 $(2 * w + 1) \times (2 * w + 1) \times (2 * w + 1)$ voxels & 1 \\ 
   \plugtabend
   
   \end{description}

   LatexEnd  
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iomanip>
#include <limits>
#include <mia/3d/fifof/median.hh>

NS_BEGIN(median_2dstack_filter)

NS_MIA_USE
using namespace std;

template < typename T>
struct __dispatch_median_3dfilter {
	static T  apply(const T3DImage<T>& data, int x, int y, int start, int end, int width, vector<T>& target_vector) {
		int idx = 0;

		typename vector<T>::iterator tend = target_vector.begin();

		for (int iz = start; iz < end;  ++iz)
			for (int iy = max(0, y - width);
			     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
				for (int ix = max(0, x - width);
				     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
					*tend++ = data(ix,iy,iz);
					++idx;
				}

		if (idx & 1) {
			typename vector<T>::iterator mid = target_vector.begin() + idx/2;
			nth_element(target_vector.begin(), mid, tend);
			return *mid;
		} else {
			sort(target_vector.begin(), tend);
			return (target_vector[idx/2 - 1] + target_vector[idx/2])/2;
		}
	}
};

template <>
struct __dispatch_median_3dfilter<C3DBitImage> {
	static C3DBitImage::value_type apply(C3DBitImage& data, int x, int y, int start, int end, int width,
					     vector<C3DBitImage::value_type>& /*target_vector*/) {
		int trues = 0;
		int falses = 0;
		for (int iz = start; iz < end;  ++iz)
			for (int iy = max(0, y - width);
			     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
				for (int ix = max(0, x - width);
				     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
					if (data(ix, iy, iz))
						++trues;
					else
						++falses;
				}
		return trues > falses;
	}
};

template <typename T>
C2DImage *C2DMedianFifoFilter::operator()(const T3DImage<T>& buffer) const {

	cvdebug() << "filter z range: " << get_start() << ":" << get_end() << "\n";

	T2DImage<T> *retval = new T2DImage<T>(m_slice_size);
	// do the actual filtering


	vector<T> target_vector((2 * m_hw + 1) * (2 * m_hw + 1) * (2 * m_hw + 1));

	typename T2DImage<T>::iterator i = retval->begin();

	for (size_t y = 0; y < m_slice_size.y; ++y)
		for (size_t x = 0; x < m_slice_size.x; ++x, ++i)
			*i = __dispatch_median_3dfilter<T>::apply(buffer, x, y,
								  get_start(),
								  get_end(),
								  m_hw, target_vector);

	return retval;
}



template <typename T>
C2DImage *C2DMedianFifoFilter::operator()(const T2DImage<T>& input) {
	TRACE("push internal");
	T3DImage<T> *buf = dynamic_cast<T3DImage<T> *>(m_buffer.get());
	if (!buf) {
		throw invalid_argument("C2DMedianFifoFilter: input images use different pixel types");
	}
	copy(input.begin(), input.end(), buf->begin());
	return NULL;
}

C2DMedianFifoFilter::C2DMedianFifoFilter(size_t hwidth):
	C2DImageFifoFilter(2 * hwidth + 1, hwidth + 1, 0),
	m_hw(hwidth)

{
}

void C2DMedianFifoFilter::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMedianFifoFilter::do_initialize");
	m_slice_size = x->get_size();
	m_buffer.reset(create_buffer(m_slice_size, 2 * m_hw + 1, x->get_pixel_type()));
}

void C2DMedianFifoFilter::do_push(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DMedianFifoFilter::do_push");
	mia::accumulate(*this, *x);
}

void C2DMedianFifoFilter::shift_buffer()
{
	mia::filter_inplace(m_ss, *m_buffer);
}

P2DImage C2DMedianFifoFilter::do_filter()
{
	TRACE("C2DMedianFifoFilter::do_filter");

	return P2DImage(mia::filter(*this, *m_buffer));
}


class C2DMedianFifoFilterPlugin : public C2DFifoFilterPlugin {
public:
	C2DMedianFifoFilterPlugin();
private:

	virtual const string do_get_descr() const;
	virtual bool do_test() const;
	virtual C2DImageFifoFilter *do_create()const;

	int m_hw;
};

C2DMedianFifoFilterPlugin::C2DMedianFifoFilterPlugin():
	C2DFifoFilterPlugin("median"),
	m_hw(1)
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(),
					     false, "filter width parameter"));
}

const string C2DMedianFifoFilterPlugin::do_get_descr() const
{
	return "median 2D fifo filter";
}

typedef TFifoFilterSink<C2DImage> C2DImageFifoFilterSink;
bool C2DMedianFifoFilterPlugin::do_test() const
{
	return true;
}

C2DImageFifoFilter *C2DMedianFifoFilterPlugin::do_create()const
{
	return new C2DMedianFifoFilter(m_hw);
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{

	return new C2DMedianFifoFilterPlugin();
}
NS_END

