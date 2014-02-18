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

#include <mia/internal/autotest.hh>

#include <numeric>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/BoundingBox.hh>
#include <mia/2d/inittesthandlers.hh>
#include <mia/2d/imageio.hh>
#include <libxml++/libxml++.h>

using namespace mia; 
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace xmlpp;
namespace bfs=boost::filesystem;

PrepareTestPluginPath plugin_path_init; 

extern const char *testframe_init;
extern const char *testframe_init2;
const size_t size = 3;
const float x_init[size] = {10.0, 20.0, 0.0};
const float y_init[size] = {20.0, 10.0, 4.0};
const float x_init2[size] = {11.0, 21.0, 1.0};
const float y_init2[size] = {21.0, 11.0, 5.0};


BOOST_AUTO_TEST_CASE( test_frame_rename_files ) 
{
	CSegStar star; 
	CSegFrame::Sections sections; 
	
	CSegFrame frame("image000.png", star, sections);
	
	frame.rename_base("test"); 

	BOOST_CHECK_EQUAL(frame.get_imagename(), "test000.png"); 
}

struct FrameTestRead {
	void init(const char *init_str);
	CSegFrame frame;
};

void FrameTestRead::init(const char *init_str)
{
	xmlpp::DomParser parser;
	parser.parse_memory(init_str);
	const xmlpp::Document *document = parser.get_document();
	const xmlpp::Element *root = document->get_root_node ();
	const xmlpp::Node::NodeList nodes = root->get_children();
	BOOST_CHECK_EQUAL(nodes.size(),1u);

	frame = CSegFrame (**nodes.begin(), 1);
}



BOOST_FIXTURE_TEST_CASE(segframe_read, FrameTestRead)
{
	init(testframe_init);


	CSegFrame::Sections sections = frame.get_sections();

	BOOST_CHECK_EQUAL(sections.size(), 2u);
	BOOST_REQUIRE(sections.size() == 2u);

	BOOST_CHECK_EQUAL(sections[0].get_id(), "white");
	BOOST_CHECK_EQUAL(sections[1].get_id(), "red");

	BOOST_CHECK_EQUAL(frame.get_imagename(), "image.png");

	const CSegStar& star = frame.get_star();

	BOOST_CHECK_EQUAL(star.m_radius, 21);
}

BOOST_FIXTURE_TEST_CASE(segframe_read2, FrameTestRead)
{
	init(testframe_init2);

	CSegFrame::Sections sections = frame.get_sections();

	BOOST_CHECK_EQUAL(sections.size(), 3u);
	BOOST_REQUIRE(sections.size() == 3);

	BOOST_CHECK_EQUAL(sections[0].get_id(), "white");
	BOOST_CHECK_EQUAL(sections[1].get_id(), "red");
	BOOST_CHECK_EQUAL(sections[2].get_id(), "blue");

	BOOST_CHECK_EQUAL(frame.get_imagename(), "image.png");

	const CSegStar& star = frame.get_star();

	BOOST_CHECK_EQUAL(star.m_radius, 21);
}

extern const char *testframe_shifted;
#if 0 // shift should not be used 

BOOST_FIXTURE_TEST_CASE(segframe_shift, FrameTestRead)
{
	init(testframe_init);
	frame.shift(C2DFVector(1.0, 2.0), "newname");
	BOOST_CHECK_EQUAL(frame.get_imagename(), "newname");

	xmlpp::Document document;
	xmlpp::Element* nodeRoot = document.create_root_node("test");
	frame.write(*nodeRoot);

	const string xmldoc = document.write_to_string();
	const string testdoc(testframe_shifted);

	BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
	BOOST_CHECK_EQUAL(xmldoc, testdoc);


}
#endif


const char *testframe_init = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test>"
	"<frame image=\"image.png\">"
	"<star y=\"118\" x=\"109\" r=\"21\">"
	"<point y=\"1\" x=\"0\"/>"
	"<point y=\"-0.5\" x=\"0.866025\"/>"
	"<point y=\"-0.5\" x=\"-0.866025\"/>"
	"</star>"
	"<section color=\"white\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
	"</section>"
	"<section color=\"red\">"
	"<point y=\"21\" x=\"11\"/>"
	"<point y=\"11\" x=\"21\"/>"
	"<point y=\"5\" x=\"1\"/>"
	"</section>"
	"</frame>"
	"</test>\n";


const char *testframe_shifted = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test>"
	"<frame image=\"newname\">"
	"<star y=\"116\" x=\"108\" r=\"21\">"
	"<point y=\"1\" x=\"0\"/>"
	"<point y=\"-0.5\" x=\"0.866025\"/>"
	"<point y=\"-0.5\" x=\"-0.866025\"/>"
	"</star>"
	"<section color=\"white\">"
	"<point y=\"18\" x=\"9\"/>"
	"<point y=\"8\" x=\"19\"/>"
	"<point y=\"2\" x=\"-1\"/>"
	"</section>"
	"<section color=\"red\">"
	"<point y=\"19\" x=\"10\"/>"
	"<point y=\"9\" x=\"20\"/>"
	"<point y=\"3\" x=\"0\"/>"
	"</section>"
	"</frame>"
	"</test>\n";


BOOST_FIXTURE_TEST_CASE(segframe_transform, FrameTestRead)
{

	P2DTransformationFactory transform_creater = C2DTransformCreatorHandler::instance().produce("translate");
	P2DTransformation transform = transform_creater->create(C2DBounds(10,20));
	auto params = transform->get_parameters(); 
	params[0] = 1.0; 
	params[1] = 2.0; 
	transform->set_parameters(params); 
	
	init(testframe_init);
	
	frame.transform(*transform);
	frame.set_imagename("newname"); 

	xmlpp::Document document;
	xmlpp::Element* nodeRoot = document.create_root_node("test");
	frame.write(*nodeRoot, 1);

	const string xmldoc = document.write_to_string("UTF-8");
	const string testdoc(testframe_shifted);

	BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
	BOOST_CHECK_EQUAL(xmldoc, testdoc);

}


BOOST_FIXTURE_TEST_CASE(test_frame_get_mask, FrameTestRead)
{
	const char *sestsection_for_draw2 = 
		"<?xml version=\"1.0\"?>\n<test>"
		"<frame image=\"framename\">"
                "<section color=\"white\">"
		"<point y=\"2\" x=\"1\"/>"
		"<point y=\"4\" x=\"1\"/>"
		"<point y=\"4\" x=\"3\"/>"
		"<point y=\"2\" x=\"3\"/>"
		"</section>"
                "<section color=\"blue\">"
		"<point y=\"0\" x=\"2\"/>"
		"<point y=\"2\" x=\"2\"/>"
		"<point y=\"2\" x=\"3\"/>"
		"<point y=\"3\" x=\"3\"/>"
		"<point y=\"3\" x=\"4\"/>"
		"<point y=\"0\" x=\"4\"/>"
		"</section>"
		"</frame></test>\n";

	init(sestsection_for_draw2);
	
	C2DBounds size(6,7); 

	unsigned char test_data[6 * 7] =  {
		0, 0, 2, 2, 0, 0, 
		0, 0, 2, 2, 0, 0, 
		0, 1, 1, 2, 0, 0, 
		0, 1, 1, 0, 0, 0, 
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0
	}; 

	C2DUBImage section_mask = frame.get_section_masks(size); 
	auto t = test_data;
	for (auto i = section_mask.begin(); i != section_mask.end(); ++i, ++t) 
		BOOST_CHECK_EQUAL(*i, *t); 

}


BOOST_FIXTURE_TEST_CASE(test_frame_get_mask_different, FrameTestRead)
{

	CSegPoint2D center(7.6,7.59); 
	float r = 4; 
	CSegPoint2D d1(1.0, 0.0); 
	CSegPoint2D d2(0.0,-1.0); 
	CSegPoint2D d3(-1.0,0.0); 
	CSegStar star(center, r, d1, d2, d3); 
	CSegSection::Points points; 
	
	for(size_t k = 0; k < 21; ++k) {
		CSegPoint2D p(center.x + (r+2.0) * cos(k / 20.0 * 2 * M_PI), 
			      center.y + (r+2.0) * sin(k / 20.0 * 2 * M_PI)); 
		points.push_back(p); 
	}
	for(int k = 21;  k >= 0; --k) {
		CSegPoint2D p(center.x + (r-2.0) * cos(k / 20.0 * 2 * M_PI), 
			      center.y + (r-2.0) * sin(k / 20.0 * 2 * M_PI));
		points.push_back(p); 
	}

	CSegFrame::Sections sections; 
	sections.push_back(CSegSection("test", points, false));
	
	CSegFrame frame("testimage.@", star, sections);

	C2DBounds size(16,16); 
	P2DImage image(new C2DFImage(size)); 
	save_image("testimage.@", image); 


	C2DUBImage section_mask = frame.get_section_masks(8); 
	

	const unsigned char test_data[] = {
		/*      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 */
		/*0*/	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		
		/*1*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		
		/*2*/   0, 0, 0, 0, 0, 6, 6, 6, 7, 0, 0, 0, 0, 0, 0, 0,
		
		/*3*/   0, 0, 0, 5, 6, 6, 6, 6, 7, 7, 7, 0, 0, 0, 0, 0,
		
		/*4*/   0, 0, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 0, 0, 0, 0,

		/*5*/   0, 0, 5, 5, 5, 5, 6, 6, 7, 7, 7, 8, 0, 0, 0, 0,

		/*6*/   0, 5, 5, 5, 5, 5, 0, 0, 7, 7, 8, 8, 8, 0, 0, 0,
		
		/*7*/   0, 5, 5, 5, 5, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0,
                /*                            x                      */   
		/*8*/   0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
		
		/*9*/   0, 4, 4, 4, 4, 4, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0,
		
		/*10*/  0, 0, 4, 4, 4, 4, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0,
		
		/*11*/  0, 0, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 0, 0, 0, 0,
		
		/*12*/  0, 0, 0, 4, 3, 3, 3, 3, 2, 2, 2, 0, 0, 0, 0, 0,
		
		/*13*/  0, 0, 0, 0, 0, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0,
		
		/*14*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		
		/*15*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	}; 
	const unsigned char *t = test_data; 
	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x, ++t) {
			cvdebug() << y<< ", " << x << ":" 
				  << (int)section_mask(x,y) << ", " << (int)*t << "\n"; 
			BOOST_CHECK_EQUAL(section_mask(x,y), *t); 
		}
	
	C2DUBImage section_mask2 = frame.get_section_masks(0); 
	t = test_data; 
	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x, ++t) {
			if (*t) 
				BOOST_CHECK_EQUAL(section_mask2(x,y),1);
			else 
				BOOST_CHECK_EQUAL(section_mask2(x,y),0);
		}
	
}


BOOST_FIXTURE_TEST_CASE(test_frame_get_mask_size, FrameTestRead)
{
	const char *sestsection_for_draw2 = 
		"<?xml version=\"1.0\"?>\n<test>"
		"<frame image=\"name.@\">"
                "<section color=\"white\">"
		"<point y=\"2\" x=\"1\"/>"
		"<point y=\"4\" x=\"1\"/>"
		"<point y=\"4\" x=\"3\"/>"
		"<point y=\"2\" x=\"3\"/>"
		"</section>"
                "<section color=\"blue\">"
		"<point y=\"0\" x=\"2\"/>"
		"<point y=\"2\" x=\"2\"/>"
		"<point y=\"2\" x=\"3\"/>"
		"<point y=\"3\" x=\"3\"/>"
		"<point y=\"3\" x=\"4\"/>"
		"<point y=\"0\" x=\"4\"/>"
		"</section>"
		"</frame></test>\n";

	init(sestsection_for_draw2);
	
	C2DBounds size(6,7); 

	float test_img[6 * 7] =  {
		0, 0, 2, 3, 0, 0, 
		0, 0, 4, 5, 0, 0, 
		0, 1, 2, 6, 0, 0, 
		0, 2, 1, 0, 0, 0, 
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0
	}; 

	C2DFImage *pimg = new C2DFImage(size, test_img); 
	P2DImage img(pimg); 
	save_image("name.@", img);
	
	C2DUBImage section_mask = frame.get_section_masks(); 
	BOOST_CHECK_EQUAL(section_mask.get_size(),size); 
}; 

BOOST_FIXTURE_TEST_CASE(test_frame_get_stats, FrameTestRead)
{
	const char *sestsection_for_draw2 = 
		"<?xml version=\"1.0\"?>\n<test>"
		"<frame image=\"name.@\">"
                "<section color=\"white\">"
		"<point y=\"2\" x=\"1\"/>"
		"<point y=\"4\" x=\"1\"/>"
		"<point y=\"4\" x=\"3\"/>"
		"<point y=\"2\" x=\"3\"/>"
		"</section>"
                "<section color=\"blue\">"
		"<point y=\"0\" x=\"2\"/>"
		"<point y=\"2\" x=\"2\"/>"
		"<point y=\"2\" x=\"3\"/>"
		"<point y=\"3\" x=\"3\"/>"
		"<point y=\"3\" x=\"4\"/>"
		"<point y=\"0\" x=\"4\"/>"
		"</section>"
		"</frame></test>\n";

	init(sestsection_for_draw2);
	
	C2DBounds size(6,7); 

	float test_img[6 * 7] =  {
		0, 0, 2, 3, 0, 0, 
		0, 0, 4, 5, 0, 0, 
		0, 1, 2, 6, 0, 0, 
		0, 2, 1, 0, 0, 0, 
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0
	}; 

	C2DFImage *pimg = new C2DFImage(size, test_img); 
	P2DImage img(pimg); 
	save_image("name.@", img);
	
	C2DUBImage section_mask = frame.get_section_masks(size); 
	CSegFrame::SectionsStats stats1 =frame.get_stats(section_mask); 
	
	BOOST_CHECK_EQUAL(stats1.size(), 2u); 
	BOOST_CHECK_CLOSE(stats1[0].first, 1.5, 0.1); 
	BOOST_CHECK_CLOSE(stats1[0].second, sqrt(1.0/3.0), 0.1); 

	BOOST_CHECK_CLOSE(stats1[1].first, 4.0, 0.1); 
	BOOST_CHECK_CLOSE(stats1[1].second, sqrt(10.0)/2.0, 0.1); 

}


BOOST_AUTO_TEST_CASE(segframe_write)
{
	CSegStar star(CSegPoint2D(109, 118), 21,
		      CSegPoint2D(0, 1), CSegPoint2D(0.866025,-0.5), CSegPoint2D(-0.866025,-0.5));

	CSegFrame::Sections sections;

	CSegSection::Points points;
	for (size_t i = 0; i  < size; ++i)
		points.push_back(CSegPoint2D(x_init[i], y_init[i]));


	sections.push_back(CSegSection("white", points, 1));

	points.clear();
	for (size_t i = 0; i  < size; ++i)
		points.push_back(CSegPoint2D(x_init2[i], y_init2[i]));

	sections.push_back(CSegSection("red", points, 1));

	CSegFrame frame("image.png", star, sections);

	xmlpp::Document document;
	xmlpp::Element* nodeRoot = document.create_root_node("test");
	frame.write(*nodeRoot, 1);

	const string xmldoc = document.write_to_string("UTF-8");
	const string testdoc(testframe_init);

	BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
	BOOST_CHECK_EQUAL(xmldoc, testdoc);

}

const char *testframe_init2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test>"
	"<frame image=\"image.png\">"
	"<star y=\"118\" x=\"109\" r=\"21\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
	" "
	"</star>"
	"<section color=\"white\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
	"</section>"
	"<section color=\"red\">"
	"<point y=\"21\" x=\"11\"/>"
	"<point y=\"11\" x=\"21\"/>"
	"<point y=\"5\" x=\"1\"/>"
	"</section>"
	"<section color=\"blue\">"
	"<point y=\"21\" x=\"11\"/>"
	"<point y=\"11\" x=\"21\"/>"
	"<point y=\"5\" x=\"1\"/>"
	"</section>"
	"</frame>"
	"</test>\n";

