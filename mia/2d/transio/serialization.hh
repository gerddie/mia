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


#ifndef mia_transform_serialization_hh
#define mia_transform_serialization_hh

#include <cassert>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_free.hpp>
#include <mia/2d/transformfactory.hh>

namespace boost { namespace serialization {
template<class Archive>
void save(Archive & ar, const mia::C2DTransformation& t, unsigned int )
{
	ar << make_nvp("creator", t.get_creator_string()); 
	ar << make_nvp("size_x", t.get_size().x); 
	ar << make_nvp("size_y", t.get_size().y); 
	auto params = t.get_parameters(); 
	std::vector<double> help(params.size()); 
	std::copy(params.begin(), params.end(), help.begin()); 
	ar << make_nvp("params", help);
}

template<class Archive>
void load(Archive & ar, mia::P2DTransformation & t, unsigned int )
{
	std::string init; 
	mia::C2DBounds size; 
	std::vector<double> help; 
	
	ar >> make_nvp("creator",init); 
	ar >> make_nvp("size_x",size.x); 
	ar >> make_nvp("size_y",size.y); 
	ar >> make_nvp("params", help); 

	auto creator = mia::C2DTransformCreatorHandler::instance().produce(init);  
	t = creator->create(size); 
	auto params = t->get_parameters(); 
	assert(params.size() == help.size()); 
	std::copy(help.begin(), help.end(), params.begin()); 
	t->set_parameters(params); 
}

/*
template<class Archive, class Object>
inline void serialize(Archive & ar, Object & t, const unsigned int file_version)
{
    split_free(ar, t, file_version); 
}
*/

}}

#endif