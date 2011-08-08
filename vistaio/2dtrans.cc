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
#include <mia/2d/transformio.hh>
#include <mia/2d/transformfactory.hh>

#include <vistaio/vista4mia.hh>

NS_BEGIN(vista_2dtrans_io)

NS_MIA_USE


class C2DVistaTransformationIO: public C2DTransformationIO {
public: 	
	C2DVistaTransformationIO(); 
private: 
	virtual void do_add_suffixes(std::multimap<std::string, std::string>& map) const;
	virtual PData do_load(const std::string& fname) const;
	virtual bool do_save(const std::string& fname, const C2DTransformation& data) const;
	const string do_get_descr() const;
}; 


C2DVistaTransformationIO::C2DVistaTransformationIO():
	C2DTransformationIO("vista")
{
}

void C2DVistaTransformationIO::do_add_suffixes(std::multimap<std::string, std::string>& map) const
{
	map.insert(pair<string,string>(".v2dt", get_name()));
}

P2DTransformation C2DVistaTransformationIO::do_load(const std::string& fname) const
{
	CInputFile f(fname);
	VAttrList vlist = VReadFile(f,NULL);
	VResetProgressIndicator();
	
	if (!vlist)
		return P2DTransformation();
	
	VLong sx = 0; 
	VLong sy = 0; 
	char *init_string = NULL;  
	VImage blob =NULL; 
	stringstream errmsg; 
	if (VGetAttr (vlist, "size_x", NULL, VLongRepn, &sx) != VAttrFound) {
		errmsg << fname << ":Bogus input, attribute size_x not found"; 
		goto fail; 
	}
	if (VGetAttr (vlist, "size_y", NULL, VLongRepn, &sy) != VAttrFound) {
		errmsg << fname << ":Bogus input, attribute size_y not found"; 
		goto fail; 
	}
	if (VGetAttr (vlist, "init-string", NULL, VStringRepn, &init_string) != VAttrFound) {
		errmsg << fname << ":Bogus input, attribute init-string not found"; 
		goto fail; 
	}
	if (VGetAttr (vlist, "parameters", NULL, VImageRepn, &blob) != VAttrFound) {
		errmsg << fname << ":Bogus input, attribute parameters not found"; 
		goto fail; 
	}
	
	// get the data from the image 
	if (VPixelRepn(blob) != VDoubleRepn) {
		errmsg << fname << ":Bogus input, parameters not if type double"; 
		goto fail; 
	}
	
	{
		auto creator = mia::C2DTransformCreatorHandler::instance().produce(init_string);  
		auto t = creator->create(C2DBounds(sx, sy)); 
		auto params = t->get_parameters(); 
		
		if ((long)params.size() != VImageNPixels(blob)){
			errmsg << fname << ":Bogus input, expected number of parameters differs from provided one"; 
			goto fail; 
		}
		{
			VDouble *data = (VDouble *)VImageData(blob); 
			std::copy(data, data + params.size(), params.begin());
			t->set_parameters(params); 
			VDestroyAttrList(vlist);
			return t; 
		}
	}
 fail: 
	VDestroyAttrList(vlist);
	throw runtime_error(errmsg.str()); 
	
}

bool C2DVistaTransformationIO::do_save(const std::string& fname, const C2DTransformation& data) const
{

	COutputFile f(fname);
	VAttrList vlist = VCreateAttrList();
	
	auto params = data.get_parameters(); 
	VImage out_field = VCreateImage(params.size(), 1, 1, VDoubleRepn);
	VDouble *output  = (VDouble *)out_field->data;
	copy(params.begin(), params.end(), output);
	
	VSetAttr (vlist, "size_x", NULL, VLongRepn, data.get_size().x); 
	VSetAttr (vlist, "size_y", NULL, VLongRepn, data.get_size().y); 
	VSetAttr (vlist, "init-string", NULL, VStringRepn, data.get_creator_string().c_str()); 
	VSetAttr (vlist, "parameters", NULL, VImageRepn, out_field);

	bool result = VWriteFile(f,vlist);
	VDestroyAttrList(vlist);

	return result;
}

const string C2DVistaTransformationIO::do_get_descr() const
{
	return "Vista storage of 2D transformations"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new C2DVistaTransformationIO;
}




NS_END
