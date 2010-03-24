/* -*- mia-c++  -*-
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
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

#include <sstream>
#include <cassert>
#include <algorithm>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/3dvfio.hh>
#include <mia/3d/3DImage.hh>

#include <vistaio/vista4mia.hh>

NS_BEGIN(vista_3dvf_io)

NS_MIA_USE

using namespace std; 
using namespace boost; 

class CVista3DVFIOPlugin : public C3DVFIOPlugin {
public: 
	CVista3DVFIOPlugin();
private: 
	void do_add_suffixes(multimap<string, string>& map) const;
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const; 
}; 

CVista3DVFIOPlugin::CVista3DVFIOPlugin():
	C3DVFIOPlugin("vista")
{
	add_supported_type(it_float);
}
         
void CVista3DVFIOPlugin::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".v", get_name())); 
	map.insert(pair<string,string>(".vf", get_name())); 
	map.insert(pair<string,string>(".V", get_name())); 
	map.insert(pair<string,string>(".VF", get_name())); 
}

CVista3DVFIOPlugin::PData  CVista3DVFIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);
	
	VAttrList vlist = VReadFile(f,NULL);
		
	VResetProgressIndicator();
		
	if (!vlist)
		return CVista3DVFIOPlugin::PData(); 
	
	CVista3DVFIOPlugin::PData result; 
	VAttrListPosn posn; 

	for (VFirstAttr(vlist, &posn); VAttrExists(&posn) && !result; VNextAttr(&posn)) {
		if (VGetAttrRepn(&posn) != VField3DRepn) 
			continue;
		
		VField3D field = NULL; 
		VGetAttrValue(&posn, 0, VField3DRepn, &field); 
		
		if (!field)
			throw runtime_error(fname + "looked like a vector field, but is none");
		
		if (field->nsize_element != 3) 
			throw runtime_error(fname + "is not a field of 3D vectors");
		
		if (field->repn != VFloatRepn) {
			cvdebug() << "Skipping input field, which is not of type float\n";
			continue; 
		}
		
		result = CVista3DVFIOPlugin::PData(new C3DIOVectorfield(C3DBounds(field->x_dim, field->y_dim, field->z_dim))); 
		T3DVector<VFloat> * input  = (T3DVector<VFloat> *)field->p.data; 
		copy(input, input + result->size(), result->begin()); 
		copy_attr_list(*result->get_attribute_list(), field->attr); 
	}
	
	VDestroyAttrList(vlist); 
	return result; 
}


bool CVista3DVFIOPlugin::do_save(const string& fname, const C3DIOVectorfield& data) const
{
	
	COutputFile f(fname);

	VAttrList vlist = VCreateAttrList();
	VField3D out_field = VCreateField3D(data.get_size().x, 
					    data.get_size().y, 
					    data.get_size().z, 
					    3, 
					    VFloatRepn); 
	T3DVector<VFloat> * output  = (T3DVector<VFloat> *)out_field->p.data; 
	copy(data.begin(), data.end(), output); 
	copy_attr_list(out_field->attr, *data.get_attribute_list()); 
	VSetAttr(vlist, "3DFVectorfield", NULL, VField3DRepn, out_field); 
	
	bool result = VWriteFile(f,vlist);
	VDestroyAttrList(vlist); 
	
	return result; 
}

const string CVista3DVFIOPlugin::do_get_descr() const
{
	return "a 3d vector field io plugin for vista";  
}

extern "C" EXPORT  CPluginBase *get_plugin_interface() 
{
	CVoxelAttributeTranslator::register_for("voxel");
	return new CVista3DVFIOPlugin();
}

NS_END
