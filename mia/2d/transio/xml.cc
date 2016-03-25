/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/2d/transformio.hh>
#include <mia/2d/transio/serialization.hh>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <fstream>
#include <iostream>

NS_MIA_BEGIN
using namespace std; 

namespace bs=boost::serialization; 

class C2DXMLTransformationIO: public C2DTransformationIO {
public: 	
	C2DXMLTransformationIO(); 
private: 
	virtual PData do_load(const std::string& fname) const;
	virtual bool do_save(const std::string& fname, const C2DTransformation& data) const;
	const string do_get_descr() const;
}; 


C2DXMLTransformationIO::C2DXMLTransformationIO():
	C2DTransformationIO("xml")
{
	add_suffix(".x2dt");
}


P2DTransformation C2DXMLTransformationIO::do_load(const std::string& fname) const
{
	std::ifstream ifs(fname);
        boost::archive::xml_iarchive ia(ifs);
        
	P2DTransformation result; 
	
	bs::load(ia, result, 0); 

	return result; 
}

bool C2DXMLTransformationIO::do_save(const std::string& fname, const C2DTransformation& data) const
{
	ofstream ofs(fname);

        boost::archive::xml_oarchive oa(ofs);
	
	bs::save(oa, data, 0); 
	
	return ofs.good();
}

const string C2DXMLTransformationIO::do_get_descr() const
{
	return "XML serialized IO of 2D transformations"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new C2DXMLTransformationIO;
}

NS_MIA_END
