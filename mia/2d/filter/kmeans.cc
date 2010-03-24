/* -*- mia-c++  -*-
 *
 * Copyright (c) 2010, Madrid
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

/*! \brief median 2D image filter using a squared support region

A median filter

\file kmeans_2dimage_filter.cc
\author Gert Wollny <gw.fossdev @ gmail.com>

*/

#include <limits>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/kmeans.hh>
#include <mia/2d/filter/kmeans.hh>

NS_BEGIN(kmeans_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DKMeans::C2DKMeans(size_t cls):
	_M_classes(cls)
{
}



template <typename T>
typename C2DKMeans::result_type C2DKMeans::operator () (const T2DImage<T>& data) const
{
	cvdebug() << "C2DKMeans::operator () begin\n";

	C2DUBImage *tresult = new C2DUBImage(data.get_size(), data);
	P2DImage result(tresult);
	std::vector<double> classes(_M_classes); 

	kmeans(data.begin(), data.end(), tresult->begin(), classes); 
	
	// store class centers in attributes; 
	tresult->set_attribute(ATTR_IMAGE_KMEANS_CLASSES, PAttribute(new CVDoubleAttribute(classes))); 

	cvdebug() << "C2DKMeans::operator () end\n";
	return result;
}

P2DImage C2DKMeans::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DKMeansFilterPluginFactory::C2DKMeansFilterPluginFactory():
	C2DFilterPlugin("kmeans")
{
	add_parameter("c", new CIntParameter(_M_classes, 0, numeric_limits<unsigned char>::max(), 
					     false, "number of classes"));
}

C2DKMeansFilterPluginFactory::ProductPtr C2DKMeansFilterPluginFactory::do_create()const
{
	return C2DKMeansFilterPluginFactory::ProductPtr(new C2DKMeans(_M_classes));
}

const string C2DKMeansFilterPluginFactory::do_get_descr()const
{
	return "2D image kmeans filter";
}




extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new  C2DKMeansFilterPluginFactory();
}

NS_END
