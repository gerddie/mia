/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

/* 
   LatexBeginPlugin{3D image filters}
   
   \subsection{Image cropping}
   \label{filter3d:crop}
   
   \begin{description}
   
   \item [Plugin:] crop
   \item [Description:] Crop a region of an image, the region is always clamped to the original image size. 
   \item [Input:] Abitray grayscale image 
   \item [Output:] A gray scale image that contains the cropped region. 
   
   \plugtabstart
   start & 3D vector & upper left corner of the requested sub-region  & <0,0,0>  \\
   end & 3D vector & lower right corner of the requested sub-region, a value of -1 refers the size of the 
      original image in that dimension  & <-1,-1, -1>  \\
   \plugtabend
   
   \end{description}

   LatexEnd  
 */


#include <mia/3d/filter/crop.hh>

NS_BEGIN(crop_3d_filter);

NS_MIA_USE;
using namespace std;

C3DCrop::C3DCrop(const C3DBounds& begin, const C3DBounds& end):
	m_begin(begin), m_end(end)
{
}

template <typename T>
C3DCrop::result_type C3DCrop::operator () (const T3DImage<T>& data) const
{
	C3DBounds end(m_end);
	if (end.x > data.get_size().x)
		end.x = data.get_size().x;

	if (end.y > data.get_size().y)
		end.y = data.get_size().y;

	if (end.z > data.get_size().z)
		end.z = data.get_size().z;

	C3DBounds begin(m_begin);
	if (begin.x >= data.get_size().x || begin.x >= end.x ||
	    begin.y >= data.get_size().y || begin.y >= end.y ||
	    begin.z >= data.get_size().z || begin.z >= end.z)
		throw invalid_argument("3DCrop: requested range outside image");

	C3DBounds size = end - begin;
	cvdebug() << "crop (" << begin << " - " << end << "\n";

	T3DImage<T> * result = new T3DImage<T>(size, data.get_attribute_list());

	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y) {
			copy(data.begin_at(begin.x, begin.y + y, begin.z + z),
			     data.begin_at(end.x, begin.y + y, begin.z + z), result->begin_at(0,y,z));
		}
	return C3DCrop::result_type(result);
}


mia::P3DImage C3DCrop::do_filter(const mia::C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DCropImageFilterFactory::C3DCropImageFilterFactory():
	C3DFilterPlugin("crop"),
	m_begin(0,0,0),
	m_end(-1,-1,-1)
{
	add_parameter("begin", new TParameter<C3DBounds>(m_begin, false, "begin of cropping range"));
	add_parameter("end", new TParameter<C3DBounds>(m_end, false, "end of cropping range, maximum = (-1,-1,-1)"));
}

C3DFilterPlugin::ProductPtr C3DCropImageFilterFactory::do_create()const
{
	return ProductPtr(new C3DCrop(m_begin, m_end));
}
const std::string C3DCropImageFilterFactory::do_get_descr()const
{
	return "image crop filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DCropImageFilterFactory();
}



NS_END
