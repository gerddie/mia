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

/* LatexBeginPluginDescription{2D image filters}
   
   \subsection{K-means classification}
   \label{filter2d:kmeans}
   
   \begin{description}
   
   \item [Plugin:] kmeans
   \item [Description:] Apply a k-means classification to the image 
   \item [Input:] A gray scale image of abitrary pixel type. 
   \item [Output:] The classification image, the class centers are saved as image attribute
   
   \plugtabstart
   c &  int & The number of classes ($\le$ 255) & 5  \\
   \plugtabend

   \end{description}

   LatexEnd  
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
	return "2D image kmeans filter";
}




extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new  C2DKMeansFilterPluginFactory();
}

NS_END
