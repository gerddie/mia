/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <mia/3d/landmarklistio.hh>
#include <libxml++/libxml++.h>
#include <mia/core/msgstream.hh>
#include <mia/core/tools.hh>



namespace
 bfs=boost::filesystem; 
using namespace xmlpp;

NS_MIA_USE
using namespace std; 

class C3DLMXLandmarklistIOPlugin: public C3DLandmarklistIOPlugin {
public:
	C3DLMXLandmarklistIOPlugin();
private:
	virtual P3DLandmarklist do_load(string const&  filename)const;
	virtual bool do_save(string const&  filename, const C3DLandmarklist& data) const;
	virtual const string do_get_descr() const;

	P3DLandmark get_landmark(const Node& node) const ; 
	void add_landmark(Element* lmnode, const C3DLandmark& lm) const;  
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DLMXLandmarklistIOPlugin();
}

C3DLMXLandmarklistIOPlugin::C3DLMXLandmarklistIOPlugin():
C3DLandmarklistIOPlugin("lmx")
{
	add_suffix(".lmx"); 
}

template <typename Expect> 
bool translate_value(const Glib::ustring& content, Expect& result) 
{
	istringstream shelp(content.raw()); 
	shelp >> result; 
	bool good = !shelp.fail();
	cvdebug() << "Get streamable type from from '" << content << ( good ? " success" : " fail") << "\n"; 
	return good; 

}

template <> 
bool translate_value(const Glib::ustring& content, string& result) 
{
	result= content.raw();
	return true; 
}

template <> 
bool translate_value(const Glib::ustring& content, C3DFVector& result) 
{
	istringstream shelp(content.raw());
	shelp >> result.x >>  result.y >> result.z; 
	bool good = !shelp.fail();
	cvdebug() << "Get 3D vector from '" << content << ( good ? " success" : " fail") << "\n"; 
	return good; 

}

template <> 
bool translate_value(const Glib::ustring& content, Quaternion& result) 
{
	istringstream shelp(content.raw());
	double x,y,z,w; 
	shelp >> x >> y >> z >> w; 
	result = Quaternion(w,x,y,z); 
	bool good = !shelp.fail();
	cvdebug() << "Get quaternion from '" << content << ( good ? " success" : " fail") << "\n"; 
	return good; 
}


template <typename Expect> 
bool get_single_xml_value(const Node& node, const string& tag, Expect& result) 
{
	cvdebug() << "search value for tag '" << tag << "'\n"; 
	auto children = node.get_children(tag); 
	if (children.empty()) { 
		cvdebug() << "  tag not found\n"; 
		return false; 
	}
	if (children.size() > 1) {
		cvwarn() << "C3DLMXLandmarklistIOPlugin:Tag '" << tag 
			 << "' specified more then once. Only reading first."; 
	}

	auto  content = dynamic_cast<Element*>(*children.begin());
	if (!content) {
		cvdebug() << "  got empty node\n"; 
		return false; 
	}
	
	auto text = content->get_child_text(); 
	if (!text) {
		cvdebug() << "  no text in node\n"; 
		return false; 
	}
	
	return translate_value(text->get_content(), result); 
}

template <> 
bool get_single_xml_value(const Node& node, const string& tag, C3DCamera& result)
{

	auto children = node.get_children(tag); 
	if (children.empty()) 
		return false; 
	if (children.size() > 1) {
		cvwarn() << "C3DLMXLandmarklistIOPlugin:Tag '" << tag 
			 << "' specified more then once. Only reading first.";
	}
	
	C3DFVector location; 
	float distance; 
	Quaternion rotation;
	float zoom; 

	auto  content = *children.begin();
	bool camera_complete = 
		get_single_xml_value(*content, "location", location) && 
		get_single_xml_value(*content, "zoom", zoom) && 
		get_single_xml_value(*content, "rotation", rotation); 

	// work around old style saving that uses 
	if (get_single_xml_value(*content, "distance", distance)) 
		location.z = distance; 
	
	if (camera_complete) {
		result = C3DCamera(location, rotation, zoom); 
		return true; 
	}else {
		cvwarn() << "C3DLMXLandmarklistIOPlugin: discarding an incomplete camera description.\n";
		return false; 
	}
}


P3DLandmark C3DLMXLandmarklistIOPlugin::get_landmark(const Node& node) const 
{

	string name; 
	string picfile; 
	float isovalue; 
	C3DFVector location;
	
	if (!get_single_xml_value(node, "name", name))
		throw create_exception<runtime_error>("C3DLMXLandmarklistIOPlugin: Bogus file, landmark doesn't have a name"); 

	P3DLandmark lm = make_shared<C3DLandmark>(name); 
	
	if (get_single_xml_value(node, "location", location))
		lm->set_location(location); 
	
	if (get_single_xml_value(node, "isovalue", isovalue))
		lm->set_isovalue(isovalue); 
	
	if (get_single_xml_value(node, "picfile", picfile))
		lm->set_picture_file(picfile);
	
	
	C3DCamera camera; 
	if (get_single_xml_value(node, "camera", camera))
		lm->set_view(camera);

	return lm; 
}

P3DLandmarklist C3DLMXLandmarklistIOPlugin::do_load(string const&  filename)const
{
	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(filename);
	
	if (!parser) {
		cvinfo() << "C3DLMXLandmarklistIOPlugin: unable to parse '" << filename << "'\n"; 
		return P3DLandmarklist(); 
	}

	auto root = parser.get_document()->get_root_node ();
	if (root->get_name() != "list") {
		cvinfo() << "C3DLMXLandmarklistIOPlugin: input file '" 
			 << filename << "'is XML but lot an lmx file\n";
		return P3DLandmarklist(); 
	}

	P3DLandmarklist result(new C3DLandmarklist);

	bfs::path bfsfilename(filename); 
	result->set_path(bfsfilename.root_path()); 

	// get name of the set 
	string name;  
	if ( get_single_xml_value(*root, "name", name))
		result->set_name(name); 
	
	auto lmdescr = root->get_children("landmark");
	auto i = lmdescr.begin();
	auto e = lmdescr.end();

	while (i != e) {
		result->add(get_landmark(**i));
		++i;
	}
	return result; 
}

template <typename T>
void add_node(Element& parent, const string& name, const T& value) 
{
	ostringstream s;
	s << value; 
	auto node = parent.add_child(name);
	node->set_child_text(s.str());
}

template <>
void add_node(Element& parent, const string& name, const string& value) 
{
	auto node = parent.add_child(name);
	node->set_child_text(value);
}

template <typename T>
void add_node(Element& parent, const string& name, const T3DVector<T>& value) 
{
	ostringstream s;
	s << value.x << " " << value.y << " " << value.z;

	auto node = parent.add_child(name);
	node->set_child_text(s.str());
}

template <>
void add_node(Element& parent, const string& name, const Quaternion& value) 
{
	ostringstream s;
	s << value.x() << " " << value.y() << " " << value.z() << " " << value.w() ;
	auto node = parent.add_child(name);
	node->set_child_text(s.str());
}

template <>
void add_node(Element& parent, const string& name, const C3DCamera& value) 
{
	auto node = parent.add_child(name);
	add_node(*node, "location", value.get_location()); 
	add_node(*node, "rotation", value.get_rotation()); 
	add_node(*node, "zoom", value.get_zoom()); 
}

void C3DLMXLandmarklistIOPlugin::add_landmark(Element* lmnode, const C3DLandmark& lm) const 
{
	add_node(*lmnode, "name", lm.get_name()); 
	add_node(*lmnode, "picfile", lm.get_picture_file());
	if (lm.has_location()) 
		add_node(*lmnode, "location", lm.get_location());
	add_node(*lmnode, "isovalue", lm.get_isovalue());
	add_node(*lmnode, "camera", lm.get_view());
}

bool C3DLMXLandmarklistIOPlugin::do_save(string const&  filename, const C3DLandmarklist& data) const
{
	unique_ptr<Document> doc(new Document);
	
	Element* list = doc->create_root_node("list");
	add_node(*list, "name", data.get_name()); 
	
	for(auto i = data.begin(); i != data.end(); ++i) {
		auto lmnode = list->add_child("landmark"); 
		add_landmark(lmnode, *i->second);
	}
	
	ofstream outfile(filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << doc->write_to_string_formatted();
	else 
		cvdebug() << "Unable to open file '" << filename << "'\n"; 
	return outfile.good();
}

const string C3DLMXLandmarklistIOPlugin::do_get_descr() const
{
	return "IO plugin to load and store 3D landmark lists"; 
}
