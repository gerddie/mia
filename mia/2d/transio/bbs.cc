/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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


/*
  This plug in implements a BOOST binary serialization object for 2D transformations. 
  The output is non-portable (what ever that  exacly means)
*/

#include <mia/2d/transformio.hh>
#include <mia/2d/transio/serialization.hh>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>
#include <iostream>

NS_MIA_BEGIN

namespace bs=boost::serialization; 

using namespace std; 

class C2DBBSTransformationIO: public C2DTransformationIO {
public: 	
	C2DBBSTransformationIO(); 
private: 
	virtual PData do_load(const std::string& fname) const;
	virtual bool do_save(const std::string& fname, const C2DTransformation& data) const;
	const string do_get_descr() const;
}; 


C2DBBSTransformationIO::C2DBBSTransformationIO():
	C2DTransformationIO("bbs")
{
	add_suffix(".bbs");
}

P2DTransformation C2DBBSTransformationIO::do_load(const std::string& fname) const
{
	std::ifstream ifs(fname);
        boost::archive::binary_iarchive ia(ifs);
        
	P2DTransformation result; 
	
	bs::load(ia, result, 0); 

	return result; 
}

bool C2DBBSTransformationIO::do_save(const std::string& fname, const C2DTransformation& data) const
{
	ofstream ofs(fname);

        boost::archive::binary_oarchive oa(ofs);
	
	bs::save(oa, data, 0); 
	
	return ofs.good();
}

const string C2DBBSTransformationIO::do_get_descr() const
{
	return "Binary (non-portable) serialized IO of 2D transformations"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new C2DBBSTransformationIO;
}

NS_MIA_END
