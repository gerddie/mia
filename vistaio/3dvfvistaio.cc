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

	CVAttrList vlist(VReadFile(f,NULL));

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
		copy_attr_list(*result, field->attr);
	}

	return result;
}


bool CVista3DVFIOPlugin::do_save(const string& fname, const C3DIOVectorfield& data) const
{

	COutputFile f(fname);

	CVAttrList vlist(VCreateAttrList()); 
	VField3D out_field = VCreateField3D(data.get_size().x,
					    data.get_size().y,
					    data.get_size().z,
					    3,
					    VFloatRepn);
	T3DVector<VFloat> * output  = (T3DVector<VFloat> *)out_field->p.data;
	copy(data.begin(), data.end(), output);
	copy_attr_list(out_field->attr, data);
	VSetAttr(vlist, "3DFVectorfield", NULL, VField3DRepn, out_field);

	bool result = VWriteFile(f,vlist);
	return result;
}

const string CVista3DVFIOPlugin::do_get_descr() const
{
	return "a 3d vector field io plugin for vista";
}


class CScaled3DVFIOPlugin : public C3DVFIOPlugin {
public:
	CScaled3DVFIOPlugin();
private:
	void do_add_suffixes(multimap<string, string>& map) const;
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
	template <typename T>
	CScaled3DVFIOPlugin::PData read_compressed(const T3DVector<VLong>& _size, const C3DFVector& scale, 
						   T3DVector<VImage>& values)const; 

};

CScaled3DVFIOPlugin::CScaled3DVFIOPlugin():
	C3DVFIOPlugin("cvista")
{
	add_supported_type(it_float);
}

void CScaled3DVFIOPlugin::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".svf", get_name()));
	map.insert(pair<string,string>(".SVF", get_name()));
}

template <typename T>
CScaled3DVFIOPlugin::PData CScaled3DVFIOPlugin::read_compressed(const T3DVector<VLong>& _size, const C3DFVector& scale, 
								T3DVector<VImage>& values)const
{
	CScaled3DVFIOPlugin::PData result(new C3DIOVectorfield(C3DBounds(_size)));

	T *x = &VPixel( values.x, 0, 0, 0, T );
	T *y = &VPixel( values.x, 0, 0, 0, T );
	T *z = &VPixel( values.x, 0, 0, 0, T );

	for (auto r = result->begin(); r != result->end(); ++r, ++y, ++y, ++z) {
		r->x = scale.x * *x; 
		r->y = scale.y * *y; 
		r->z = scale.z * *z; 
	}
	return result; 
}

CScaled3DVFIOPlugin::PData CScaled3DVFIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);

	CVAttrList  vlist(VReadFile(f,NULL));
	
	VResetProgressIndicator();
	
	if (!vlist)
		return CScaled3DVFIOPlugin::PData();
	
	CScaled3DVFIOPlugin::PData result;
	VAttrListPosn posn;

	if((VLookupAttr( vlist, "3DScaledVectorfield", &posn)) ){
		if (VGetAttrRepn(&posn) != VListRepn) {
			throw invalid_argument("CScaled3DVFIOPlugin::do_load; got a 3DScaledVectorfield tag, but it is not an attribute list"); 
		}
		VAttrList data; 
		VGetAttrValue(&posn, 0, VListRepn, &data);
		
		VLong pixel_repn; 
		C3DFVector scale(1.0,1.0,1.0);
		T3DVector<VLong> size; 
		T3DVector<VImage> values; 
		if (!VExtractAttr (data, VRepnAttr, VNumericRepnDict, VLongRepn, &pixel_repn, TRUE) ||
		    !VExtractAttr (data, "x_scale", 0, VFloatRepn, &scale.x, TRUE) ||
		    !VExtractAttr (data, "y_scale", 0, VFloatRepn, &scale.y, TRUE) ||
		    !VExtractAttr (data, "z_scale", 0, VFloatRepn, &scale.z, TRUE) ||
		    !VExtractAttr (data, "x_component", 0, VImageRepn, &values.x, TRUE) ||
		    !VExtractAttr (data, "y_component", 0, VImageRepn, &values.y, TRUE) ||
		    !VExtractAttr (data, "z_component", 0, VImageRepn, &values.z, TRUE)||
		    !VExtractAttr (data, "size_x", 0, VLongRepn, &size.x, TRUE) ||
		    !VExtractAttr (data, "size_y", 0, VLongRepn, &size.y, TRUE) ||
		    !VExtractAttr (data, "size_z", 0, VLongRepn, &size.z, TRUE)) 
			throw invalid_argument("CScaled3DVFIOPlugin::do_load: bogus file"); 
		
		switch (pixel_repn) {
		case VShortRepn: result = read_compressed<short>(size, scale, values);
			break; 
		case VSByteRepn: result = read_compressed<unsigned char>(size, scale, values);
			break; 
		default: 
			throw invalid_argument("CScaled3DVFIOPlugin::do_load: unsupported representation"); 
		}
		copy_attr_list(*result, data);
	}
	return result; 
}

bool CScaled3DVFIOPlugin::do_save(const string& /*fname*/, const Data& /*data*/) const
{
	throw invalid_argument("cviste: obsolete file format, saving nor supported"); 
}

const string CScaled3DVFIOPlugin::do_get_descr() const
{
	return "a 3d vector field io plugin for compressend vista";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	CVoxelAttributeTranslator::register_for("voxel");
	CPluginBase *p = new CVista3DVFIOPlugin();
	p->append_interface(new CScaled3DVFIOPlugin());
	return p; 
}

NS_END
