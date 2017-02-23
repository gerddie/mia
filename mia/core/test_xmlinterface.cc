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

#include <mia/internal/autotest.hh>
#include <mia/core/xmlinterface.hh>

using namespace mia;
using namespace std;

BOOST_AUTO_TEST_CASE( test_nested_xml_doc_string )
{
        CXMLDocument doc;
        
        auto root_node = doc.create_root_node("root");
 
        auto child1 = root_node->add_child("child1");
        child1->set_attribute("attribute1", "1");
        child1->set_attribute("attribute2", "2");
	child1->set_attribute("attribute2", "2");
        child1->set_child_text("Some <text>&");

        auto child2 = root_node->add_child("child2");
        child2->set_attribute("attribute21", "3");
        child2->set_attribute("attribute22", "4");


        auto result_string = doc.write_to_string();

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
 
        auto child1 = root_node->add_child("child1");
        child1->set_attribute("attribute1", "1");
        child1->set_attribute("attribute2", "2");

        auto child2 = child1->add_child("child2");
        child2->set_attribute("attribute21", "3");
        child2->set_attribute("attribute22", "4");


        auto result_string = doc.write_to_string();

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

BOOST_AUTO_TEST_CASE( test_xml_doc_string_with_content_and_special_values )
{
        CXMLDocument doc;
        
        auto root_node = doc.create_root_node("root");
 
        auto child1 = root_node->add_child("child1");
        child1->set_attribute("attribute1", "1");
        child1->set_attribute("attribute2", "2");
	child1->set_child_text("1 <> 2");
	
        auto child2 = child1->add_child("child2");
        child2->set_attribute("attribute21", "3");
        child2->set_attribute("attribute22", "4");
	

        auto result_string = doc.write_to_string();

        cvdebug() << "Result=\n" << result_string << "\n";

        const char *test_string =
                "<?xml version=\"1.0\"?>\n"
                "<root>\n"
                "  <child1 attribute1=\"1\" attribute2=\"2\">1 &lt;&gt; 2"
                "<child2 attribute21=\"3\" attribute22=\"4\"/>"
                "</child1>\n"
                "</root>\n"; 

        BOOST_CHECK_EQUAL(result_string, string(test_string)); 

}               


BOOST_AUTO_TEST_CASE( test_xml_doc_string_with_content )
{
        CXMLDocument doc;
        
        auto root_node = doc.create_root_node("root");
 
        auto child1 = root_node->add_child("child1");
        child1->set_attribute("attribute1", "1");
        child1->set_attribute("attribute2", "2");
	child1->set_child_text("some text");
	
        auto child2 = child1->add_child("child2");
        child2->set_attribute("attribute21", "3");
        child2->set_attribute("attribute22", "4");
	

        auto result_string = doc.write_to_string();

        cvdebug() << "Result=\n" << result_string << "\n";

        const char *test_string =
                "<?xml version=\"1.0\"?>\n"
                "<root>\n"
                "  <child1 attribute1=\"1\" attribute2=\"2\">some text"
                "<child2 attribute21=\"3\" attribute22=\"4\"/>"
                "</child1>\n"
                "</root>\n"; 

        BOOST_CHECK_EQUAL(result_string, string(test_string)); 

}               


BOOST_AUTO_TEST_CASE( test_simple_xml_doc_string_read )
{
        const char *test_string =
                "<?xml version=\"1.0\"?>\n"
                "<root version=\"3\">\n"
                "  <frame attribute1=\"1\" attribute2=\"2\">\n"
                "    <child attribute21=\"3\" attribute22=\"4\"/>\n"
                "  </frame>\n"
		"  <frame attribute1=\"5\" attribute2=\"6\">\n"
                "    <child attribute21=\"7\" attribute22=\"8\"/>\n"
		"    <child attribute21=\"9\" attribute22=\"A\"/>\n"
                "  </frame>\n"
		"  <text>This is a text</text>\n"
                "</root>\n"; 
	

	CXMLDocument doc(test_string);
        
	
        auto root = doc.get_root_node();

	BOOST_CHECK_EQUAL(root->get_attribute("version"), "3");
	
	auto frames = root->get_children("frame");
	BOOST_CHECK_EQUAL(frames.size(), 2u);
	BOOST_REQUIRE(frames.size() == 2u);

	auto f0 = frames[0];
	BOOST_CHECK_EQUAL(f0->get_attribute("attribute1"), "1");
	BOOST_CHECK_EQUAL(f0->get_attribute("attribute2"), "2");

	auto children0 = f0->get_children("child");
	BOOST_REQUIRE(children0.size() == 1u);

	auto c00 =  children0[0];
	BOOST_CHECK_EQUAL(c00->get_attribute("attribute21"), "3");
	BOOST_CHECK_EQUAL(c00->get_attribute("attribute22"), "4");


	auto f1 = frames[1];
	BOOST_CHECK_EQUAL(f1->get_attribute("attribute1"), "5");
	BOOST_CHECK_EQUAL(f1->get_attribute("attribute2"), "6");

	auto children1 = f1->get_children("child");
	BOOST_REQUIRE(children1.size() == 2u);

	auto c10 =  children1[0];
	BOOST_CHECK_EQUAL(c10->get_attribute("attribute21"), "7");
	BOOST_CHECK_EQUAL(c10->get_attribute("attribute22"), "8");

	auto c11 =  children1[1];
	BOOST_CHECK_EQUAL(c11->get_attribute("attribute21"), "9");
	BOOST_CHECK_EQUAL(c11->get_attribute("attribute22"), "A");

	auto textnode = root->get_children("text");
	BOOST_REQUIRE(textnode.size() == 1u);
	
	auto text = textnode[0]->get_content();
	BOOST_CHECK_EQUAL(text, "This is a text"); 
}

BOOST_AUTO_TEST_CASE( test_simple_xml_doc_string_read_with_special_chars )
{
        const char *test_string =
                "<?xml version=\"1.0\"?>\n"
                "<root version=\"3\">\n"
		"  <text>1 &lt; 2; 2 &gt; 1</text>\n"
                "</root>\n"; 
	

	CXMLDocument doc(test_string);
        
	
        auto root = doc.get_root_node();

	BOOST_CHECK_EQUAL(root->get_attribute("version"), "3");
	
	auto textnode = root->get_children("text");
	BOOST_REQUIRE(textnode.size() == 1u);
	
	auto text = textnode[0]->get_content();
	BOOST_CHECK_EQUAL(text, "1 < 2; 2 > 1"); 
}

BOOST_AUTO_TEST_CASE( test_simple_xml_broken1 )
{
        const char *test_string =
                "<?xml version="; 
	

	BOOST_CHECK_THROW(CXMLDocument(test_string).get_root_node(),runtime_error);

	CXMLDocument manual_read; 
	BOOST_CHECK(!manual_read.read_from_string(test_string));
}

BOOST_AUTO_TEST_CASE( test_simple_xml_broken2 )
{
        const char *test_string =
                "<?xml version=\"1.0\"?>\n"
                "<root version=\"3\">\n"
		"  <text>1 &lt; 2; 2 &gt; 1</text>\n"; 
	

	BOOST_CHECK_THROW(CXMLDocument(test_string).get_root_node(),runtime_error);

	CXMLDocument manual_read; 
	BOOST_CHECK(!manual_read.read_from_string(test_string));
}
