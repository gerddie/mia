/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <sstream>
#include <cassert>
#include <algorithm>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/vfio.hh>
#include <mia/3d/image.hh>

#include <vistaio/vista4mia.hh>

NS_BEGIN(vista_3dvf_io)

NS_MIA_USE

using namespace std;
using namespace boost;

class CVista3DVFIOPlugin : public C3DVFIOPlugin {
public:
	CVista3DVFIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const std::string do_get_preferred_suffix() const; 
	const string do_get_descr() const;
};

CVista3DVFIOPlugin::CVista3DVFIOPlugin():
	C3DVFIOPlugin("vista")
{
	add_supported_type(it_float);
	add_suffix(".v");
	add_suffix(".vf");
	add_suffix(".V");
	add_suffix(".VF");
	
	add_standard_vistaio_properties(*this); 
}

const std::string CVista3DVFIOPlugin::do_get_preferred_suffix() const
{
	return "vf"; 
}


CVista3DVFIOPlugin::PData  CVista3DVFIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);

	CVAttrList vlist(VistaIOReadFile(f,NULL));

	VistaIOResetProgressIndicator();

	if (!vlist)
		return CVista3DVFIOPlugin::PData();

	CVista3DVFIOPlugin::PData result;
	VistaIOAttrListPosn posn;

	for (VistaIOFirstAttr(vlist, &posn); VistaIOAttrExists(&posn) && !result; VistaIONextAttr(&posn)) {
		if (VistaIOGetAttrRepn(&posn) != VistaIOField3DRepn)
			continue;

		VistaIOField3D field = NULL;
		VistaIOGetAttrValue(&posn, 0, VistaIOField3DRepn, &field);

		if (!field)
			throw runtime_error(fname + "looked like a vector field, but is none");

		if (field->nsize_element != 3)
			throw runtime_error(fname + "is not a field of 3D vectors");

		if (field->repn != VistaIOFloatRepn) {
			cvdebug() << "Skipping input field, which is not of type float\n";
			continue;
		}


		result = CVista3DVFIOPlugin::PData(new C3DIOVectorfield(C3DBounds(field->x_dim, field->y_dim, field->z_dim)));
		
		T3DVector<VistaIOFloat> * input  = (T3DVector<VistaIOFloat> *)field->p.data;
		copy(input, input + result->size(), result->begin());
		copy_attr_list(*result, field->attr);
	}

	return result;
}


bool CVista3DVFIOPlugin::do_save(const string& fname, const C3DIOVectorfield& data) const
{

	COutputFile f(fname);

	CVAttrList vlist(VistaIOCreateAttrList()); 
	VistaIOField3D out_field = VistaIOCreateField3D(data.get_size().x,
					    data.get_size().y,
					    data.get_size().z,
					    3,
					    VistaIOFloatRepn);
	T3DVector<VistaIOFloat> * output  = (T3DVector<VistaIOFloat> *)out_field->p.data;
	copy(data.begin(), data.end(), output);
	copy_attr_list(out_field->attr, data);
	VistaIOSetAttr(vlist, "3DFVectorfield", NULL, VistaIOField3DRepn, out_field);

	bool result = VistaIOWriteFile(f,vlist);
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
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const std::string do_get_preferred_suffix() const; 
	const string do_get_descr() const;
	template <typename T>
	CScaled3DVFIOPlugin::PData read_compressed(const T3DVector<VistaIOLong>& _size, const C3DFVector& scale, 
						   T3DVector<VistaIOImage>& values)const; 

};

CScaled3DVFIOPlugin::CScaled3DVFIOPlugin():
	C3DVFIOPlugin("cvista")
{
	add_supported_type(it_float);
	add_suffix(".cvf");
	add_suffix(".CVF");

}

template <typename T>
CScaled3DVFIOPlugin::PData CScaled3DVFIOPlugin::read_compressed(const T3DVector<VistaIOLong>& _size, const C3DFVector& scale, 
								T3DVector<VistaIOImage>& values)const
{
	CScaled3DVFIOPlugin::PData result(new C3DIOVectorfield(C3DBounds(_size)));

	T *x = &VistaIOPixel( values.x, 0, 0, 0, T );
	T *y = &VistaIOPixel( values.x, 0, 0, 0, T );
	T *z = &VistaIOPixel( values.x, 0, 0, 0, T );

	for (auto r = result->begin(); r != result->end(); ++r, ++y, ++y, ++z) {
		r->x = scale.x * *x; 
		r->y = scale.y * *y; 
		r->z = scale.z * *z; 
	}
	return result; 
}

const std::string CScaled3DVFIOPlugin::do_get_preferred_suffix() const
{
	return "cvf"; 
}


CScaled3DVFIOPlugin::PData CScaled3DVFIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);

	CVAttrList  vlist(VistaIOReadFile(f,NULL));
	
	VistaIOResetProgressIndicator();
	
	if (!vlist)
		return CScaled3DVFIOPlugin::PData();
	
	CScaled3DVFIOPlugin::PData result;
	VistaIOAttrListPosn posn;

	if((VistaIOLookupAttr( vlist, "3DScaledVectorfield", &posn)) ){
		if (VistaIOGetAttrRepn(&posn) != VistaIOListRepn) {
			throw invalid_argument("CScaled3DVFIOPlugin::do_load; got a 3DScaledVectorfield tag, but it is not an attribute list"); 
		}
		VistaIOAttrList data; 
		VistaIOGetAttrValue(&posn, 0, VistaIOListRepn, &data);
		
		VistaIOLong pixel_repn; 
		C3DFVector scale(1.0,1.0,1.0);
		T3DVector<VistaIOLong> size; 
		T3DVector<VistaIOImage> values; 
		if (!VistaIOExtractAttr (data, VistaIORepnAttr, VistaIONumericRepnDict, VistaIOLongRepn, &pixel_repn, TRUE) ||
		    !VistaIOExtractAttr (data, "x_scale", 0, VistaIOFloatRepn, &scale.x, TRUE) ||
		    !VistaIOExtractAttr (data, "y_scale", 0, VistaIOFloatRepn, &scale.y, TRUE) ||
		    !VistaIOExtractAttr (data, "z_scale", 0, VistaIOFloatRepn, &scale.z, TRUE) ||
		    !VistaIOExtractAttr (data, "x_component", 0, VistaIOImageRepn, &values.x, TRUE) ||
		    !VistaIOExtractAttr (data, "y_component", 0, VistaIOImageRepn, &values.y, TRUE) ||
		    !VistaIOExtractAttr (data, "z_component", 0, VistaIOImageRepn, &values.z, TRUE)||
		    !VistaIOExtractAttr (data, "size_x", 0, VistaIOLongRepn, &size.x, TRUE) ||
		    !VistaIOExtractAttr (data, "size_y", 0, VistaIOLongRepn, &size.y, TRUE) ||
		    !VistaIOExtractAttr (data, "size_z", 0, VistaIOLongRepn, &size.z, TRUE)) 
			throw invalid_argument("CScaled3DVFIOPlugin::do_load: bogus file"); 
		
		switch (pixel_repn) {
		case VistaIOShortRepn: result = read_compressed<short>(size, scale, values);
			break; 
		case VistaIOSByteRepn: result = read_compressed<unsigned char>(size, scale, values);
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
	return "obsolete 3d vector field io plugin for range compressend vista (saving not supported)";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	CVoxelAttributeTranslator::register_for("voxel");
	CPluginBase *p = new CVista3DVFIOPlugin();
	p->append_interface(new CScaled3DVFIOPlugin());
	return p; 
}

NS_END
