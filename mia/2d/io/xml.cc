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



#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>
#include <mia/core/tools.hh>
#include <mia/2d/io/xml.hh>

NS_BEGIN(myosegset2d)

using namespace mia; 
using std::string; 
using std::ofstream; 
using std::unique_ptr; 
namespace bfs=boost::filesystem;

CXMLSegSetWithImagesIOPlugin::CXMLSegSetWithImagesIOPlugin():
	CSegSetWithImagesIOPlugin("xml") 
{
	add_suffix(".set");
	add_suffix(".SET");
}


PSegSetWithImages CXMLSegSetWithImagesIOPlugin::do_load(const string& fname) const
{
	xmlpp::DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(fname);

	if (!parser){
		cvdebug() << "CXMLSegSetWithImagesIOPlugin:'" << fname << "' is not a XML document\n"; 
		return PSegSetWithImages(); 
	}
	
	const auto *document = parser.get_document(); 
	const auto *root = document->get_root_node ();
	if (root->get_name() != "workset") {
		cvdebug() << "CXMLSegSetWithImagesIOPlugin:'" << fname 
			  << "' XML file is not a segmentation set\n"; 
		return PSegSetWithImages();
	}

	bfs::path src_path_(fname);
	src_path_.remove_filename();
	auto src_path = src_path_.string();

	return  PSegSetWithImages(new CSegSetWithImages(*document, src_path)); 
}

bool CXMLSegSetWithImagesIOPlugin::do_save(const string& fname, const CSegSetWithImages& data) const
{
	unique_ptr<xmlpp::Document> outset(data.write());
	ofstream outfile(fname.c_str() );
	if (outfile.good()) {
		outfile << outset->write_to_string_formatted();
		data.save_images(fname); 
	}
	return outfile.good();
}

const string CXMLSegSetWithImagesIOPlugin::do_get_descr() const
{
	return "Load and store segmentation sets as xml file and separate image files."; 
}


const string CXMLSegSetWithImagesIOPlugin::do_get_preferred_suffix() const
{
	return "set"; 
}




NS_END
