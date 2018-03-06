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

#include <sstream>
#include <mia/internal/main.hh>
#include <mia/core/fullstats.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/imageio.hh>


NS_MIA_USE;
using namespace std;


const SProgramDescription g_general_help = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"},
       {pdi_short, "Evaluate some stats over an image."},
       {
              pdi_description, "This progranm is used to evaluate some statistics of an image. "
              "Output is Mean, Variation, Median, Min and Max of the intensity values."
       },
       {pdi_example_descr, "Evaluate the statistics of image input.png"},
       {pdi_example_code, "-i input.png"}
};


class CStatsEvaluator : public TFilter<CFullStats>
{
public:
       template <typename T>
       CFullStats operator () (const T2DImage<T>& image) const
       {
              return CFullStats(image.begin(), image.end());
       }
};

int do_main( int argc, char *argv[] )
{
       string in_filename;
       CCmdOptionList options(g_general_help);
       options.set_stdout_is_result();
       options.add(make_opt( in_filename, "in-file", 'i', "input image",
                             CCmdOptionFlags::required_input, &C2DImageIOPluginHandler::instance()));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       // read image
       P2DImage image = load_image2d(in_filename);
       CFullStats stats = mia::filter(CStatsEvaluator(), *image);
       cout << stats << "\n";
       return EXIT_SUCCESS;
}

MIA_MAIN(do_main);
