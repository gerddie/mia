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

#include <stdexcept>


#include <mia/2d/2dfilter.hh>

#include <libmona/probmapio.hh>

namespace labelmap_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const *plugin_name = "labelmap";
static const CStringOption param_map("map", "map file", NULL);

class C2DLabelMap: public C2DFilter {
	vector<size_t>  _M_map;
public:
	C2DLabelMap(const CLabelMap& lmap):
		_M_map(numeric_limits<unsigned short>::max())
	{
		for (size_t i = 0; i < _M_map.size(); ++i)
			_M_map[i] = i; 
		
		for (CLabelMap::const_iterator i = lmap.begin(), e = lmap.end(); 
		     i != e; ++i)
			_M_map[i->first] = i->second; 
	}
	
	template <class Data2D>
	typename C2DLabelMap::result_type operator () (const Data2D& data) const ;

};


class C2DLabelMapImageFilter: public C2DImageFilterBase {
	C2DLabelMap _M_filter; 
public:
	C2DLabelMapImageFilter(const CLabelMap& map);

	virtual P2DImage do_filter(const C2DImage& image) const;
};

class C2DLabelMapImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DLabelMapImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
	virtual int do_test() const; 
};

template <typename T> 
struct dispatch_label_map {
	static P2DImage apply(const T& image, const vector<size_t>& lmap){
		throw invalid_argument("Only unsigned short images are supported"); 
	}
};

struct FRemap {
	FRemap(const vector<size_t>& lmap):
		_M_map(lmap)
	{
	}
	
	unsigned short operator ()(unsigned short x) {
		return _M_map[x]; 
	}
private: 
	const vector<size_t>& _M_map; 
};

template <> 
struct dispatch_label_map<C2DUSImage> {
	static P2DImage apply(const C2DUSImage& image, const vector<size_t>& lmap){
		C2DUSImage *result = new C2DUSImage(image.get_size()); 
		transform(image.begin(), image.end(), result->begin(), FRemap(lmap)); 
		return P2DImage(result); 
	}
};

/*	
template <> 
struct dispatch_label_map<C2DUBImage> {
	static P2DImage apply(const C2DUBImage& image, const vector<size_t>& lmap){
		C2DUBImage *result = new C2DUBImage(image.get_size()); 
		transform(image.begin(), image.end(), result->begin(), FRemap(lmap)); 
		return P2DImage(result); 
	}
};
*/	
	
template <class Data2D>
typename C2DLabelMap::result_type C2DLabelMap::operator () (const Data2D& data) const
{
	return dispatch_label_map<Data2D>::apply(data, _M_map); 
	
}

C2DLabelMapImageFilter::C2DLabelMapImageFilter(const CLabelMap& map):
	_M_filter(map)
{
}

P2DImage C2DLabelMapImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(_M_filter,image); 
}

C2DLabelMapImageFilterFactory::C2DLabelMapImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_map); 
}

C2DFilterPlugin::ProductPtr C2DLabelMapImageFilterFactory::create(const CParsedOptions& options) const
{
	string map_file = param_map.get_value(options); 
	CLabelMap map = load_label_map(map_file);

	return C2DFilterPlugin::ProductPtr(new C2DLabelMapImageFilter(map)); 
}

const string C2DLabelMapImageFilterFactory::do_get_descr()const
{
	return "2D image label mapper filter"; 
}

int C2DLabelMapImageFilterFactory::do_test() const
{
	C2DUSImage *src = new C2DUSImage(C2DBounds(8,32));
	C2DUSImage ref(C2DBounds(8,32));
	
	C2DUSImage::iterator is = src->begin();
	C2DUSImage::iterator ir = ref.begin();
	
	for (size_t i = 0; i < src->size(); ++i, ++is, ++ir)
		*is = *ir = i;
	
	CLabelMap map;
	map[1] = 2; 
	map[2] = 4;
	map[23] = 7;
	map[189] = 10;
	
	ref(1,0) = 2; 
	ref(2,0) = 4;
	ref(7,2) = 7; 
	ref(5,23) = 10; 
	
	P2DImage wsrc(src); 
	
	C2DLabelMapImageFilter filter(map); 
	
	P2DImage wres = filter.filter(wsrc); 
	
	if (wres.get_type() != it_ushort) {
		cvfail() << get_name() << "Expected 'unsigned short' as pixel type but got other\n"; 
		return -1; 
	}
		
	C2DUSImage *result = wres.getC2DUSImage(); 
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
	return new C2DLabelMapImageFilterFactory(); 
}
} // end namespace labelmap_2dimage_filter
