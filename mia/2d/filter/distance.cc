/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#include <mia/2d/2dfilter.hh>
#include <libmona/filter.hh>

namespace distance_2dimage_filter {
NS_MIA_USE;
using namespace std; 

#define SCALE 256	
static char const *plugin_name = "distance";
	
static const CFloatOption param_hw("scale", "scaling factor for fixed point storage", SCALE, 16, 1024); 
	
class C2DDistance: public C2DFilter {
public:
	C2DDistance(float scale):m_scale(scale){}
	
	template <class T>
	typename C2DDistance::result_type operator () (const T2DImage<T>& data) const ;
private: 
	float m_scale; 

};


class C2DDistanceImageFilter: public C2DImageFilterBase {
	C2DDistance m_filter; 
public:
	C2DDistanceImageFilter(float scale):m_filter(scale){}; 
	
	virtual P2DImage do_filter(const C2DImage& image) const;
};


class C2DDistanceImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DDistanceImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
	virtual int do_test() const; 
};

template <class T>
typename C2DDistance::result_type C2DDistance::operator() (const T2DImage<T>& image) const 
{
	C2DUSImage *result = new C2DUSImage(image.get_size()); 
	fill(result->begin(),   result->end(), numeric_limits<C2DUSImage::value_type>::max()); 
	
	// brute force approach, should be a better way ...
	typename T2DImage<T>::const_iterator i = image.begin(); 
	for (size_t yi = 0; yi < image.get_size().y; ++yi) {
		for (size_t xi = 0; xi < image.get_size().x; ++xi, ++i) {
			if (!*i) 
				continue; 
			C2DUSImage::iterator r = result->begin(); 
			for (size_t yr = 0; yr < image.get_size().y; ++yr) {
				double dy2 = double(yr) - yi; 
				dy2 *= dy2; 
				for (int xr = 0; xr < (int)image.get_size().x; ++xr, ++r) {
					double min_d = *r / m_scale; 
					min_d *= min_d; 
					double dx = double(xi) - xr; 
					double d = dx * dx + dy2; 
					if (min_d > d) {
						d = m_scale * sqrt(d) + 0.5; 
						*r = d > numeric_limits<C2DUSImage::value_type>::max() ? 
							numeric_limits<C2DUSImage::value_type>::max(): 
							(unsigned short)d; 
					}
				}
			}
		}
	}
	return P2DImage(result); 
}
	
P2DImage C2DDistanceImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(m_filter,image); 
}

C2DDistanceImageFilterFactory::C2DDistanceImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_hw);
}

C2DFilterPlugin::ProductPtr C2DDistanceImageFilterFactory::create(const CParsedOptions& options) const
{
	float scale = param_hw.get_value(options); 
	return C2DFilterPlugin::ProductPtr(new C2DDistanceImageFilter(scale)); 
}

const string C2DDistanceImageFilterFactory::do_get_descr()const
{
	return "2D image distance filter"; 
}
	
	
int C2DDistanceImageFilterFactory::do_test() const
{
	C2DBitImage *input = new C2DBitImage(C2DBounds(3,5)); 
	P2DImage iwrap(input); 
	fill(input->begin(), input->end(), 0); 
	(*input)(1,2) = 1; 
	
	C2DUSImage ref(C2DBounds(3,5)); 
	
	ref(0,0) = ref(2,0) = ref(0,4) = ref(2,4) = (unsigned int)(sqrt(5) * SCALE + 0.5);
	ref(0,1) = ref(2,1) = ref(0,3) = ref(2,3) = (unsigned int)(sqrt(2) * SCALE + 0.5);
	ref(1,1) = ref(1,3) = ref(0,2) = ref(2,2) = (unsigned int)(SCALE);
	ref(1,0) = ref(1,4) = (unsigned int)( 2* SCALE);
	ref(1,2) = 0; 
	
	C2DDistanceImageFilter filter(SCALE); 
	P2DImage rwrap = filter.filter( iwrap );
	
	if (rwrap.get_type() != it_ushort) {
		cvfail() << get_name() << "Expected 'unsigned short' as pixel type but got other\n"; 
		return -1; 
	}
	
	C2DUSImage *result = rwrap.getC2DUSImage(); 
	if (!result) {
		cvfail() << get_name()  << "Can not get result image\n"; 
		return -1; 
	}
	
	if (result->get_size() != ref.get_size()) {
		cvfail()  << get_name() << "filtered image has unexpected size\n"; 
		return -1; 
	}
	if (! equal(result->begin(), result->end(), ref.begin())) {
		cvfail()  << get_name() << "filter result wrong\n"; 
		return -1; 
	}
	return 1; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DDistanceImageFilterFactory(); 
}
} // end namespace distance_2dimage_filter
