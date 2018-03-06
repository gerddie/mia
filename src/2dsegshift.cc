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

#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ostream>
#include <cmath>
#include <list>
#include <cassert>
#include <boost/filesystem.hpp>

#include <mia/core.hh>
#include <mia/2d/segset.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/internal/main.hh>

using namespace std;
using namespace mia;
namespace bfs = boost::filesystem;


const SProgramDescription g_description = {
       {pdi_group, "Tools for Myocardial Perfusion Analysis"},
       {pdi_short, "Translate a segmentation by the given amount."},
       {
              pdi_description, "This program move the segmentation(s) of an image series by using a shift "
              "that is equal for all slices. The program also may remove images from the "
              "begin of the series. The program can be used to correct the segmentation "
              "of the images if the images where cropped."
       },
       {
              pdi_example_descr, "Shift the segmentations of a set segement.set by -20 pixels in "
              "horizontal direction and -30 pixels in vertical direction (as if the images where "
              "cropped starting at (20,30) and store it in translate.set. The shiftes files are "
              "named translatedXXXX.png"
       },
       {pdi_example_code, "-i segment.set -o translate.set -g translated -S \"<20,30>\""}
};


int do_main(int argc, char *argv[])
{
       string src_filename;
       string out_filename;
       string shift_filename("crop");
       size_t skip = 2;
       C2DFVector shift;
       const auto& imageio = C2DImageIOPluginHandler::instance();
       CCmdOptionList options(g_description);
       options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set",
                             CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( out_filename, "out-file", 'o', "input segmentation set",
                             CCmdOptionFlags::required_output, &imageio));
       options.add(make_opt( shift_filename, "image-file", 'g', "output image filename base"));
       options.add(make_opt(shift, "shift", 'S', "shift of segmentation"));
       options.add(make_opt(skip, "skip", 'k', "skip frames at the beginning"));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       CSegSet src_segset(src_filename);
       cvinfo() << "shift by " << shift << ", skip " << skip << " and rename to base " << shift_filename << "\n";
       CSegSet shifted = src_segset.shift_and_rename(skip, shift, shift_filename);
       ofstream outfile(out_filename.c_str(), ios_base::out );

       if (outfile.good())
              outfile << shifted.write().write_to_string();

       return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}

MIA_MAIN(do_main);
