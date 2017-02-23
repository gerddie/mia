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

#ifndef mia_transform_serialization_hh
#define mia_transform_serialization_hh

#include <cassert>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
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
	
	std::map<std::string, std::string> attr; 
	for (auto i = t.begin_attributes(); i != t.end_attributes(); ++i) {
		attr.insert(std::make_pair(i->first, i->second->as_string())); 
	}
	ar << make_nvp("attributes", attr);

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
	std::map<std::string, std::string> attr; 
	ar >> make_nvp("attributes", attr); 
	ar >> make_nvp("params", help); 

	auto creator = mia::C2DTransformCreatorHandler::instance().produce(init);  
	t = creator->create(size); 
	auto params = t->get_parameters(); 
	DEBUG_ASSERT_RELEASE_THROW(params.size() == help.size(), 
				   "bogus transformation file: provides ", 
				   help.size(), " parameters, but transformation needs ", params.size()); 
	std::copy(help.begin(), help.end(), params.begin()); 
	t->set_parameters(params); 

	for(auto i = attr.begin(); i != attr.end(); ++i) {
		t->set_attribute(i->first, mia::CStringAttrTranslatorMap::instance().to_attr(i->first, i->second)); 
	}
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
