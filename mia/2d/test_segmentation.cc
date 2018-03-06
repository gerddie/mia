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

#include <numeric>
#include <mia/2d/segsetwithimages.hh>
#include <mia/2d/boundingbox.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/imageio.hh>
#include <mia/core/xmlinterface.hh>

namespace bfs = boost::filesystem;


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;

const char *testpoint_init  =
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test><point x=\"10\" y=\"20\"/></test>\n";

BOOST_AUTO_TEST_CASE(segpoint_read)
{
       CXMLDocument document(testpoint_init);
       auto root = document.get_root_node ();
       auto nodes = root->get_children("point");
       BOOST_CHECK_EQUAL(nodes.size(), 1u);

       for (auto i = nodes.begin(); i != nodes.end(); ++i) {
              CSegPoint2D p(**i);
              BOOST_CHECK_EQUAL(p.x, 10);
              BOOST_CHECK_EQUAL(p.y, 20);
       }
}

BOOST_AUTO_TEST_CASE(segpoint_write)
{
       CXMLDocument document;
       auto nodeRoot = document.create_root_node("test");
       CSegPoint2D point(10, 20);
       point.write(*nodeRoot);
       const string xmldoc = document.write_to_string("UTF-8", false);
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
extern const char *testset_version_2;

struct SegStarFixture {
       void init(const char *init_str);
       void check(const C2DFVector& center, float radius,
                  const float *x_directions, const float *y_directions);

       CSegStar star;
};


void SegStarFixture::init(const char *init_str)
{
       CXMLDocument document(init_str);
       auto root = document.get_root_node ();
       auto nodes = root->get_children("star");
       BOOST_CHECK_EQUAL(nodes.size(), 1u);
       star = CSegStar(**nodes.begin());
}

void SegStarFixture::check(const C2DFVector& center, float radius,
                           const float *x_directions, const float *y_directions)
{
       BOOST_CHECK_EQUAL(star.m_center.x, center.x);
       BOOST_CHECK_EQUAL(star.m_center.y, center.y);
       BOOST_CHECK_EQUAL(star.m_radius, radius);

       for (size_t i = 0; i < 3; ++i) {
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

BOOST_FIXTURE_TEST_CASE(test_segstart_no_radius, SegStarFixture)
{
       const char *sestsection_error_r =
              "<?xml version=\"1.0\"?>\n<test>"
              "<star  x=\"109\" y=\"118\">"
              "<point x=\"10\" y=\"20\"/>"
              "<point x=\"20\" y=\"10\"/>"
              "<point x=\"0\" y=\"4\"/>"
              "</star>"
              "</test>";
       CXMLDocument doc;
       BOOST_CHECK_THROW(init(sestsection_error_r), runtime_error);
}

BOOST_FIXTURE_TEST_CASE(test_segstart_radius_no_number, SegStarFixture)
{
       const char *sestsection_error_r =
              "<?xml version=\"1.0\"?>\n<test>"
              "<star  x=\"109\" y=\"118\" r=\"12a\">"
              "<point x=\"10\" y=\"20\"/>"
              "<point x=\"20\" y=\"10\"/>"
              "<point x=\"0\" y=\"4\"/>"
              "</star>"
              "</test>";
       CXMLDocument doc;
       BOOST_CHECK_THROW(init(sestsection_error_r), runtime_error);
}

BOOST_AUTO_TEST_CASE(segstar_write)
{
       CSegStar star(CSegPoint2D(109, 118), 21,
                     CSegPoint2D(10, 20),
                     CSegPoint2D(20, 10),
                     CSegPoint2D(0, 4));
       CXMLDocument document;
       auto nodeRoot = document.create_root_node("test");
       star.write(*nodeRoot);
       const string xmldoc = document.write_to_string("UTF-8", false);
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
       BOOST_CHECK_EQUAL(box.get_size(), C2DFVector(20, 16));
       BOOST_CHECK_EQUAL(box.get_begin(), C2DFVector(0, 4));
       BOOST_CHECK_EQUAL(box.get_end(), C2DFVector(20, 20));
}


BOOST_AUTO_TEST_CASE(segment_section_write)
{
       CSegSection::Points points;

       for (size_t i = 0; i  < size; ++i)
              points.push_back(CSegPoint2D(x_init[i], y_init[i]));

       CSegSection section("white", points, 1);
       CXMLDocument document;
       auto nodeRoot = document.create_root_node("test");
       section.write(*nodeRoot, 1);
       const string xmldoc = document.write_to_string("UTF-8", false);
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
       BOOST_CHECK_EQUAL(box.get_begin(), C2DFVector(85.76, 94.72));
       BOOST_CHECK_EQUAL(box.get_end(), C2DFVector(115.52, 124.16));
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
       C2DUBImage test_image(C2DBounds(10, 10));
       // sanity test, is the area correct?
       section.draw(test_image, 1);
       BOOST_CHECK_EQUAL(accumulate(test_image.begin(), test_image.end(), 0.0), 31);
}

BOOST_FIXTURE_TEST_CASE(test_segsection_draw2, SectionTestRead)
{
       const char *sestsection_for_draw2 =
              "<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
              "<point x=\"1\" y=\"2\"/>"
              "<point x=\"1\" y=\"4\"/>"
              "<point x=\"3\" y=\"4\"/>"
              "<point x=\"3\" y=\"2\"/>"
              "</section></test>\n";
       init(sestsection_for_draw2);
       C2DUBImage test_image(C2DBounds(6, 7));
       // sanity test, is the area correct?
       section.draw(test_image, 2);
       BOOST_CHECK_EQUAL(accumulate(test_image.begin(), test_image.end(), 0.0), 8);

       for (size_t y = 0; y < 7; ++y)
              for (size_t x = 0; x < 6; ++x)
                     BOOST_CHECK_EQUAL(test_image(x, y), ( (x >= 1 && x < 3 && y >= 2 && y < 4) ? 2 : 0));
}


BOOST_FIXTURE_TEST_CASE(test_segsection_error_attribute, SectionTestRead)
{
       const char *sestsection_error_x =
              "<?xml version=\"1.0\"?>\n<test><section color=\"white\">"
              "<point x=\"1 y=\"2\"a\"/>"
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
              "<star  r=\"21\"x=\"109\" y=\"118\">"
              "<point x=\"10\" y=\"20\"/>"
              "<point x=\"20\" y=\"10\"/>"
              "<point x=\"0\" y=\"4\"/>"
              "</star>"
              "</test>";
       CXMLDocument doc;
       BOOST_CHECK(!doc.read_from_string(sestsection_error_r));
}





/*
input_set.save_images();
*/

BOOST_AUTO_TEST_CASE( test_segset_write_version1 )
{
       CSegSet segset(1);
       CSegStar star1(CSegPoint2D(109, 118), 21, CSegPoint2D(10, 20),
                      CSegPoint2D(20, 10), CSegPoint2D(0, 4));
       CSegStar star2(CSegPoint2D(109, 118), 22, CSegPoint2D(10, 20),
                      CSegPoint2D(20, 10), CSegPoint2D(1, 4));
       segset.add_frame(CSegFrame("image.png", star1, CSegFrame::Sections()));
       segset.add_frame(CSegFrame("image2.png", star2, CSegFrame::Sections()));
       CXMLDocument document(segset.write());
       const string xmldoc = document.write_to_string("UTF-8", false);
       const string testdoc(testset_init2);
       BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
       BOOST_CHECK_EQUAL(xmldoc, testdoc);
}

BOOST_FIXTURE_TEST_CASE( test_segset_shift_and_rename, SegSetReadFixture )
{
       init(testset_bboxtest);
       C2DFVector shift(-3.0f, -10.0f);
       CSegSet result = segset.shift_and_rename(0, shift, "moved");
       CXMLDocument document(result.write());
       const string xmldoc = document.write_to_string("UTF-8", false);
       const string testdoc(testset_shift_and_rename);
       BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
       BOOST_CHECK_EQUAL(xmldoc, testdoc);

       if (xmldoc != testdoc) {
              for ( string::const_iterator x = xmldoc.begin(), t = testdoc.begin();
                    x != xmldoc.end() && t != testdoc.end(); ++x, ++t ) {
                     if (*x != *t) {
                            cvfail() << "'" << *x << "' vs '" << *t << "'" << endl;
                     }
              }
       }
}

BOOST_FIXTURE_TEST_CASE( test_segset_version_2_read_write, SegSetReadFixture )
{
       init(testset_version_2);
       const auto& frames = segset.get_frames();
       BOOST_CHECK_EQUAL(frames.size(), 1u);
       BOOST_CHECK_EQUAL(segset.get_LV_peak(), 2);
       BOOST_CHECK_EQUAL(segset.get_RV_peak(), 1);
       BOOST_CHECK_EQUAL(segset.get_preferred_reference(), 0);
       const auto& frame = frames[0];
       BOOST_CHECK_EQUAL(frame.get_imagename(), "moved0000.png");
       BOOST_CHECK_EQUAL(frame.get_star().m_radius, 21);
       BOOST_CHECK_EQUAL(frame.get_star().m_center.y, 128);
       BOOST_CHECK_EQUAL(frame.get_star().m_center.x, 112);
       BOOST_CHECK_EQUAL(frame.get_quality(), 4);
       BOOST_CHECK_EQUAL(frame.get_brightness(), 0.625);
       BOOST_CHECK_EQUAL(frame.get_contrast(), 1.5);
       const auto& sections = frame.get_sections();
       BOOST_CHECK_EQUAL(sections.size(), 2u);
       const auto& sec1 = sections[0];
       BOOST_CHECK(sec1.is_open());
       BOOST_CHECK_EQUAL(sec1.get_points().size(), 3u);
       float test_x = 2.1;
       float test_y = 1.1;

       for (auto i = sec1.get_points().begin(); i != sec1.get_points().end(); ++i, test_x += 1.0, test_y += 1.0) {
              BOOST_CHECK_EQUAL(i->x, test_x);
              BOOST_CHECK_EQUAL(i->y, test_y);
       }

       const auto& sec2 = sections[1];
       BOOST_CHECK(!sec2.is_open());
       BOOST_CHECK_EQUAL(sec2.get_points().size(), 3u);
       test_x = 2.25;
       test_y = 1.2;

       for (auto i = sec2.get_points().begin(); i != sec2.get_points().end(); ++i, test_x += 1.0, test_y += 1.0) {
              BOOST_CHECK_EQUAL(i->x, test_x);
              BOOST_CHECK_EQUAL(i->y, test_y);
       }

       CXMLDocument document(segset.write());
       const string xmldoc = document.write_to_string("UTF-8", false);
       const string testdoc(testset_version_2);
       BOOST_CHECK_EQUAL(xmldoc.size(), testdoc.size());
       BOOST_CHECK_EQUAL(xmldoc, testdoc);
}


BOOST_FIXTURE_TEST_CASE( test_segset_version_2_draw, SegSetReadFixture )
{
       const char *testset_version_2_draw =
              "<?xml version=\"1.0\"?>\n<workset version=\"2\">"
              "<description><RVpeak value=\"1\"/><LVpeak value=\"2\"/><PreferedRef value=\"0\"/></description>"
              "<frame image=\"moved0000.png\" quality=\"4\" brightness=\"0.625\" contrast=\"1.5\">"
              "<star r=\"21\" x=\"112\" y=\"128\">"
              "<point x=\"10\" y=\"20\"/>"
              "<point x=\"20\" y=\"10\"/>"
              "<point x=\"0\" y=\"4\"/>"
              "</star>"
              "<section color=\"white\" open=\"false\">"
              "<point x=\"2\" y=\"1\"/>"
              "<point x=\"2\" y=\"9\"/>"
              "<point x=\"8\" y=\"9\"/>"
              "<point x=\"8\" y=\"1\"/>"
              "</section>"
              "<section color=\"blue\" open=\"false\">"
              "<point x=\"4\" y=\"3\"/>"
              "<point x=\"4\" y=\"7\"/>"
              "<point x=\"6\" y=\"7\"/>"
              "<point x=\"6\" y=\"3\"/>"
              "</section>"
              "</frame></workset>\n";
       init(testset_version_2_draw);
       const auto& frames = segset.get_frames();
       BOOST_REQUIRE(frames.size() > 0);
       C2DUBImage mask = frames[0].get_section_masks(C2DBounds(12, 13));
       vector<unsigned char> test_image = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
              0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
              0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
              0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
              0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
       };
       auto m = mask.begin();
       auto t = test_image.begin();

       for (int y = 0; y < 12; ++y)
              for (int x = 0; x < 12; ++x, ++m, ++t) {
                     if (*m != *t)
                            cvdebug() << "(" << x << ", " << y << ")=" << (int)*m << "!=" << (int)*t << "\n";

                     BOOST_CHECK_EQUAL(*m, *t);
              }
}

BOOST_FIXTURE_TEST_CASE( test_segset_version_2_draw_fail, SegSetReadFixture )
{
       const char *testset_version_2_draw =
              "<?xml version=\"1.0\"?>\n<workset version=\"2\">"
              "<description><RVpeak value=\"1\"/><LVpeak value=\"2\"/><PreferedRef value=\"0\"/></description>"
              "<frame image=\"moved0000.png\" quality=\"4\" brightness=\"0.625\" contrast=\"1.5\">"
              "<star r=\"21\" x=\"112\" y=\"128\">"
              "<point x=\"10\" y=\"20\"/>"
              "<point x=\"20\" y=\"10\"/>"
              "<point x=\"0\" y=\"4\"/>"
              "</star>"
              "<section color=\"white\" open=\"true\">"
              "<point x=\"2\" y=\"1\"/>"
              "<point x=\"2\" y=\"9\"/>"
              "<point x=\"8\" y=\"9\"/>"
              "<point x=\"8\" y=\"1\"/>"
              "</section>"
              "<section color=\"blue\" open=\"false\">"
              "<point x=\"4\" y=\"3\"/>"
              "<point x=\"4\" y=\"7\"/>"
              "<point x=\"6\" y=\"7\"/>"
              "<point x=\"6\" y=\"3\"/>"
              "</section>"
              "</frame></workset>\n";
       init(testset_version_2_draw);
       const auto& frames = segset.get_frames();
       // one section is not closed, and can, therefore, not be drawn properly
       BOOST_CHECK_THROW(frames[0].get_section_masks(C2DBounds(12, 13)), invalid_argument);
}

void SegSetReadFixture::init(const char *data)
{
       CXMLDocument document(data);
       segset = CSegSet(document);
}


void SectionTestRead::init(const char *init_str)
{
       CXMLDocument document(init_str);
       auto root = document.get_root_node ();
       auto nodes = root->get_all_children();
       BOOST_CHECK_EQUAL(nodes.size(), 1u);
       section = CSegSection(**nodes.begin(), 1);
}

void SectionTestRead::check(const float *x_data, const float *y_data) const
{
       BOOST_CHECK_EQUAL(section.get_id(), "white");
       const CSegSection::Points& points = section.get_points();
       BOOST_REQUIRE(points.size() == 3);

       for (size_t i = 0; i < size; ++i) {
              BOOST_CHECK_EQUAL(points[i].x, x_data[i]);
              BOOST_CHECK_EQUAL(points[i].y, y_data[i]);
       }
}



const char *sestsection_for_draw = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test><section color=\"white\">"
                                   "<point x=\"1\" y=\"4\"/>"
                                   "<point x=\"1\" y=\"8\"/>"
                                   "<point x=\"7\" y=\"8\"/>"
                                   "<point x=\"7\" y=\"1\"/>"
                                   "<point x=\"4\" y=\"1\"/>"
                                   "<point x=\"4\" y=\"6\"/>"
                                   "<point x=\"3\" y=\"6\"/>"
                                   "<point x=\"3\" y=\"4\"/>"
                                   "</section></test>\n";

const char *teststar_init  =
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test>"
       "<star r=\"21\" x=\"109\" y=\"118\">"
       "<point x=\"10\" y=\"20\"/>"
       "<point x=\"20\" y=\"10\"/>"
       "<point x=\"0\" y=\"4\"/>"
       "</star></test>\n";


const char *teststar_init2  =
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test>"
       "<star r=\"21\" x=\"109\" y=\"118\">"
       "  <point x=\"10\" y=\"20\"/>"
       "  <point x=\"20\" y=\"10\"/>"
       "  <point x=\"0\" y=\"4\"/>"
       "</star></test>\n";

const char *testsection_init = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test><section color=\"white\">"
                               "<point x=\"10\" y=\"20\"/>"
                               "<point x=\"20\" y=\"10\"/>"
                               "<point x=\"0\" y=\"4\"/>"
                               "</section></test>\n";

const char *testsection_init2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<test><section color=\"white\">"
                                "<point x=\"10\" y=\"20\"/>"
                                "<point x=\"20\" y=\"10\"/>"
                                "<point x=\"0\" y=\"4\"/>"
                                "some text"
                                "</section></test>\n";



const char *testset_init = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workset>"
                           "<description><RVpeak value=\"0\"/><LVpeak value=\"1\"/><PreferedRef value=\"1\"/></description>"
                           "<frame image=\"image.png\">"
                           "<star r=\"21\" x=\"109\" y=\"118\">"
                           "<point x=\"10\" y=\"20\"/>"
                           "<point x=\"20\" y=\"10\"/>"
                           "<point x=\"0\" y=\"4\"/>"
                           "</star>"
                           "<section color=\"white\">"
                           "<point x=\"10\" y=\"20\"/>"
                           "<point x=\"20\" y=\"10\"/>"
                           "<point x=\"0\" y=\"4\"/>"
                           "</section>"
                           "<section color=\"red\">"
                           "<point x=\"11\" y=\"21\"/>"
                           "<point x=\"21\" y=\"11\"/>"
                           "<point x=\"1\" y=\"5\"/>"
                           "</section>"
                           "</frame>"
                           "<frame image=\"image2.png\">"
                           "<star r=\"22\" x=\"109\" y=\"118\">"
                           "<point x=\"10\" y=\"20\"/>"
                           "<point x=\"20\" y=\"10\"/>"
                           "<point x=\"0\" y=\"4\"/>"
                           "</star>"
                           "<section color=\"white\">"
                           "<point x=\"10\" y=\"20\"/>"
                           "<point x=\"20\" y=\"10\"/>"
                           "<point x=\"0\" y=\"4\"/>"
                           "</section>"
                           "<section color=\"red\">"
                           "<point x=\"11\" y=\"21\"/>"
                           "<point x=\"21\" y=\"11\"/>"
                           "<point x=\"1\" y=\"5\"/>"
                           "</section>"
                           "</frame>"
                           "</workset>\n";

const char *testset_init2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workset>"
                            "<description><RVpeak value=\"-1\"/><LVpeak value=\"-1\"/><PreferedRef value=\"-1\"/></description>"
                            "<frame image=\"image.png\">"
                            "<star r=\"21\" x=\"109\" y=\"118\">"
                            "<point x=\"10\" y=\"20\"/>"
                            "<point x=\"20\" y=\"10\"/>"
                            "<point x=\"0\" y=\"4\"/>"
                            "</star>"
                            "</frame>"
                            "<frame image=\"image2.png\">"
                            "<star r=\"22\" x=\"109\" y=\"118\">"
                            "<point x=\"10\" y=\"20\"/>"
                            "<point x=\"20\" y=\"10\"/>"
                            "<point x=\"1\" y=\"4\"/>"
                            "</star>"
                            "</frame>"
                            "</workset>\n";

const char *testset_init3 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workset>"
                            "<description><RVpeak value=\"-1\"/><LVpeak value=\"-1\"/><PreferedRef value=\"-1\"/></description>"
                            " <frame image=\"image.png\">"
                            "  <star r=\"21\" x=\"109\" y=\"118\">"
                            "   <point x=\"10\" y=\"20\"/>"
                            "   <point x=\"20\" y=\"10\"/>"
                            "   <point x=\"0\" y=\"4\"/>"
                            "  </star>"
                            " </frame>"
                            " <frame image=\"image2.png\">"
                            "  <star r=\"22\" x=\"109\" y=\"118\">"
                            "  <point x=\"10\" y=\"20\"/>"
                            "  <point x=\"20\" y=\"10\"/>"
                            "  <point x=\"0\" y=\"4\"/>"
                            "  </star>"
                            " </frame>"
                            "</workset>\n";

const char *testset_bboxtest =
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workset>"
       "<description>"
       "<RVpeak value=\"2\"/>"
       "<LVpeak value=\"3\"/>"
       "<PreferedRef value=\"1\"/>"
       "</description>"
       "<frame image=\"data0000.png\">"
       "<star r=\"21\" x=\"109\" y=\"118\">"
       "<point x=\"10\" y=\"20\"/>"
       "<point x=\"20\" y=\"10\"/>"
       "<point x=\"0\" y=\"4\"/>"
       "</star>"
       "<section color=\"white\">"
       "<point x=\"91.520000\" y=\"106.24\"/>"
       "<point x=\"102.080000\" y=\"97.6\"/>"
       "<point x=\"111.360000\" y=\"101.44\"/>"
       "<point x=\"100.480000\" y=\"106.56\"/>"
       "<point x=\"94.400000\" y=\"109.44\"/>"
       "</section>"
       "<section color=\"cyan\">"
       "<point x=\"96.32\" y=\"109.44\"/>"
       "<point x=\"96.64\" y=\"124.16\"/>"
       "<point x=\"85.76\" y=\"115.52\"/>"
       "<point x=\"92.16\" y=\"105.28\"/>"
       "</section>"
       "</frame>"
       "<frame image=\"data0001.png\">"
       "<section color=\"white\">"
       "<point x=\"90.88\" y=\"102.4\"/>"
       "<point x=\"95.04\" y=\"97.60\"/>"
       "<point x=\"115.52\" y=\"94.72\"/>"
       "<point x=\"101.44\" y=\"101.44\"/>"
       "<point x=\"94.08\" y=\"104.96\"/>"
       "</section>"
       "</frame></workset>";



const char *testset_shift_and_rename =
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workset>"
       "<description><RVpeak value=\"2\"/><LVpeak value=\"3\"/><PreferedRef value=\"1\"/></description>"
       "<frame image=\"moved0000.png\">"
       "<star r=\"21\" x=\"112\" y=\"128\">"
       "<point x=\"10\" y=\"20\"/>"
       "<point x=\"20\" y=\"10\"/>"
       "<point x=\"0\" y=\"4\"/>"
       "</star>"
       "<section color=\"white\">"
       "<point x=\"94.52\" y=\"116.24\"/>"
       "<point x=\"105.08\" y=\"107.6\"/>"
       "<point x=\"114.36\" y=\"111.44\"/>"
       "<point x=\"103.48\" y=\"116.56\"/>"
       "<point x=\"97.4\" y=\"119.44\"/>"
       "</section>"
       "<section color=\"cyan\">"
       "<point x=\"99.32\" y=\"119.44\"/>"
       "<point x=\"99.64\" y=\"134.16\"/>"
       "<point x=\"88.76\" y=\"125.52\"/>"
       "<point x=\"95.16\" y=\"115.28\"/>"
       "</section>"
       "</frame>"
       "<frame image=\"moved0001.png\">"
       "<section color=\"white\">"
       "<point x=\"93.88\" y=\"112.4\"/>"
       "<point x=\"98.04\" y=\"107.6\"/>"
       "<point x=\"118.52\" y=\"104.72\"/>"
       "<point x=\"104.44\" y=\"111.44\"/>"
       "<point x=\"97.08\" y=\"114.96\"/>"
       "</section>"
       "</frame></workset>\n";


const char *testset_version_2 =
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workset version=\"2\">"
       "<description><RVpeak value=\"1\"/><LVpeak value=\"2\"/><PreferedRef value=\"0\"/></description>"
       "<frame brightness=\"0.625\" contrast=\"1.5\" image=\"moved0000.png\" quality=\"4\">"
       "<star r=\"21\" x=\"112\" y=\"128\">"
       "<point x=\"10\" y=\"20\"/>"
       "<point x=\"20\" y=\"10\"/>"
       "<point x=\"0\" y=\"4\"/>"
       "</star>"
       "<section color=\"white\" open=\"true\">"
       "<point x=\"2.1\" y=\"1.1\"/>"
       "<point x=\"3.1\" y=\"2.1\"/>"
       "<point x=\"4.1\" y=\"3.1\"/>"
       "</section>"
       "<section color=\"green\" open=\"false\">"
       "<point x=\"2.25\" y=\"1.2\"/>"
       "<point x=\"3.25\" y=\"2.2\"/>"
       "<point x=\"4.25\" y=\"3.2\"/>"
       "</section>"
       "</frame></workset>\n";



