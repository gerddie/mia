/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/kmeans.hh>
#include <mia/3d/filter/kmeans.hh>

NS_BEGIN(kmeans_3dimage_filter)
NS_MIA_USE;
using namespace std;

C3DKMeans::C3DKMeans(size_t cls):
	m_classes(cls)
{
}



template <typename T>
typename C3DKMeans::result_type C3DKMeans::operator () (const T3DImage<T>& data) const
{
	cvdebug() << "C3DKMeans::operator () begin\n";

	C3DUBImage *tresult = new C3DUBImage(data.get_size(), data);
	P3DImage result(tresult);
	std::vector<double> classes(m_classes);

	kmeans(data.begin(), data.end(), tresult->begin(), classes);

	// store class centers in attributes;
	tresult->set_attribute(ATTR_IMAGE_KMEANS_CLASSES, PAttribute(new CVDoubleAttribute(classes)));

	cvdebug() << "C3DKMeans::operator () end\n";
	return result;
}

P3DImage C3DKMeans::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DKMeansFilterPluginFactory::C3DKMeansFilterPluginFactory():
	C3DFilterPlugin("kmeans"),
	m_classes(3)
{
	add_parameter("c", new CIntParameter(m_classes, 0, numeric_limits<unsigned char>::max(),
					     false, "number of classes"));
}

C3DFilter *C3DKMeansFilterPluginFactory::do_create()const
{
	return new C3DKMeans(m_classes);
}

const string C3DKMeansFilterPluginFactory::do_get_descr()const
{
	return "3D image k-means filter. In the output image the pixel value represents the class "
		"membership and the class centers are stored as attribute in the image.";
}




extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new  C3DKMeansFilterPluginFactory();
}

NS_END
