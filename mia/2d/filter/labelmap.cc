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

#include <stdexcept>
#include <istream>
#include <fstream>
#include <mia/2d/filter/labelmap.hh>

NS_BEGIN(labelmap_2dimage_filter)
NS_MIA_USE;
using namespace std; 

C2DLabelMapFilter::C2DLabelMapFilter(const CLabelMap& lmap):
	m_map(numeric_limits<unsigned short>::max())
{
	for (size_t i = 0; i < m_map.size(); ++i)
		m_map[i] = i; 
	
	for (CLabelMap::const_iterator i = lmap.begin(), e = lmap.end(); 
	     i != e; ++i)
		m_map[i->first] = i->second; 
}



P2DImage C2DLabelMapFilter::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image); 
}

template <typename T> 
struct dispatch_label_map {
	static P2DImage apply(const T& /*image*/, const vector<size_t>& /*lmap*/){
		throw invalid_argument("Only unsigned short images are supported"); 
	}
};

template <> 
struct dispatch_label_map<C2DUSImage> {
	static P2DImage apply(const C2DUSImage& image, const vector<size_t>& lmap){
		C2DUSImage *result = new C2DUSImage(image.get_size()); 
		transform(image.begin(), image.end(), result->begin(), 
			  [&lmap](unsigned short x){ return lmap[x]; });
		return P2DImage(result); 
	}
};

template <class Data2D>
typename C2DLabelMapFilter::result_type C2DLabelMapFilter::operator () (const Data2D& data) const
{
	return dispatch_label_map<Data2D>::apply(data, m_map); 
	
}

C2DLabelMapFilterPlugin::C2DLabelMapFilterPlugin():
	C2DFilterPlugin("labelmap")
{
	add_parameter("map", new CStringParameter(m_map, true, "Label mapping file")) ;
}


C2DFilter *C2DLabelMapFilterPlugin::do_create() const
{
	ifstream is(m_map.c_str()); 

	if (!is.good())
		THROW(runtime_error, "C2DLabelMapFilterPlugin::load_label_map:"
		      "Unable to open file '" << m_map << "'"); 
	
	CLabelMap map(is);
	return new C2DLabelMapFilter(map); 
}

const string C2DLabelMapFilterPlugin::do_get_descr()const
{
	return "2D image filter to remap label id's."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DLabelMapFilterPlugin(); 
}

NS_END
