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



#include <mia/2d/io/xml.hh>
#include <mia/2d/imageio.hh>

#include <mia/internal/autotest.hh>

NS_BEGIN(myosegset2d)

using namespace myosegset2d;
using namespace mia;
using namespace std;


const char *testset_init = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workset>"
                           "<description><RVpeak value=\"2\"/>"
                           "<LVpeak value=\"1\"/><PreferedRef value=\"0\"/></description>"
                           " <frame image=\"image.bmp\">"
                           "  <star r=\"21\" x=\"109\" y=\"118\">"
                           "   <point x=\"10\" y=\"20\"/>"
                           "   <point x=\"20\" y=\"10\"/>"
                           "   <point x=\"0\" y=\"4\"/>"
                           "  </star>"
                           " </frame>"
                           " <frame image=\"image2.bmp\">"
                           "  <star r=\"22\" x=\"119\" y=\"117\">"
                           "  <point x=\"11\" y=\"21\"/>"
                           "  <point x=\"21\" y=\"11\"/>"
                           "  <point x=\"1\" y=\"5\"/>"
                           "  </star>"
                           " </frame>"
                           "</workset>\n";


BOOST_AUTO_TEST_CASE( test_read_simple )
{
       C2DUBImage img1(C2DBounds(2, 3));
       C2DUBImage img2(C2DBounds(2, 3));
       fill(img1.begin(), img1.end(), 0);
       fill(img2.begin(), img2.end(), 1);
       save_image("image.bmp", img1);
       save_image("image2.bmp", img2);
       ofstream testfile("segset.set");
       testfile << testset_init;
       testfile.close();
       CXMLSegSetWithImagesIOPlugin io;
       // create a pipe to write the data to and read from it,
       auto inset = io.load("segset.set");
       BOOST_REQUIRE(inset);
       BOOST_CHECK_EQUAL(inset->get_RV_peak(), 2);
       BOOST_CHECK_EQUAL(inset->get_LV_peak(), 1);
       BOOST_CHECK_EQUAL(inset->get_preferred_reference(), 0);
       auto frames = inset->get_frames();
       BOOST_CHECK_EQUAL(frames.size(), 2u);
//	unlink("segset.set");
       unlink("image.bmp");
       unlink("image2.bmp");
}


void check_segpoint_close(const CSegPoint2D& h, const CSegPoint2D& e)
{
       BOOST_CHECK_CLOSE(h.x, e.x, 0.01);
       BOOST_CHECK_CLOSE(h.y, e.y, 0.01);
}

void check_segstar_close(const CSegStar& have, const CSegStar& expect)
{
       check_segpoint_close(have.m_center, expect.m_center);

       for (int i = 0; i < 3; ++i)
              check_segpoint_close(have.m_directions[i], expect.m_directions[i]);

       BOOST_CHECK_CLOSE(have.m_radius, expect.m_radius, 0.01);
}

void check_section_close(const CSegSection& have, const CSegSection& expect)
{
       BOOST_CHECK_EQUAL(have.get_id(), expect.get_id());
       auto hp = have.get_points();
       auto ep = expect.get_points();
       BOOST_CHECK_EQUAL(hp.size(), ep.size());
       BOOST_REQUIRE(hp.size() == ep.size());

       for (unsigned i = 0; i < hp.size(); ++i) {
              check_segpoint_close(hp[i], ep[i]);
       }

       BOOST_CHECK_EQUAL(have.is_open(), expect.is_open());
}

BOOST_AUTO_TEST_CASE( test_write_simple )
{
       CSegSetWithImages set;
       set.set_RV_peak(2);
       set.set_LV_peak(1);
       set.set_preferred_reference(0);
       CSegStar in_star0(CSegPoint2D(1, 2), 3,
                         CSegPoint2D(2, 1), CSegPoint2D(0, 1), CSegPoint2D(1, 1));
       CSegFrame::Sections in_sections0;
       CSegSection::Points s11p =  {{3, 4}, {5, 6}, {1, 2}};
       CSegSection s11("s11", s11p, true);
       CSegSection s12("s12", s11p, false);
       in_sections0.push_back(s11);
       in_sections0.push_back(s12);
       CSegFrame in_frame0("image1.bmp", in_star0, in_sections0);
       in_frame0.set_quality(3.3);
       in_frame0.set_brightness(1.0);
       in_frame0.set_contrast(-1.0);
       P2DImage img1(new C2DUBImage(C2DBounds(2, 3)));
       set.add_frame(in_frame0, img1);
       CXMLSegSetWithImagesIOPlugin io;
       io.save("testsaveset.set", set);
       auto inset = io.load("testsaveset.set");
       BOOST_CHECK_EQUAL(inset->get_RV_peak(), 2);
       BOOST_CHECK_EQUAL(inset->get_LV_peak(), 1);
       BOOST_CHECK_EQUAL(inset->get_preferred_reference(), 0);
       auto inframes = inset->get_frames();
       BOOST_CHECK_EQUAL(inframes.size(), 1u);
       auto f0 = inframes[0];
       BOOST_CHECK_CLOSE(f0.get_quality(), in_frame0.get_quality(), 0.01);
       BOOST_CHECK_CLOSE(f0.get_brightness(), in_frame0.get_brightness(), 0.01);
       BOOST_CHECK_CLOSE(f0.get_contrast(), in_frame0.get_contrast(), 0.01);
       auto star0 = f0.get_star();
       check_segstar_close(star0, in_star0);
       auto sections0 = f0.get_sections();
       BOOST_CHECK_EQUAL(sections0.size(), 2u);

       for (int i = 0; i < 2; ++i) {
              check_section_close(sections0[i], in_sections0[i]);
       }

       unlink("testsaveset.set");
       unlink("image1.bmp");
}

NS_END
