/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
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

#include <limits>
#include <mia/2d/2dfilter.hh>

namespace midpoint_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const * plugin_name = "midpoint";
static const CIntOption param_hw("w", "half filter width", 0, 1, numeric_limits<int>::max()); 

class C2DMidpoint : public C2DFilter{
public:
	C2DMidpoint(int hw); 

	template <class Data2D>
	typename C2DMidpoint::result_type operator () (const Data2D& data) const ;
private: 
	int m_hw; 
};



class C2DMidpointImageFilter: public C2DImageFilterBase {
	C2DMidpoint m_filter; 
public:
	C2DMidpointImageFilter(int hw);

	virtual P2DImage do_filter(const C2DImage& image) const;
};

class C2DMidpointImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DMidpointImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
};

C2DMidpoint::C2DMidpoint(int hw):
	m_hw(hw)
{
}

template < class Data2D>
struct __dispatch_filter {
	static typename Data2D::value_type  apply(const Data2D& data, int x, int y, int width, 
						  vector<typename Data2D::value_type>& target_vector) {
		int idx = 0;
		
		typename vector<typename Data2D::value_type>::iterator tend = target_vector.begin(); 
		
		for (int iy = max(0, y - width); 
		     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
			for (int ix = max(0, x - width); 
			     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
				++tend; 
				target_vector[idx++] = data(ix,iy);
			}
			
		sort(target_vector.begin(), tend); 
		return (target_vector[0] + target_vector[idx - 1])/2; 
	}
};	

template <>
struct __dispatch_filter<C2DBitImage> {
	static C2DBitImage::value_type apply(const C2DBitImage& data, int x, int y, int width, 
					     vector<C2DBitImage::value_type>& target_vector) {
		int trues = 0; 
		int falses = 0; 
		for (int iy = max(0, y - width); 
		     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
			for (int ix = max(0, x - width); 
			     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
				if (data(ix, iy)) 
					++trues;
				else
					++falses; 
			}
		return trues > falses;
	}
};


template <class Data2D>
typename C2DMidpoint::result_type C2DMidpoint::operator () (const Data2D& data) const
{
	cvdebug() << "C2DMidpoint::operator () begin\n";

	Data2D *result = new Data2D(data.get_size()); 

	vector<typename Data2D::value_type> target_vector((2 * m_hw + 1) * 
							       (2 * m_hw + 1));

	typename Data2D::iterator i = result->begin(); 
	
	for (int y = 0; y < (int)data.get_size().y; ++y) 
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i)
			*i = __dispatch_filter<Data2D>::apply(data, x, y, m_hw, target_vector);  
	
	cvdebug() << "C2DMidpoint::operator () end\n";
	return P2DImage(result); 
}

C2DMidpointImageFilter::C2DMidpointImageFilter(int hw):
		m_filter(hw)
{
}

P2DImage C2DMidpointImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(m_filter, image); 
}


C2DMidpointImageFilterFactory::C2DMidpointImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_hw); 
}

C2DFilterPlugin::ProductPtr C2DMidpointImageFilterFactory::create(const CParsedOptions& options) const
{
	int  hw = param_hw.get_value(options); 

	return C2DFilterPlugin::ProductPtr(new C2DMidpointImageFilter(hw)); 
}

const string C2DMidpointImageFilterFactory::do_get_descr()const
{
	return "2D image midpoint filter"; 
}


int C2DMidpointImageFilterFactory::do_test() const
{
	const size_t size_x = 7; 
	const size_t size_y = 5;
	
	const float src[size_y][size_x] =
		{{ 0, 1, 2, 3, 2, 3, 5}, 
		 { 2, 5, 2, 3, 5, 3, 2},
		 { 1, 2, 7, 6, 4, 2, 1},
		 { 3, 4, 4, 3, 4, 3, 2},
		 { 1, 3, 2, 4, 5, 6, 2}}; 
	
	// "hand filtered" w = 1 -> 3x3
	const float src_ref[size_y][size_x] = 
		{{ 2.5, 2.5, 3.0, 3.5, 3.5, 3.5, 3.5}, 
		 { 2.5, 3.5, 4.0, 4.5, 4.0, 3.0, 3.0},
		 { 3.0, 4.0, 4.5, 4.5, 4.0, 3.0, 2.0},
		 { 2.5, 4.0, 4.5, 4.5, 4.0, 3.5, 3.5},
		 { 2.5, 2.5, 3.0, 3.5, 4.5, 4.0, 4.0}};
	
	
	C2DBounds size(size_x, size_y);
	
	C2DFImage *src_img = new C2DFImage(size); 
	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x) 
			(*src_img)(x,y) = src[y][x]; 
	
	C2DMidpointImageFilter midpoint(1); 
	
	P2DImage src_wrap(src_img); 
	
	P2DImage res_wrap = midpoint.filter(src_wrap); 
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
			if ((*res_img)(x,y) != src_ref[y][x]) {
				cvfail() << "images different at(" << x << "," << y << "): got "<< (*res_img)(x,y) <<
					" expected " << src_ref[y][x] << "\n"; 
				return -1; 
			}
	
	return 1; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DMidpointImageFilterFactory(); 
}

} // end namespace midpoint_2dimage_filter
