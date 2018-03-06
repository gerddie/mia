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

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/internal/main.hh>
#include <mia/2d/segsetwithimages.hh>
#include <ostream>
#include <fstream>

using namespace mia;
using namespace std;


const SProgramDescription g_description = {
       {pdi_group, "Tools for Myocardial Perfusion Analysis"},
       {pdi_short, "Evaluate the dice index between segmentations."},
       {
              pdi_description,	"This program is used to evaluate the per-frame dice index of "
              "segmented regions of an image with respect to the segmentation of a reference frame "
              "from the same series."
       },
       {
              pdi_example_descr,	"Evaluate the dice index of segmentation set segment.set with reference 30 and "
              "skipping the first two frames."
       },
       {pdi_example_code,	"-i segment.set -r 30 -k 2"}
};

float dice_value(const C2DUBImage& mask1, const C2DUBImage& mask2)
{
       assert (mask1.get_size() == mask2.get_size());
       int schnitt = 0;
       int sum = 0;

       for (auto m1 = mask1.begin(), m2 = mask2.begin();
            m1 != mask1.end(); ++m1, ++m2) {
              if (*m1)
                     ++sum;

              if (*m2)
                     ++sum;

              if (*m1 && *m2)
                     ++schnitt;
       }

       return sum ? (2.0f * schnitt) / sum : 1.0f;
}

int do_main( int argc, char *argv[] )
{
       string org_filename;
       int skip = 2;
       size_t reference = 20;
       CCmdOptionList options(g_description);
       options.add(make_opt( org_filename, "input", 'i', "original segmentation set", CCmdOptionFlags::required_input));
       options.add(make_opt( skip, "skip", 'k', "images to skip atthe bgin of the series"));
       options.add(make_opt( reference, "reference", 'r', "reference image"));
       options.set_stdout_is_result();

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       CSegSetWithImages original(org_filename, true);
       size_t real_skip = skip;

       if (skip < 0) {
              // if RV peak is given in the segmentation file, use it, otherwiese use
              // absolue value of skip
              int sk = original.get_RV_peak();
              real_skip = (sk < 0 ) ? -skip : sk;
       }

       auto original_frames = original.get_frames();

       if (reference < real_skip || reference >= original_frames.size())
              throw create_exception<invalid_argument>( "reference frame must be larger then skip=",
                            skip, " and smaller then the length of the series ", original_frames.size());

       C2DUBImage reference_mask = original_frames[reference].get_section_masks(1);

       for (size_t i = real_skip; i < original_frames.size(); ++i)  {
              auto current = original_frames[i].get_section_masks(1);
              cout << dice_value(current, reference_mask) << '\n';
       }

       return EXIT_SUCCESS;
}

MIA_MAIN(do_main);
