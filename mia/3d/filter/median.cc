/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Biomedical Image Technologies, Universidad Politecnica de Madrid
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
   LatexBeginPlugin{3D image filters}
   
   \subsection{Median filter}
   \label{filter3d:median}
   
   \begin{description}
   
   \item [Plugin:] median
   \item [Description:] Apply the median filter. 
   \item [Input:] Abitrary gray scale or binary image 
   \item [Output:] The filtered image of the same pixel type and dimension 
   
   \plugtabstart
   w &  int & filter width parameter & 1  \\
   \plugtabend
   
   \end{description}

   LatexEnd  
 */


#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/median.hh>

#include <limits>

NS_BEGIN(median_3dimage_filter)
NS_MIA_USE;
using namespace std;
using namespace boost;

template < typename T>
struct __dispatch_median_3dfilter {
	static T  apply(const T3DImage<T>& data, int x, int y, int z, int width, vector<T>& target_vector) {
		int idx = 0;

		typename vector<T>::iterator tend = target_vector.begin();

		for (int iz = max(0, z - width);
		     iz < min(z + width + 1, (int)data.get_size().z);  ++iz)
			for (int iy = max(0, y - width);
			     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)

				for (int ix = max(0, x - width);
				     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
					++tend;
					target_vector[idx++] = data(ix,iy,iz);
				}

		if (idx & 1) {
			typename vector<T>::iterator mid = target_vector.begin() + idx/2;
			nth_element(target_vector.begin(), mid, tend);
			return *mid;
		} else {
			cvdebug() << "idx even\n";
			sort(target_vector.begin(), tend);
			return (target_vector[idx/2 - 1] + target_vector[idx/2])/2;
		}
	}
};

template <>
struct __dispatch_median_3dfilter<C3DBitImage> {
	static C3DBitImage::value_type apply(C3DBitImage& data, int x, int y, int z, int width,
					     vector<C3DBitImage::value_type>& /*target_vector*/) {
		int trues = 0;
		int falses = 0;
		for (int iz = max(0, z - width);
		     iz < min(z + width + 1, (int)data.get_size().z);  ++iz)
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
P3DImage C3DMedianFilter::operator () (const T3DImage<T>& data) const
{
	T3DImage<T> *result = new T3DImage<T>(data.get_size(), data.get_attribute_list());


	typename T3DImage<T>::iterator i = result->begin();

	vector<T> target_vector((2 * m_width + 1) * (2 * m_width + 1) * (2 * m_width + 1));

	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i)
				*i = __dispatch_median_3dfilter<T>::apply(data, x, y, z, m_width, target_vector);

	return P3DImage(result);
}

C3DMedianFilter::C3DMedianFilter(int hwidth):
	m_width(hwidth)
{
}

P3DImage C3DMedianFilter::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}

bool  C3DMedianFilterFactory::do_test() const
{
	return false;
}

C3DMedianFilterFactory::C3DMedianFilterFactory():
	C3DFilterPlugin("median"),
	m_hw(1)
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(), false, "filter width parameter"));
}

const string  C3DMedianFilterFactory::do_get_descr() const
{
	return "median 3d filter";
}

C3DFilterPlugin::ProductPtr C3DMedianFilterFactory::do_create() const
{
	return C3DFilterPlugin::ProductPtr(new C3DMedianFilter(m_hw));
}



C3DSaltAndPepperFilter::C3DSaltAndPepperFilter(int hwidth, float thresh):
	m_width(hwidth),
	m_thresh(thresh)
{
}

template <class T>
P3DImage C3DSaltAndPepperFilter::operator () (const T3DImage<T>& data) const
{
	T3DImage<T> *result = new T3DImage<T>(data);
	typename T3DImage<T>::const_iterator inp = data.begin();

	typename T3DImage<T>::iterator i = result->begin();

	vector<T> target_vector((2 * m_width + 1) * (2 * m_width + 1) * (2 * m_width + 1));

	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i, ++inp) {
				T res = __dispatch_median_3dfilter<T>::apply(data, x, y, z, m_width, target_vector);
				float delta = ::fabs((double)(res - *inp));
				if (delta > m_thresh)
					*i = res;
			}
	return P3DImage(result);
}


P3DImage C3DSaltAndPepperFilter::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DSaltAndPepperFilterFactory::C3DSaltAndPepperFilterFactory():
	C3DFilterPlugin("sandp"),
	m_hw(1),
	m_thresh(100)
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(), false, "filter width parameter"));
	add_parameter("thresh", new CFloatParameter(m_thresh, 0, numeric_limits<float>::max(), false, "thresh value"));
}

C3DFilterPlugin::ProductPtr C3DSaltAndPepperFilterFactory::do_create()const
{
	return C3DFilterPlugin::ProductPtr(new C3DSaltAndPepperFilter(m_hw, m_thresh));
}
const string  C3DSaltAndPepperFilterFactory::do_get_descr() const
{
	return "salt and pepper  3d filter";
}

bool  C3DSaltAndPepperFilterFactory::do_test() const
{
	return false;
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *median = new C3DMedianFilterFactory();
	median->append_interface(new C3DSaltAndPepperFilterFactory());
	return median;
}

}

