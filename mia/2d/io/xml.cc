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



#include <boost/filesystem.hpp>
#include <mia/core/tools.hh>
#include <mia/core/xmlinterface.hh>
#include <mia/2d/io/xml.hh>

#include <iterator>


NS_BEGIN(myosegset2d)

using namespace mia; 
using std::string; 
using std::ofstream;
using std::ifstream; 
using std::unique_ptr;
using std::runtime_error;
namespace bfs=boost::filesystem;

CXMLSegSetWithImagesIOPlugin::CXMLSegSetWithImagesIOPlugin():
	CSegSetWithImagesIOPlugin("xml") 
{
	add_suffix(".set");
	add_suffix(".SET");
}


PSegSetWithImages CXMLSegSetWithImagesIOPlugin::do_load(const string& fname) const
{
	ifstream infile(fname);
	string xml_init(std::istreambuf_iterator<char>{infile}, {});
	
	if (!infile.good())
		throw create_exception<runtime_error>("CXMLSegSetWithImagesIOPlugin: Unable to read input file: '", fname, "'");
	
	CXMLDocument doc;
	if (!doc.read_from_string(xml_init.c_str())) {
		cvdebug() << "CXMLSegSetWithImagesIOPlugin:'" << fname 
			  << "' not an XML file\n";
		return PSegSetWithImages();
	}

	auto root = doc.get_root_node ();
	
	if (root->get_name() != "workset") {
		cvdebug() << "CXMLSegSetWithImagesIOPlugin:'" << fname 
			  << "' XML file is not a segmentation set\n"; 
		return PSegSetWithImages();
	}
	
	bfs::path src_path_(fname);
	src_path_.remove_filename();
	auto src_path = src_path_.string();
	return  PSegSetWithImages(new CSegSetWithImages(doc, src_path));

}

bool CXMLSegSetWithImagesIOPlugin::do_save(const string& fname, const CSegSetWithImages& data) const
{
	auto outset = data.write();
	ofstream outfile(fname.c_str() );
	if (outfile.good()) {
		outfile << outset.write_to_string();
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

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CXMLSegSetWithImagesIOPlugin();
}

NS_END
