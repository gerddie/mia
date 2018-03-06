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

#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>
#include <mia/3d/cost.hh>
#include <mia/3d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescription g_description = {
       {pdi_group, "Registration, Comparison, and Transformation of 3D images"},

       {pdi_short, "Evaluate the registration force between two 3D images."},

       {
              pdi_description, "This program is used to create an image comprising the pixel-wise norm "
              "of the ggradient of a given cost function."
       },

       {
              pdi_example_descr,
              "Evaluate the weigtes sum of SSD between src.v and ref.v and store the result in forcenorm.v."
       },

       {pdi_example_code, "-i src.v -r ref.v -c ssd -o forcenorm.v"}
};


struct FVectorNorm {

       FVectorNorm(): m_max_norm(0.0f)
       {
       }

       float operator ()(const C3DFVector& v)
       {
              float n = v.norm();

              if (m_max_norm < n)
                     m_max_norm = n;

              return n;
       }
       float get_max()const
       {
              return m_max_norm;
       }
private:
       float m_max_norm;
};

int do_main(int argc, char **argv)
{
       CCmdOptionList options(g_description);
       string src_filename;
       string out_filename;
       string ref_filename;
       P3DImageCost cost;
       const auto& imageio = C3DImageIOPluginHandler::instance();
       options.add(make_opt( src_filename, "src-file", 'i', "input image", CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( ref_filename, "ref-file", 'r', "output force norm image", CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( out_filename, "out-file", 'o', "reference image", CCmdOptionFlags::required_output, &imageio));
       options.add(make_opt( cost, "ssd", "cost", 'c', "cost function to use"));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       typedef C3DImageIOPluginHandler::Instance::PData PImageVector;
       PImageVector source    = imageio.load(src_filename);
       PImageVector ref    = imageio.load(ref_filename);

       if (!source || source->empty()) {
              throw invalid_argument(string("no image found in ") + src_filename);
       }

       if (!ref || ref->empty())
              throw invalid_argument(string("no image found in ") + ref_filename);

       C3DFVectorfield forcefield((*source->begin())->get_size());
       cost->set_reference(**ref->begin());
       cvmsg() << "Cost = " << cost->evaluate_force(**source->begin(), forcefield) << "\n";
       C3DFImage *presult = new C3DFImage(forcefield.get_size());
       P3DImage result(presult);
       FVectorNorm vnorm;
       transform(forcefield.begin(), forcefield.end(), presult->begin(), vnorm);
       cvmsg() << "max norm: " << vnorm.get_max() << "\n";
       C3DImageIOPluginHandler::Instance::Data vout;
       vout.push_back(result);
       return imageio.save(out_filename, vout) ? EXIT_SUCCESS : EXIT_FAILURE;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main);
