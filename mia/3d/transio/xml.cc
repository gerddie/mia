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


/*
  This plug in implements a BOOST binary serialization object for 2D transformations. 
  The output is non-portable (what ever that  exacly means)
*/

#include <mia/3d/transformio.hh>
#include <mia/3d/transio/serialization.hh>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <fstream>
#include <iostream>

NS_MIA_BEGIN

namespace bs=boost::serialization; 

class C3DXMLTransformationIO: public C3DTransformationIO {
public: 	
	C3DXMLTransformationIO(); 
private: 
	virtual void do_add_suffixes(std::multimap<std::string, std::string>& map) const;
	virtual PData do_load(const std::string& fname) const;
	virtual bool do_save(const std::string& fname, const C3DTransformation& data) const;
	const string do_get_descr() const;
}; 


C3DXMLTransformationIO::C3DXMLTransformationIO():
	C3DTransformationIO("xml")
{
}

void C3DXMLTransformationIO::do_add_suffixes(std::multimap<std::string, std::string>& map) const
{
	map.insert(pair<string,string>(".x3dt", get_name()));
}

P3DTransformation C3DXMLTransformationIO::do_load(const std::string& fname) const
{
	std::ifstream ifs(fname);
        boost::archive::xml_iarchive ia(ifs);
        
	P3DTransformation result; 
	
	bs::load(ia, result, 0); 

	return result; 
}

bool C3DXMLTransformationIO::do_save(const std::string& fname, const C3DTransformation& data) const
{
	ofstream ofs(fname);

        boost::archive::xml_oarchive oa(ofs);
	
	bs::save(oa, data, 0); 
	
	return ofs.good();
}

const string C3DXMLTransformationIO::do_get_descr() const
{
	return "XML serialized IO of 3D transformations"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new C3DXMLTransformationIO;
}

NS_MIA_END
