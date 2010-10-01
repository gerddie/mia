/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
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

#include <mia/2d/transformio.hh>
#include <mia/2d/transio/serialization.hh>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <fstream>
#include <iostream>

NS_MIA_BEGIN

namespace bs=boost::serialization; 

class C2DBBSTransformationIO: public C2DTransformationIO {
public: 	
	C2DBBSTransformationIO(); 
private: 
	virtual void do_add_suffixes(std::multimap<std::string, std::string>& map) const;
	virtual PData do_load(const std::string& fname) const;
	virtual bool do_save(const std::string& fname, const C2DTransformation& data) const;
	const string do_get_descr() const;
}; 


C2DBBSTransformationIO::C2DBBSTransformationIO():
	C2DTransformationIO("xml")
{
}

void C2DBBSTransformationIO::do_add_suffixes(std::multimap<std::string, std::string>& map) const
{
	map.insert(pair<string,string>(".x2dt", get_name()));
}

P2DTransformation C2DBBSTransformationIO::do_load(const std::string& fname) const
{
	std::ifstream ifs(fname);
        boost::archive::xml_iarchive ia(ifs);
        
	P2DTransformation result; 
	
	bs::load(ia, result, 0); 

	return result; 
}

bool C2DBBSTransformationIO::do_save(const std::string& fname, const C2DTransformation& data) const
{
	ofstream ofs(fname);

        boost::archive::xml_oarchive oa(ofs);
	
	bs::save(oa, data, 0); 
	
	return ofs.good();
}

const string C2DBBSTransformationIO::do_get_descr() const
{
	return "XML serialized IO of 2D transformations"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new C2DBBSTransformationIO;
}

NS_MIA_END
