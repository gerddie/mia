/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

#include <mia/internal/autotest.hh>

#include <numeric>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/BoundingBox.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/2dimageio.hh>

#include <libxml++/libxml++.h>

namespace bfs=boost::filesystem;

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace xmlpp;


const char *testpoint_init  =
	"<?xml version=\"1.0\"?>\n<test><point y=\"20\" x=\"10\"/></test>\n";

BOOST_AUTO_TEST_CASE(segpoint_read)
{
	DomParser parser;
	parser.parse_memory(testpoint_init);

	const Document *document = parser.get_document();
	const Element *root = document->get_root_node ();
	const Node::NodeList nodes = root->get_children("point");
	BOOST_CHECK_EQUAL(nodes.size(),1u);

	for (Node::NodeList::const_iterator i = nodes.begin();
	     i != nodes.end(); ++i) {
		CSegPoint2D p(**i);
		BOOST_CHECK_EQUAL(p.x, 10);
		BOOST_CHECK_EQUAL(p.y, 20);
	}
}

BOOST_AUTO_TEST_CASE(segpoint_write)
{
	Document document;
	Element* nodeRoot = document.create_root_node("test");
	CSegPoint2D point(10,20);
	point.write(*nodeRoot);

	const string xmldoc = document.write_to_string();
	const string testdoc(testpoint_init);
	BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
	BOOST_CHECK_EQUAL(xmldoc, testdoc);
}

const size_t size = 3;
const float x_init[size] = {10.0, 20.0, 0.0};
const float y_init[size] = {20.0, 10.0, 4.0};

const float x_init_shift[size] = {9.0, 19.0, -1.0};
const float y_init_shift[size] = {18.0, 8.0, 2.0};


const float x_init2[size] = {11.0, 21.0, 1.0};
const float y_init2[size] = {21.0, 11.0, 5.0};



extern const char *teststar_init;
extern const char *teststar_init2;
extern const char *testsection_init;
extern const char *testsection_init2;
extern const char *testset_init;
extern const char *testset_init2;
extern const char *testset_init3;
extern const char *testset_bboxtest;
extern const char *testset_shift_and_rename;

struct SegStarFixture {
	void init(const char *init_str);
	void check(const C2DFVector& center, float radius,
		   const float *x_directions, const float *y_directions);

	CSegStar star;
};


void SegStarFixture::init(const char *init_str)
{
	DomParser parser;
	parser.parse_memory(init_str);
	const Document *document = parser.get_document();
	const Element *root = document->get_root_node ();
	const Node::NodeList nodes = root->get_children("star");
	BOOST_CHECK_EQUAL(nodes.size(),1u);

	star = CSegStar(**nodes.begin());
}

void SegStarFixture::check(const C2DFVector& center, float radius,
	   const float *x_directions, const float *y_directions)
{
	BOOST_CHECK_EQUAL(star.m_center.x,center.x);
	BOOST_CHECK_EQUAL(star.m_center.y,center.y);
	BOOST_CHECK_EQUAL(star.m_radius, radius);

	for(size_t i = 0; i < 3; ++i) {
		BOOST_CHECK_EQUAL(star.m_directions[i].x, x_directions[i]);
		BOOST_CHECK_EQUAL(star.m_directions[i].y, y_directions[i]);
	}

}

BOOST_FIXTURE_TEST_CASE(segstar_read, SegStarFixture)
{
	init(teststar_init);
	check(C2DFVector(109, 118), 21, x_init, y_init);
}

BOOST_FIXTURE_TEST_CASE(segstar_read2, SegStarFixture)
{
	init(teststar_init2);
	check(C2DFVector(109, 118), 21, x_init, y_init);

}

BOOST_FIXTURE_TEST_CASE(segstar_shift, SegStarFixture)
{
	init(teststar_init2);
	star.shift(C2DFVector(1.0, 2.0));
	check(C2DFVector(108, 116), 21, x_init, y_init);
}

BOOST_AUTO_TEST_CASE(segstar_write)
{
	CSegStar star(CSegPoint2D(109, 118), 21,
		      CSegPoint2D(10, 20),
		      CSegPoint2D(20, 10),
		      CSegPoint2D(0, 4));

	Document document;
	Element* nodeRoot = document.create_root_node("test");
	star.write(*nodeRoot);

	const string xmldoc = document.write_to_string();

	string teststar(teststar_init);
	BOOST_CHECK_EQUAL(xmldoc.size(), teststar.size());
	BOOST_CHECK_EQUAL(xmldoc, teststar);

}




struct SectionTestRead {

	void init(const char *init_str);
	void check(const float *x_data, const float *y_data) const;

	CSegSection section;
};

BOOST_FIXTURE_TEST_CASE(segment_section_read, SectionTestRead)
{
	init(testsection_init);
	check(x_init, y_init);
}

BOOST_FIXTURE_TEST_CASE(segment_section_shift, SectionTestRead)
{
	init(testsection_init);
	section.shift(C2DFVector(1.0, 2.0));
	check(x_init_shift, y_init_shift);



}


BOOST_FIXTURE_TEST_CASE(segment_section_read_with_whitespaces, SectionTestRead)
{
	init(testsection_init2);
	check(x_init, y_init);
}

BOOST_FIXTURE_TEST_CASE(segment_section_box, SectionTestRead)
{
	init(testsection_init);
	C2DBoundingBox box = section.get_boundingbox();

	BOOST_CHECK_EQUAL(box.get_size(), C2DFVector(20,16));
	BOOST_CHECK_EQUAL(box.get_begin(), C2DFVector(0,4));
	BOOST_CHECK_EQUAL(box.get_end(), C2DFVector(20,20));
}


BOOST_AUTO_TEST_CASE(segment_section_write)
{
	CSegSection::Points points;
	for (size_t i = 0; i  < size; ++i)
		points.push_back(CSegPoint2D(x_init[i], y_init[i]));

	CSegSection section("white", points);

	xmlpp::Document document;
	xmlpp::Element* nodeRoot = document.create_root_node("test");
	section.write(*nodeRoot);

	const string xmldoc = document.write_to_string();
	const string testdoc(testsection_init);

	BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
	BOOST_CHECK_EQUAL(xmldoc, testdoc);
}

struct SegSetReadFixture {
	void init(const char *data);
	CSegSet segset;
};


BOOST_FIXTURE_TEST_CASE( test_segset_read, SegSetReadFixture )
{
	init(testset_init);

	const CSegSet::Frames& frames = segset.get_frames();
	BOOST_CHECK_EQUAL(frames.size(), 2u);

	BOOST_CHECK_EQUAL(segset.get_LV_peak(), 1);
	BOOST_CHECK_EQUAL(segset.get_RV_peak(), 0);

	BOOST_CHECK_EQUAL(frames[0].get_imagename(), "image.png");
	BOOST_CHECK_EQUAL(frames[1].get_imagename(), "image2.png");


	BOOST_CHECK_EQUAL(frames[0].get_star().m_radius, 21);
	BOOST_CHECK_EQUAL(frames[1].get_star().m_radius, 22);
}

BOOST_FIXTURE_TEST_CASE( test_segset_read2, SegSetReadFixture )
{
	init(testset_init2);

	const CSegSet::Frames& frames = segset.get_frames();
	BOOST_CHECK_EQUAL(frames.size(), 2u);

	BOOST_CHECK_EQUAL(frames[0].get_imagename(), "image.png");
	BOOST_CHECK_EQUAL(frames[1].get_imagename(), "image2.png");


	BOOST_CHECK_EQUAL(frames[0].get_star().m_radius, 21);
	BOOST_CHECK_EQUAL(frames[1].get_star().m_radius, 22);
}

BOOST_FIXTURE_TEST_CASE( test_segset_cropbox, SegSetReadFixture )
{
	init(testset_bboxtest);
	C2DBoundingBox box = segset.get_boundingbox();

	BOOST_CHECK_EQUAL(box.get_begin(), C2DFVector(85.76,94.72));
	BOOST_CHECK_EQUAL(box.get_end(), C2DFVector(115.52,124.16));

	BOOST_CHECK_EQUAL(box.get_size(), C2DFVector(115.52f - 85.76f, 124.16f - 94.72f));



}


BOOST_AUTO_TEST_CASE( test_segset_rename_files ) 
{
	CSegStar star; 
	CSegFrame::Sections sections; 
	CSegSet segset; 
	segset.add_frame(CSegFrame("image000.png", star, sections));
	segset.add_frame(CSegFrame("image001.png", star, sections));

	segset.rename_base("test");

	BOOST_CHECK_EQUAL(segset.get_frames()[0].get_imagename(), "test000.png"); 
	BOOST_CHECK_EQUAL(segset.get_frames()[1].get_imagename(), "test001.png"); 
}

BOOST_AUTO_TEST_CASE( test_segset_set_images ) 
{
	CSegStar star; 
	CSegFrame::Sections sections; 
	CSegSetWithImages segset; 
	segset.add_frame(CSegFrame("image000.png", star, sections));
	segset.add_frame(CSegFrame("image001.png", star, sections));

	C2DImageSeries lala2(2); 
	segset.set_images(lala2); 
	
	C2DImageSeries lala3(3); 

	BOOST_CHECK_THROW(segset.set_images(lala3), invalid_argument); 
	

}

extern const char *sestsection_for_draw; 

BOOST_FIXTURE_TEST_CASE(test_segsection_draw, SectionTestRead)
{
	init(sestsection_for_draw);
	C2DUBImage test_image(C2DBounds(10,10)); 

	// sanity test, is the area correct? 
	section.draw(test_image, 1); 
	BOOST_CHECK_EQUAL(accumulate(test_image.begin(),test_image.end(),0.0),31);
}

BOOST_FIXTURE_TEST_CASE(test_segsection_draw2, SectionTestRead)
{
	const char *sestsection_for_draw2 = 
		"<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
		"<point y=\"2\" x=\"1\"/>"
		"<point y=\"4\" x=\"1\"/>"
		"<point y=\"4\" x=\"3\"/>"
		"<point y=\"2\" x=\"3\"/>"
		"</section></test>\n";


	init(sestsection_for_draw2);
	C2DUBImage test_image(C2DBounds(6,7)); 

	// sanity test, is the area correct? 
	section.draw(test_image, 2); 
	BOOST_CHECK_EQUAL(accumulate(test_image.begin(),test_image.end(),0.0),8);

	for(size_t y = 0; y < 7; ++y)
		for(size_t x = 0; x < 6; ++x)
			BOOST_CHECK_EQUAL(test_image(x,y),( (x>=1 && x<3 && y >= 2 && y < 4) ? 2 : 0)); 

}


BOOST_FIXTURE_TEST_CASE(test_segsection_error_attribute, SectionTestRead)
{
	const char *sestsection_error_x = 
		"<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
		"<point y=\"2\" x=\"1a\"/>"
		"</section></test>\n";


	BOOST_CHECK_THROW(init(sestsection_error_x), runtime_error); 

	const char *sestsection_error_y = 
		"<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
		"<point y=\"2a\" x=\"1\"/>"
		"</section></test>\n";

	BOOST_CHECK_THROW(init(sestsection_error_y), runtime_error); 

}


BOOST_AUTO_TEST_CASE(test_segstart_error_attribute)
{
	const char *sestsection_error_r = 
		"<?xml version=\"1.0\"?>\n<test>"
		"<star y=\"118\" x=\"109\" r=\"21a\">"
		"<point y=\"20\" x=\"10\"/>"
		"<point y=\"10\" x=\"20\"/>"
		"<point y=\"4\" x=\"0\"/>"
		"</star>"
		"</test>"; 


	xmlpp::DomParser parser;
	parser.parse_memory(sestsection_error_r);
	const xmlpp::Document *document = parser.get_document();
	const xmlpp::Element *root = document->get_root_node ();
	const xmlpp::Node::NodeList nodes = root->get_children();
	BOOST_CHECK_EQUAL(nodes.size(),1u);
	BOOST_CHECK_THROW(CSegStar(**nodes.begin()), runtime_error); 
}


/*
input_set.save_images(); 
*/

BOOST_AUTO_TEST_CASE( test_segset_write )
{
	CSegSet segset;
	CSegStar star1(CSegPoint2D(109, 118), 21, CSegPoint2D(10, 20),
		       CSegPoint2D(20, 10), CSegPoint2D(0, 4));
	CSegStar star2(CSegPoint2D(109, 118), 22, CSegPoint2D(10, 20),
		       CSegPoint2D(20, 10), CSegPoint2D(1, 4));
	segset.add_frame(CSegFrame("image.png", star1, CSegFrame::Sections()));
	segset.add_frame(CSegFrame("image2.png", star2, CSegFrame::Sections()));

	auto_ptr<xmlpp::Document> document(segset.write());

	const string xmldoc = document->write_to_string();
	const string testdoc(testset_init2);

	BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
	BOOST_CHECK_EQUAL(xmldoc, testdoc);
}

BOOST_FIXTURE_TEST_CASE( test_segset_shift_and_rename, SegSetReadFixture )
{
	init(testset_bboxtest);
	C2DFVector shift(-3.0f, -10.0f);
	CSegSet result = segset.shift_and_rename(0, shift, "moved");
	unique_ptr<xmlpp::Document> document(result.write());
	const string xmldoc = document->write_to_string();
	const string testdoc(testset_shift_and_rename);
	BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
	BOOST_CHECK_EQUAL(xmldoc, testdoc);
	if (xmldoc != testdoc) {
		for ( string::const_iterator x = xmldoc.begin(), t = testdoc.begin();
		      x != xmldoc.end() && t != testdoc.end(); ++x, ++t ) {
			if (*x != *t) {
				cvfail() << "'" << *x << "' vs '"<< *t << "'" << endl;
			}
		}
	}
}


void SegSetReadFixture::init(const char *data)
{
	xmlpp::DomParser parser;
	parser.parse_memory(data);
	const xmlpp::Document *document = parser.get_document();
	segset = CSegSet(*document);
}


void SectionTestRead::init(const char *init_str)
{
	xmlpp::DomParser parser;
	parser.parse_memory(init_str);
	const xmlpp::Document *document = parser.get_document();
	const xmlpp::Element *root = document->get_root_node ();
	const xmlpp::Node::NodeList nodes = root->get_children();
	BOOST_CHECK_EQUAL(nodes.size(),1u);
	section = CSegSection(**nodes.begin());

}

void SectionTestRead::check(const float *x_data, const float *y_data) const
{
	BOOST_CHECK_EQUAL(section.get_id(),"white");

	const CSegSection::Points& points = section.get_points();

	BOOST_REQUIRE(points.size()== 3);

	for(size_t i = 0; i < size; ++i) {
		BOOST_CHECK_EQUAL(points[i].x, x_data[i]);
		BOOST_CHECK_EQUAL(points[i].y, y_data[i]);
	}
}



const char *sestsection_for_draw = "<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
	"<point y=\"4\" x=\"1\"/>"
	"<point y=\"8\" x=\"1\"/>"
	"<point y=\"8\" x=\"7\"/>"
	"<point y=\"1\" x=\"7\"/>"
	"<point y=\"1\" x=\"4\"/>"
	"<point y=\"6\" x=\"4\"/>"
	"<point y=\"6\" x=\"3\"/>"
	"<point y=\"4\" x=\"3\"/>"
	"</section></test>\n";

const char *teststar_init  =
	"<?xml version=\"1.0\"?>\n<test>"
	"<star y=\"118\" x=\"109\" r=\"21\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
	"</star></test>\n";


const char *teststar_init2  =
	"<?xml version=\"1.0\"?>\n<test>"
	"<star y=\"118\" x=\"109\" r=\"21\">"
	"  <point y=\"20\" x=\"10\"/>"
	"  <point y=\"10\" x=\"20\"/>"
	"  <point y=\"4\" x=\"0\"/>"
	"</star></test>\n";

const char *testsection_init = "<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
	"</section></test>\n";

const char *testsection_init2 = "<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
	"some text"
	"</section></test>\n";



const char *testset_init = "<?xml version=\"1.0\"?>\n<workset>"
	"<description><RVpeak value=\"0\"/><LVpeak value=\"1\"/></description>"
	"<frame image=\"image.png\">"
	"<star y=\"118\" x=\"109\" r=\"21\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
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
	"<frame image=\"image2.png\">"
	"<star y=\"118\" x=\"109\" r=\"22\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
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
	"</workset>\n";

const char *testset_init2 = "<?xml version=\"1.0\"?>\n<workset>"
	"<description><RVpeak value=\"-1\"/><LVpeak value=\"-1\"/></description>"
	"<frame image=\"image.png\">"
	"<star y=\"118\" x=\"109\" r=\"21\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
	"</star>"
	"</frame>"
	"<frame image=\"image2.png\">"
	"<star y=\"118\" x=\"109\" r=\"22\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"1\"/>"
	"</star>"
	"</frame>"
	"</workset>\n";

const char *testset_init3 = "<?xml version=\"1.0\"?>\n<workset>"
	"<description><RVpeak value=\"-1\"/><LVpeak value=\"-1\"/></description>"
	" <frame image=\"image.png\">"
	"  <star y=\"118\" x=\"109\" r=\"21\">"
	"   <point y=\"20\" x=\"10\"/>"
	"   <point y=\"10\" x=\"20\"/>"
	"   <point y=\"4\" x=\"0\"/>"
	"  </star>"
	" </frame>"
	" <frame image=\"image2.png\">"
	"  <star y=\"118\" x=\"109\" r=\"22\">"
	"  <point y=\"20\" x=\"10\"/>"
	"  <point y=\"10\" x=\"20\"/>"
	"  <point y=\"4\" x=\"0\"/>"
	"  </star>"
	" </frame>"
	"</workset>\n";

const char *testset_bboxtest =
"<?xml version=\"1.0\"?>\n<workset>"
  "<description>"
	"<RVpeak value=\"-1\"/>"
	"<LVpeak value=\"-1\"/>"
  "</description>"
  "<frame image=\"data0000.png\">"
      "<star y=\"118\" x=\"109\" r=\"21\">"
	"<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
      "</star>"
    "<section color=\"white\">"
      "<point y=\"106.24\" x=\"91.520000\"/>"
      "<point y=\"97.6\" x=\"102.080000\"/>"
      "<point y=\"101.44\" x=\"111.360000\"/>"
      "<point y=\"106.56\" x=\"100.480000\"/>"
      "<point y=\"109.44\" x=\"94.400000\"/>"
    "</section>"
    "<section color=\"cyan\">"
      "<point y=\"109.44\" x=\"96.32\"/>"
      "<point y=\"124.16\" x=\"96.64\"/>"
      "<point y=\"115.52\" x=\"85.76\"/>"
      "<point y=\"105.28\" x=\"92.16\"/>"
    "</section>"
  "</frame>"
  "<frame image=\"data0001.png\">"
    "<section color=\"white\">"
      "<point y=\"102.4\" x=\"90.88\"/>"
      "<point y=\"97.60\" x=\"95.04\"/>"
      "<point y=\"94.72\" x=\"115.52\"/>"
      "<point y=\"101.44\" x=\"101.44\"/>"
      "<point y=\"104.96\" x=\"94.08\"/>"
    "</section>"
  "</frame></workset>";



const char *testset_shift_and_rename =
"<?xml version=\"1.0\"?>\n<workset>"
	"<description><RVpeak value=\"-1\"/><LVpeak value=\"-1\"/></description>"
  "<frame image=\"moved0000.png\">"
      "<star y=\"128\" x=\"112\" r=\"21\">"
        "<point y=\"20\" x=\"10\"/>"
	"<point y=\"10\" x=\"20\"/>"
	"<point y=\"4\" x=\"0\"/>"
      "</star>"
    "<section color=\"white\">"
      "<point y=\"116.24\" x=\"94.52\"/>"
      "<point y=\"107.6\" x=\"105.08\"/>"
      "<point y=\"111.44\" x=\"114.36\"/>"
      "<point y=\"116.56\" x=\"103.48\"/>"
      "<point y=\"119.44\" x=\"97.4\"/>"
    "</section>"
    "<section color=\"cyan\">"
      "<point y=\"119.44\" x=\"99.32\"/>"
      "<point y=\"134.16\" x=\"99.64\"/>"
      "<point y=\"125.52\" x=\"88.76\"/>"
      "<point y=\"115.28\" x=\"95.16\"/>"
    "</section>"
  "</frame>"
  "<frame image=\"moved0001.png\">"
    "<section color=\"white\">"
      "<point y=\"112.4\" x=\"93.88\"/>"
      "<point y=\"107.6\" x=\"98.04\"/>"
      "<point y=\"104.72\" x=\"118.52\"/>"
      "<point y=\"111.44\" x=\"104.44\"/>"
      "<point y=\"114.96\" x=\"97.08\"/>"
    "</section>"
  "</frame></workset>\n";
