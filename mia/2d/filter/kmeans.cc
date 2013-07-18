/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/kmeans.hh>
#include <mia/2d/filter/kmeans.hh>

NS_BEGIN(kmeans_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DKMeans::C2DKMeans(size_t cls):
	m_classes(cls)
{
}



template <typename T>
typename C2DKMeans::result_type C2DKMeans::operator () (const T2DImage<T>& data) const
{
	cvdebug() << "C2DKMeans::operator () begin\n";

	C2DUBImage *tresult = new C2DUBImage(data.get_size(), data);
	P2DImage result(tresult);
	std::vector<double> classes(m_classes);

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
	C2DFilterPlugin("kmeans"),
	m_classes(3)
{
	add_parameter("c", new CIntParameter(m_classes, 2, numeric_limits<unsigned char>::max(),
					     false, "number of classes"));
}

C2DFilter *C2DKMeansFilterPluginFactory::do_create()const
{
	return new C2DKMeans(m_classes);
}

const string C2DKMeansFilterPluginFactory::do_get_descr()const
{
	return "2D image k-means filter. In the output image the pixel value represents the class "
		"membership and the class centers are stored as attribute in the image.";
}




extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new  C2DKMeansFilterPluginFactory();
}

NS_END
