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


#include <mia/3d/filter/resize.hh>

NS_BEGIN(resize_3d_filter);

NS_MIA_USE;
using namespace std;

C3DResize::C3DResize(const C3DBounds& new_size):
	m_new_size(new_size)
{
}

template <typename T>
C3DResize::result_type C3DResize::operator () (const T3DImage<T>& data) const
{
	C3DBounds size(m_new_size);
	if (!size.x)
		size.x = data.get_size().x;
	
	if (!size.y)
		size.y = data.get_size().y;
	
	if (!size.z)
		size.z = data.get_size().z;

	T3DImage<T> * result = new T3DImage<T>(size, data);

	T3DVector<int> delta = T3DVector<int>(data.get_size() - size); 
	T3DVector<int> start = delta / 2; 
	T3DVector<int> length(data.get_size().x < size.x ? data.get_size().x : size.x, 
			       data.get_size().y < size.y ? data.get_size().y : size.y, 
			       data.get_size().z < size.z ? data.get_size().z : size.z); 
		
	C3DBounds write_start( start.x < 0 ?  -start.x : 0, 
			       start.y < 0 ?  -start.y : 0, 
			       start.z < 0 ?  -start.z : 0); 
	
	C3DBounds read_start(start.x > 0 ?  start.x : 0, 
			     start.y > 0 ?  start.y : 0, 
			     start.z > 0 ?  start.z : 0);

	cvdebug() << "read start = "<< read_start << " length= " << length 
		  << " write start = " << write_start << "\n"; 
		
	for (int z = 0; z < length.z; ++z)
		for (int y = 0; y < length.y; ++y)
			copy(data.begin_at(read_start.x, read_start.y + y, read_start.z + z),
			     data.begin_at(read_start.x + length.x, read_start.y + y, read_start.z + z), 
			     result->begin_at(write_start.x, write_start.y + y, write_start.z + z));
	
	return C3DResize::result_type(result);
}

mia::P3DImage C3DResize::do_filter(const mia::C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DResizeImageFilterPlugin::C3DResizeImageFilterPlugin():
	C3DFilterPlugin("resize"),
	m_new_size(0,0,0)
{
	add_parameter("size", new TParameter<C3DBounds>(m_new_size, false, "new size of the image a size 0 indicates "
							"to keep the size for the corresponding dimension."));
}

C3DFilter *C3DResizeImageFilterPlugin::do_create()const
{
	return new C3DResize(m_new_size);
}
const std::string C3DResizeImageFilterPlugin::do_get_descr()const
{
	return "Resize an image. The original data is centered within the new sized image."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DResizeImageFilterPlugin();
}



NS_END
