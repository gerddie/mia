/* -*- mia-c++  -*-
 * Copyright (c) 2007 Gert Wollny <gert at die.upm.es>
 * Biomedical Image Technologies, Universidad Politecnica de Madrid
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

/*! 
A 3D conected component labeling filter 
\file label.cc

*/

#include <stdexcept>
#include <queue>
#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/label.hh>

NS_MIA_USE; 
using namespace std; 
using namespace boost; 

NS_BEGIN(label_3dimage_filter)

CLabel::CLabel(P3DShape& mask):
	_M_mask(mask)
{
}

void CLabel::grow_region(const C3DBounds& loc, const C3DBitImage& input, C3DUSImage& result, unsigned short label)const
{
	queue<C3DBounds> neighbors; 
	neighbors.push(loc); 
	result(loc) = label; 
	C3DBounds size = result.get_size(); 

	while (!neighbors.empty()) {
		C3DBounds  l = neighbors.front(); 

		neighbors.pop(); 
		for (C3DShape::const_iterator s = _M_mask->begin(); s != _M_mask->end(); ++s) {
			C3DBounds  pos(l.x + s->x, l.y + s->y, l.z + s->z);
			if (pos.x < size.x && pos.y < size.y && pos.z < size.z && input(pos) && result(pos) == 0) {
				result(pos) = label; 
				neighbors.push(pos);
			}
		}
	}
}

CLabel::result_type CLabel::do_filter(const C3DImage& image) const
{
	if (image.get_pixel_type() != it_bit){
		throw invalid_argument("Label: Only bit input images are allowed"); 
	}
	
	const C3DBitImage& input = dynamic_cast<const C3DBitImage&>(image); 
	
	unsigned short current_label = 1;
	C3DUSImage *result = new C3DUSImage(image.get_size(), image.get_attribute_list()); 
	P3DImage presult(result); 
	fill(result->begin(), result->end(), 0); 

	C3DBitImage::const_iterator  i = input.begin(); 
	C3DUSImage::const_iterator   r = result->begin(); 

	C3DBounds size(image.get_size()); 
	C3DBounds loc; 

	for (loc.z = 0; loc.z < size.z; ++loc.z)
		for (loc.y = 0; loc.y < size.y; ++loc.y)
			for (loc.x = 0; loc.x < size.x; ++loc.x, ++i, ++r) {
				if (*i && !*r) {
					cvdebug() << "label:" << current_label << " seed at " << loc << "\n"; 
					grow_region(loc, input, *result, current_label); 
					++current_label; 

				}
			}

	cvmsg() << "\n"; 
	if (current_label < 257) {
		C3DUBImage *real_result = new C3DUBImage(image.get_size(), image.get_attribute_list()); 
		copy(result->begin(), result->end(), real_result->begin()); 
		presult.reset(real_result); 
	}
	return presult; 
}

C3DLabelFilterPlugin::C3DLabelFilterPlugin():
	C3DFilterPlugin("label"),
	_M_mask_descr("6n")
{
	add_parameter("n", new CStringParameter(_M_mask_descr, false, "neighborhood mask")) ; 
}

C3DFilterPlugin::ProductPtr C3DLabelFilterPlugin::do_create()const
{
	P3DShape mask = C3DShapePluginHandler::instance().produce(_M_mask_descr.c_str()); 
	if (!mask) 
		return C3DFilterPlugin::ProductPtr(); 
	return C3DFilterPlugin::ProductPtr(new CLabel(mask));
}
			
const string C3DLabelFilterPlugin::do_get_descr()const
{
	return "a 3D conected component labeler"; 
}

bool  C3DLabelFilterPlugin::do_test() const
{
	return true; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DLabelFilterPlugin(); 
}


NS_END

