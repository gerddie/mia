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

/*! \brief basic type of a plugin handler 

A 3D weighted median filter plugin 

\file gauss_image3d_filter.hh
\author Gert Wollny <wollny at eva.mpg.de>

*/

#include <mia/2d/2dfilter.hh>

namespace wmean_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const * plugin_name = "wmean";
static const CIntOption param_width("hw", "half window width w, actual window will be (2*w+1)", 1, 1, 256);
static const CFloatOption param_weight("weight", "weigt of the surrounding pixels", 0.7, 0.01, 2.0);

class C2DWMean : public C2DFilter{
	int _M_width; 
	float _M_weight; 
public:
	C2DWMean(int hwidth, float weight):
		_M_width(hwidth), 
		_M_weight(weight)
	{
	}
	
	template <class Data2D>
	typename C2DWMean::result_type operator () (const Data2D& data) const ;

};


class CWMean2DImageFilter: public C2DImageFilterBase {
	C2DWMean _M_filter; 
public:
	CWMean2DImageFilter(int hwidth, float weight);

	virtual P2DImage do_filter(const C2DImage& image) const;
};


class CWMean2DImageFilterFactory: public C2DFilterPlugin {
public: 
	CWMean2DImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
	virtual int do_test() const; 
};

template <class Data2D>
typename C2DWMean::result_type C2DWMean::operator () (const Data2D& data) const
{
	Data2D *result = new Data2D(data.get_size()); 


	typename Data2D::iterator i = result->begin(); 
	typename Data2D::const_iterator t = data.begin(); 

	cvdebug() << "filter with width = " << _M_width << " and weight " << _M_weight << endl; 

	for (int y = 0; y < (int)data.get_size().y; ++y)
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i, ++t) {
			
			float sum = 0.0; 
			int num = -1; 
			
			
			for (int iy = max(0, y - _M_width); 
			     iy < min(y + _M_width + 1, (int)data.get_size().y);  ++iy)
				
				for (int ix = max(0, x - _M_width); 
				     ix < min(x + _M_width + 1, (int)data.get_size().x);  ++ix) {
					
					sum += data(ix,iy); 
					++num; 
				}
			*i = (typename Data2D::value_type) ( (*t + (_M_weight / num) * (sum - *t)) / (1 + _M_weight));
		}
	return P2DImage(result); 
}

CWMean2DImageFilter::CWMean2DImageFilter(int hwidth, float weight):
	_M_filter(hwidth, weight)
{
}

P2DImage CWMean2DImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(_M_filter,image); 
}

CWMean2DImageFilterFactory::CWMean2DImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_width);
	add_help(param_weight);
}

C2DFilterPlugin::ProductPtr CWMean2DImageFilterFactory::create(const CParsedOptions& options) const
{
	int hwidth = param_width.get_value(options); 
	float weight = param_weight.get_value(options); 

	return C2DFilterPlugin::ProductPtr(new CWMean2DImageFilter(hwidth, weight)); 
}

const string CWMean2DImageFilterFactory::do_get_descr()const
{
	return "2D weighted mean image filter"; 
}


int CWMean2DImageFilterFactory::do_test() const
{
	const size_t size_x = 3; 
	const size_t size_y = 3;
	const int width = 1; 
	const float alpha = 0.7; 
	
	const float src[size_y][size_x] =
		{{ 0, 1, 2}, 
		 { 2, 5, 5},
		 { 1, 7, 1}}; 
	
	
	// "hand filtered" w = 1 -> 3x3
	const float src_ref[size_y][size_x] = 
		{{     0.7 *  8/3, 1 + 0.7 * 14/5, 2 + 0.7 * 11/3}, 
		 { 2 + 0.7 * 14/5, 5 + 0.7 * 19/8, 5 + 0.7 * 16/5},
		 { 1 + 0.7 * 14/3, 7 + 0.7 * 14/5, 1 + 0.7 * 17/3}}; 
	
	C2DBounds size(size_x, size_y);
	
	C2DFImage *src_img = new C2DFImage(size); 
	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x) 
			(*src_img)(x,y) = src[y][x]; 
	
	CWMean2DImageFilter wmean(width, alpha);
	
	P2DImage src_wrap(src_img); 
	
	P2DImage res_wrap = wmean.filter(src_wrap); 
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
			if (::fabs(1.7 * (*res_img)(x,y) - src_ref[y][x]) > 0.0001) {
				cvfail() << "images different at(" << x << "," << y << "): got "<< 1.7 * (*res_img)(x,y) <<
					" expected " << src_ref[y][x] << "\n"; 
				return -1; 
			}
	
	return 1; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CWMean2DImageFilterFactory(); 
}

} // end namespace wmean_2dimage_filter
