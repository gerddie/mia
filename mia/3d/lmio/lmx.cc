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

#include <mia/3d/landmarklistio.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/tools.hh>
#include <mia/core/xmlinterface.hh>
#include <boost/filesystem.hpp>


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

namespace  bfs = boost::filesystem;


namespace mia
{
template <>
bool from_string(const char *s, C3DFVector& result)
{
       std::istringstream sx(s);
       sx >> result.x >> result.y >> result.z;

       if (sx.fail())
              return false;

       if (sx.eof())
              return true;

       std::string remaining;
       sx >> remaining;
       bool retval = true;

       for (auto i = remaining.begin(); i != remaining.end(); ++i)
              retval &= isspace(remaining[0]);

       return retval;
}

template <>
bool from_string(const char *s, Quaternion& result)
{
       std::istringstream sx(s);
       double w,  x,  y, z;
       sx >> x >> y >> z >> w;
       result = Quaternion(w, x, y, z);

       if (sx.fail())
              return false;

       if (sx.eof())
              return true;

       std::string remaining;
       sx >> remaining;
       bool retval = true;

       for (auto i = remaining.begin(); i != remaining.end(); ++i)
              retval &= isspace(remaining[0]);

       return retval;
}

}

NS_MIA_USE
using namespace std;

class C3DLMXLandmarklistIOPlugin: public C3DLandmarklistIOPlugin
{
public:
       C3DLMXLandmarklistIOPlugin();
private:
       virtual P3DLandmarklist do_load(string const&  filename)const;
       virtual bool do_save(string const&  filename, const C3DLandmarklist& data) const;
       virtual const string do_get_descr() const;

       P3DLandmark get_landmark(const CXMLElement& node) const ;
       void add_landmark(CXMLElement& lmnode, const C3DLandmark& lm) const;
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
bool get_single_xml_value(const CXMLElement& node, const string& tag, Expect& result)
{
       cvdebug() << "search value for tag '" << tag << "' in node '" << node.get_name() <<  "'\n";
       auto children = node.get_children(tag.c_str());

       if (children.empty()) {
              cvdebug() << "  tag not found\n";
              return false;
       }

       if (children.size() > 1) {
              cvwarn() << "C3DLMXLandmarklistIOPlugin:Tag '" << tag
                       << "' specified more then once. Only reading first.";
       }

       auto content = children[0]->get_content();
       cvdebug() << " got content  '" << content <<  "'\n";

       if (content.empty()) {
              cvdebug() << "  got empty node\n";
              return false;
       }

       bool r = from_string(content, result);

       if (!r)
              cvdebug() << "ERROR:reading '" << tag << "' from '" << content << "'failed\n";

       return r;
}

template <>
bool get_single_xml_value(const CXMLElement& node, const string& tag, string& result)
{
       cvdebug() << "search value for tag '" << tag << "' in node '" << node.get_name() <<  "'\n";
       auto children = node.get_children(tag.c_str());

       if (children.empty()) {
              cvdebug() << "  tag not found\n";
              return false;
       }

       if (children.size() > 1) {
              cvwarn() << "C3DLMXLandmarklistIOPlugin:Tag '" << tag
                       << "' specified more then once. Only reading first.";
       }

       auto content = children[0]->get_content();
       cvdebug() << " got content  '" << content <<  "'\n";

       if (content.empty()) {
              cvdebug() << "  got empty node\n";
              return false;
       }

       result = content;
       return true;
}

template <>
bool get_single_xml_value(const CXMLElement& node, const string& tag, C3DCamera& result)
{
       cvdebug() << "search value for tag '" << tag << "' in node '" << node.get_name() <<  "'\n";
       auto children = node.get_children(tag.c_str());

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
       auto camera = children[0];
       bool camera_complete =
              get_single_xml_value(*camera, "location", location) &&
              get_single_xml_value(*camera, "zoom", zoom) &&
              get_single_xml_value(*camera, "rotation", rotation);

       // work around old style saving that uses
       if (get_single_xml_value(*camera, "distance", distance))
              location.z = distance;

       if (camera_complete) {
              result = C3DCamera(location, rotation, zoom);
              return true;
       } else {
              cvwarn() << "C3DLMXLandmarklistIOPlugin: discarding an incomplete camera description.\n";
              return false;
       }
}


P3DLandmark C3DLMXLandmarklistIOPlugin::get_landmark(const CXMLElement& node) const
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
       ifstream infile(filename);
       string xml_init(std::istreambuf_iterator<char> {infile}, {});

       if (!infile.good()) {
              throw create_exception<runtime_error>("C3DLMXLandmarklistIOPlugin: Unable to read input file: '", filename, "'");
       }

       CXMLDocument doc;

       if (!doc.read_from_string(xml_init.c_str())) {
              cvdebug() << "C3DLMXLandmarklistIOPlugin:'" << filename
                        << "' not an XML file\n";
              return P3DLandmarklist();
       }

       auto root = doc.get_root_node();
       P3DLandmarklist result(new C3DLandmarklist);
       bfs::path bfsfilename(filename);
       result->set_path(bfsfilename.root_path().string());
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
void add_node(CXMLElement& parent, const string& name, const T& value)
{
       ostringstream s;
       s << value;
       auto node = parent.add_child(name.c_str());
       node->set_child_text(s.str());
}

template <>
void add_node(CXMLElement& parent, const string& name, const string& value)
{
       auto node = parent.add_child(name.c_str());
       node->set_child_text(value);
}

template <typename T>
void add_node(CXMLElement& parent, const string& name, const T3DVector<T>& value)
{
       ostringstream s;
       s << value.x << " " << value.y << " " << value.z;
       auto node = parent.add_child(name.c_str());
       node->set_child_text(s.str());
}

template <>
void add_node(CXMLElement& parent, const string& name, const Quaternion& value)
{
       ostringstream s;
       s << value.x() << " " << value.y() << " " << value.z() << " " << value.w() ;
       auto node = parent.add_child(name.c_str());
       node->set_child_text(s.str());
}

template <>
void add_node(CXMLElement& parent, const string& name, const C3DCamera& value)
{
       auto node = parent.add_child(name.c_str());
       add_node(*node, "location", value.get_location());
       add_node(*node, "rotation", value.get_rotation());
       add_node(*node, "zoom", value.get_zoom());
}

void C3DLMXLandmarklistIOPlugin::add_landmark(CXMLElement& lmnode, const C3DLandmark& lm) const
{
       add_node(lmnode, "name", lm.get_name());
       add_node(lmnode, "picfile", lm.get_picture_file());

       if (lm.has_location())
              add_node(lmnode, "location", lm.get_location());

       add_node(lmnode, "isovalue", lm.get_isovalue());
       add_node(lmnode, "camera", lm.get_view());
}

bool C3DLMXLandmarklistIOPlugin::do_save(string const&  filename, const C3DLandmarklist& data) const
{
       CXMLDocument doc;
       auto list = doc.create_root_node("list");
       add_node(*list, "name", data.get_name());

       for (auto i = data.begin(); i != data.end(); ++i) {
              auto lmnode = list->add_child("landmark");
              add_landmark(*lmnode, *i->second);
       }

       ofstream outfile(filename.c_str(), ios_base::out );

       if (outfile.good())
              outfile << doc.write_to_string();
       else
              cvdebug() << "Unable to open file '" << filename << "'\n";

       return outfile.good();
}

const string C3DLMXLandmarklistIOPlugin::do_get_descr() const
{
       return "IO plugin to load and store 3D landmark lists";
}
