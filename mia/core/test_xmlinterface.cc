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

#include <mia/internal/autotest.hh>
#include <mia/core/xmlinterface.hh>

using namespace mia;
using namespace std;

BOOST_AUTO_TEST_CASE( test_nested_xml_doc_string )
{
        CXMLDocument doc;
        
        auto root_node = doc.create_root_node("root");
 
        CXMLElement* child1 = root_node->add_child("child1");
        child1->set_attribute("attribute1", "1");
        child1->set_attribute("attribute2", "2");
	child1->set_attribute("attribute2", "2");
        child1->set_child_text("Some <text>&");

        CXMLElement* child2 = root_node->add_child("child2");
        child2->set_attribute("attribute21", "3");
        child2->set_attribute("attribute22", "4");


        auto result_string = doc.write_to_string_formatted();

        cvdebug() << "Result=\n" << result_string << "\n";

        const char *test_string =
                "<?xml version=\"1.0\"?>\n"
                "<root>\n"
                "  <child1 attribute1=\"1\" attribute2=\"2\">Some &lt;text&gt;&amp;</child1>\n"
                "  <child2 attribute21=\"3\" attribute22=\"4\"/>\n"
                "</root>\n"; 

        BOOST_CHECK_EQUAL(result_string, string(test_string)); 

}               

BOOST_AUTO_TEST_CASE( test_simple_xml_doc_string )
{
        CXMLDocument doc;
        
        auto root_node = doc.create_root_node("root");
 
        CXMLElement* child1 = root_node->add_child("child1");
        child1->set_attribute("attribute1", "1");
        child1->set_attribute("attribute2", "2");

        CXMLElement* child2 = child1->add_child("child2");
        child2->set_attribute("attribute21", "3");
        child2->set_attribute("attribute22", "4");


        auto result_string = doc.write_to_string_formatted();

        cvdebug() << "Result=\n" << result_string << "\n";

        const char *test_string =
                "<?xml version=\"1.0\"?>\n"
                "<root>\n"
                "  <child1 attribute1=\"1\" attribute2=\"2\">\n"
                "    <child2 attribute21=\"3\" attribute22=\"4\"/>\n"
                "  </child1>\n"
                "</root>\n"; 

        BOOST_CHECK_EQUAL(result_string, string(test_string)); 

}               
