/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/3d/transformfactory.hh>

#include <vistaio/vista4mia.hh>

#include <vistaio/3dtrans.hh>

NS_BEGIN(vista_3dtrans_io)

NS_MIA_USE
using namespace std; 


C3DVistaTransformationIO::C3DVistaTransformationIO():
	C3DTransformationIO("vista")
{
	add_suffix(".v3dt");
	add_suffix(".v");
}

const std::string C3DVistaTransformationIO::do_get_preferred_suffix() const
{
	return "v3dt"; 
}

P3DTransformation C3DVistaTransformationIO::do_load(const std::string& fname) const
{
	CInputFile f(fname);
	VistaIOAttrList vlist = VistaIOReadFile(f,NULL);
	VistaIOResetProgressIndicator();
	
	if (!vlist)
		return P3DTransformation();
	
	VistaIOLong sx = 0; 
	VistaIOLong sy = 0; 
	VistaIOLong sz = 0; 
	char *init_string = NULL;  
	VistaIOImage blob =NULL; 
	stringstream errmsg; 
	if (VistaIOGetAttr (vlist, "size_x", NULL, VistaIOLongRepn, &sx) != VistaIOAttrFound) {
		errmsg << fname << ":Bogus input, attribute size_x not found"; 
		goto fail; 
	}
	if (VistaIOGetAttr (vlist, "size_y", NULL, VistaIOLongRepn, &sy) != VistaIOAttrFound) {
		errmsg << fname << ":Bogus input, attribute size_y not found"; 
		goto fail; 
	}
	if (VistaIOGetAttr (vlist, "size_z", NULL, VistaIOLongRepn, &sz) != VistaIOAttrFound) {
		errmsg << fname << ":Bogus input, attribute size_z not found"; 
		goto fail; 
	}

	if (VistaIOGetAttr (vlist, "init-string", NULL, VistaIOStringRepn, &init_string) != VistaIOAttrFound) {
		errmsg << fname << ":Bogus input, attribute init-string not found"; 
		goto fail; 
	}
	if (VistaIOGetAttr (vlist, "parameters", NULL, VistaIOImageRepn, &blob) != VistaIOAttrFound) {
		errmsg << fname << ":Bogus input, attribute parameters not found"; 
		goto fail; 
	}
	
	
	// get the data from the image 
	if (VistaIOPixelRepn(blob) != VistaIODoubleRepn) {
		errmsg << fname << ":Bogus input, parameters not if type double"; 
		goto fail; 
	}
	
	{
		auto creator = mia::C3DTransformCreatorHandler::instance().produce(init_string);  
		auto t = creator->create(C3DBounds(sx, sy, sz)); 

		VistaIOAttrList attributes = nullptr; 
		if (VistaIOGetAttr (vlist, "attributes", NULL, VistaIOAttrListRepn, &attributes) == VistaIOAttrFound) {
			copy_attr_list(*t, attributes);
		}
		
		auto params = t->get_parameters(); 
		
		if ((long)params.size() != VistaIOImageNPixels(blob)){
			errmsg << fname << ":Bogus input, expected number of parameters ("
			       << params.size()<<") differs from provided one (" << VistaIOImageNPixels(blob) << ")"; 
			goto fail; 
		}
		{
			VistaIODouble *data = (VistaIODouble *)VistaIOImageData(blob); 
			std::copy(data, data + params.size(), params.begin());
			t->set_parameters(params); 
			VistaIODestroyAttrList(vlist);
			return t; 
		}
	}
 fail: 
	VistaIODestroyAttrList(vlist);
	throw runtime_error(errmsg.str()); 
	
}

bool C3DVistaTransformationIO::do_save(const std::string& fname, const C3DTransformation& data) const
{

	COutputFile f(fname);
	VistaIOAttrList vlist = VistaIOCreateAttrList();
	
	auto params = data.get_parameters(); 
	VistaIOImage out_field = VistaIOCreateImage(params.size(), 1, 1, VistaIODoubleRepn);
	VistaIODouble *output  = (VistaIODouble *)out_field->data;
	copy(params.begin(), params.end(), output);
	
	VistaIOSetAttr (vlist, "size_x", NULL, VistaIOLongRepn, data.get_size().x); 
	VistaIOSetAttr (vlist, "size_y", NULL, VistaIOLongRepn, data.get_size().y); 
	VistaIOSetAttr (vlist, "size_z", NULL, VistaIOLongRepn, data.get_size().z); 
	VistaIOSetAttr (vlist, "init-string", NULL, VistaIOStringRepn, data.get_creator_string().c_str()); 
	VistaIOSetAttr (vlist, "parameters", NULL, VistaIOImageRepn, out_field);
	
	VistaIOAttrList attributes = VistaIOCreateAttrList();
	copy_attr_list(attributes, data);
	VistaIOSetAttr (vlist, "attributes", NULL, VistaIOAttrListRepn, attributes);
	
	bool result = VistaIOWriteFile(f,vlist);
	VistaIODestroyAttrList(vlist);

	return result;
}

const string C3DVistaTransformationIO::do_get_descr() const
{
	return "Vista storage of 3D transformations"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new C3DVistaTransformationIO;
}




NS_END
