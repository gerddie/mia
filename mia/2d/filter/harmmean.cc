/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <mia/2d/filter.hh>

namespace harmmean_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const * plugin_name = "harmmean";
static const CIntOption param_width("w", "half window width w, actual window will be (2*w+1)", 1, 1, 256);
	
class C2DHarmonicMean : public C2DFilter{
	int m_width; 
public:
	C2DHarmonicMean(int hwidth):
		m_width(hwidth)
	{
	}
	
	template <class T>
	typename C2DHarmonicMean::result_type operator () (const T2DImage<T>& data) const;
private: 
	
	template <class T>
	T eval(int x, int y, const T2DImage<T>& data)const; 

};


class CHarmonicMean2DImageFilter: public C2DImageFilterBase {
	C2DHarmonicMean m_filter; 
public:
	CHarmonicMean2DImageFilter(int hwidth);

	virtual P2DImage do_filter(const C2DImage& image) const;
};


class CHarmonicMean2DImageFilterFactory: public C2DFilterPlugin {
public: 
	CHarmonicMean2DImageFilterFactory();
	virtual C2DFilter *create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
};
	
template <class T>
T C2DHarmonicMean::eval(int x, int y, const T2DImage<T>& data)const
{
	double sum = 0.0; 
	int num = 0; 
	
	for (int iy = max(0, y - m_width); 
	     iy < min(y + m_width + 1, (int)data.get_size().y);  ++iy)
		
		for (int ix = max(0, x - m_width); 
		     ix < min(x + m_width + 1, (int)data.get_size().x);  ++ix) {
			double val = data(ix,iy); 
			if (!val) 
				return T(); 
			
			sum +=  1.0 / val; 
			++num;
		}
	return (T) (sum != 0 ? num / sum: 0);
}

template <class T>
typename C2DHarmonicMean::result_type C2DHarmonicMean::operator () (const T2DImage<T>& data) const
{
	T2DImage<T> *result = new T2DImage<T>(data.get_size()); 


	typename T2DImage<T>::iterator i = result->begin(); 

	cvdebug() << "filter with width = " << m_width <<  endl; 

	for (int y = 0; y < (int)data.get_size().y; ++y)
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i) {
			*i = eval(x,y,data); 
			
			
	
		}
	return P2DImage(result); 
}

CHarmonicMean2DImageFilter::CHarmonicMean2DImageFilter(int hwidth):
	m_filter(hwidth)
{
}

P2DImage CHarmonicMean2DImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(m_filter,image); 
}

CHarmonicMean2DImageFilterFactory::CHarmonicMean2DImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_width);
}

C2DFilter *CHarmonicMean2DImageFilterFactory::create(const CParsedOptions& options) const
{
	int hwidth = param_width.get_value(options); 

	return new CHarmonicMean2DImageFilter(hwidth); 
}

const string CHarmonicMean2DImageFilterFactory::do_get_descr()const
{
	return "2D harmonic mean image filter"; 
}


int CHarmonicMean2DImageFilterFactory::do_test() const
{
	const size_t size_x = 3; 
	const size_t size_y = 3;
	const int width = 1; 
	
	const float src[size_y][size_x] =
		{{ 0, 1, 2}, 
		 { 2, 5, 5},
		 { 1, 4, 1}}; 
	
	
	// "hand filtered" w = 1 -> 3x3
	const float src_ref[size_y][size_x] = {
		{ 0,  0, 4.0 / 1.9}, 
		{ 0,  0, 6.0 / 3.15}, 
		{ 4.0 / 1.95, 6.0/ 3.15, 4.0 / 1.65}
	};
	
	C2DBounds size(size_x, size_y);
	
	C2DFImage *src_img = new C2DFImage(size); 
	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x) 
			(*src_img)(x,y) = src[y][x]; 
	
	CHarmonicMean2DImageFilter HarmonicMean(width);
	
	P2DImage src_wrap(src_img); 
	
	P2DImage res_wrap = HarmonicMean.filter(src_wrap); 
	C2DFImage *res_img = res_wrap.getC2DFImage(); 
	
	if (!res_img) {
		cvfail() << get_name() << "wrong return type\n"; 
		return -1; 
	}
	
	if (res_img->get_size() != src_img->get_size()) {
		cvfail() << get_name() << "wrong return size\n"; 
		return -1; 
	}

	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x) 
			if (::fabs((*res_img)(x,y) - src_ref[y][x]) > 0.0001) {
				cvfail() << "images different at(" << x << "," << y << "): got "<< (*res_img)(x,y) <<
					" expected " << src_ref[y][x] << "\n"; 
				return -1; 
			}
	
	return 1; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CHarmonicMean2DImageFilterFactory(); 
}

} // end namespace HarmonicMean_2dimage_filter
