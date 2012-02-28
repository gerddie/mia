/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <mia/core/typedescr.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/filter/sortlabel.hh>

NS_BEGIN(sort_label_2dimage_filter)

NS_MIA_USE; 

template <typename T>
struct CSortLabel_dispatch {
	static P2DImage apply(const T2DImage<T>& /*image*/) {
		THROW(invalid_argument, "Unsupported input label type " << __type_descr<T>::value);  
	}
}; 


template <typename T>
struct LabelSort {
	LabelSort():label(0), n(0){}
	T label; 
	size_t n; 
}; 

template <typename T>
bool LabelSort_predicate (const LabelSort<T>& lhs, const LabelSort<T>& rhs) 
{
	return (lhs.n > rhs.n) || (lhs.n == rhs.n && lhs.label < rhs.label); 
}

template <typename T>
P2DImage CSortLabel_run(const T2DImage<T>& image)
{
	vector<LabelSort<T> > counter(numeric_limits<T>::max()); 
	for (size_t i = 0; i < counter.size(); i++) 
		counter[i].label = i; 
	
	for_each(image.begin(), image.end(), [&counter](T value){ ++counter[value].n;}); 
	sort(counter.begin()+1, counter.end(), LabelSort_predicate<T>);
	
	vector<T> mapper(numeric_limits<T>::max()); 
	for(T i = 1; i <  numeric_limits<T>::max(); ++i) {
		mapper[counter[i].label] = i; 
	}
	T2DImage<T> *result = new T2DImage<T>(image); 
	transform(image.begin(), image.end(), result->begin(), 
		  [&mapper](T value){return mapper[value];}); 
	return P2DImage(result); 
}

template <>
struct CSortLabel_dispatch<unsigned char> {
	static P2DImage apply(const C2DUBImage& image) {
		return CSortLabel_run(image); 
	}
}; 

template <>
struct CSortLabel_dispatch<unsigned short> {
	static P2DImage apply(const C2DUSImage& image) {
		return CSortLabel_run(image); 
	}
}; 


template <typename T> 
P2DImage CSortLabel::operator() (const mia::T2DImage<T>& image) const
{
	return CSortLabel_dispatch<T>::apply(image); 
}



CSortLabel::result_type CSortLabel::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image); 
}

CSortLabelFilterPlugin::CSortLabelFilterPlugin():
	C2DFilterPlugin("sort-label")
{
}


C2DFilter *CSortLabelFilterPlugin::do_create()const
{
	return new CSortLabel(); 
}

const std::string CSortLabelFilterPlugin::do_get_descr()const
{
	return "This plug-in sorts the labels of a gray-scale image so that the lowest label "
		"value corresponts to the lable with themost pixels. The background (0) is not touched"; 
	
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CSortLabelFilterPlugin();
}


NS_END