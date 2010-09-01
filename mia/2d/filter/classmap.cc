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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdexcept>
#include <boost/type_traits.hpp>
#include <mia/2d/2dfilter.hh>
#include <libmona/filter.hh>


#include <libmona/probmapio.hh>

namespace classmap_2dimage_filter {
NS_MIA_USE;
using namespace std; 



static char const *plugin_name = "classmap";
static const CStringOption param_map("map", "class probability map", "");
static const CIntOption param_class("class", "class to be mapped", 2, 0, 254); 

class C2DClassMapFilter: public C2DFilter {
	CDoubleVector _M_pv; 
	
public: 
	C2DClassMapFilter(const CDoubleVector& pv):_M_pv(pv)
	{
	}
	
	template <typename T>
	C2DClassMapFilter::result_type operator()(const T2DImage<T>& image) const; 
	
};


template <typename T, bool is_integral>
struct dispatch_filter {
	static P2DImage apply(const T2DImage<T>& image, const CDoubleVector& pv) 
	{
		throw invalid_argument("so far non-integer pixels are not supported"); 
	}
};

template <typename T>
struct dispatch_filter<T, true> {
	static P2DImage apply(const T2DImage<T>& image, const CDoubleVector& pv) 
	{
		C2DUBImage *result = new C2DUBImage(image.get_size()); 
		
		typename T2DImage<T>::const_iterator i = image.begin(); 
		typename T2DImage<T>::const_iterator e = image.end(); 
		C2DUBImage::iterator r= result->begin(); 
		
		while (i != e) {
			size_t si = *i++; 
			if (si < pv.size())
				*r = (unsigned char)(255 * pv[si]); 
			else {
				*r = 0;
				cvwarn() << "input pixel value "<< *i <<" not covered by probability map\n";
			}
			++r; 
		}
		return P2DImage(result); 
	}
};

template <typename T>
C2DClassMapFilter::result_type C2DClassMapFilter::operator()(const T2DImage<T>& image) const
{
	
	const bool is_integral = ::boost::is_integral<T>::value; 
	return dispatch_filter<T, is_integral>::apply(image, _M_pv);
}


class C2DClassMapFilterImageFilter: public C2DImageFilterBase {
	C2DClassMapFilter _M_filter; 
public:
	C2DClassMapFilterImageFilter(const CDoubleVector& pv);

	virtual P2DImage do_filter(const C2DImage& image) const;
};


class C2DClassMapFilterImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DClassMapFilterImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
//	virtual int do_test() const; 
};


C2DClassMapFilterImageFilter::C2DClassMapFilterImageFilter(const CDoubleVector& pv):
	_M_filter(pv)
{
}

P2DImage C2DClassMapFilterImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(_M_filter, image); 
}


C2DClassMapFilterImageFilterFactory::C2DClassMapFilterImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_map);
	add_help(param_class);
}

C2DFilterPlugin::ProductPtr C2DClassMapFilterImageFilterFactory::create(const CParsedOptions& options) const
{
	string map_name = param_map.get_value(options); 
	size_t sclass = param_class.get_value(options); 
	
	CProbabilityVector pv = load_probability_map(map_name); 
	if (pv.empty())
		throw invalid_argument(string("Unable to load probability map from ") + map_name); 

	if (sclass >= pv.size()) {
		stringstream errmsg; 
		errmsg << "no class nr. " << sclass << " in map '" << map_name << "'"; 
		throw invalid_argument(errmsg.str()); 
	}
	
	return C2DFilterPlugin::ProductPtr(new C2DClassMapFilterImageFilter(pv[sclass])); 		
}
	
const string C2DClassMapFilterImageFilterFactory::do_get_descr()const
{
	return "convert an image into the probability map of a given class";
}

#if 0
bool C2DClassMapFilterImageFilterFactory::do_test() const
{
	cvfail() << do_get_descr() <<": NO TEST IMPLEMENTED\n"; 
	return true; 
}
#endif

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DClassMapFilterImageFilterFactory(); 
}
} // end namespace classmap_2dimage_filter
