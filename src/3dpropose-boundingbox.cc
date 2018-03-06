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

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include <mia/3d/filter.hh>
#include <mia/3d/imageio.hh>
#include <mia/core.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"},
       {pdi_short, "Evaluate a bounding box for the image."},
       {
              pdi_description, "This program evaluates a box that contains all the pixels "
              "of a given mask image. If the input image is not a binary image, a pre-filter "
              "must be applied that converts the imput image into a mask."
       },
       {
              pdi_example_descr, "Evaluate a bounding box with boundary padding of 5 of a mask "
              "resulting from a binarization with a minimum value 10."
              "10 and a boundary padding of 5 from image image.v ."
       },
       {pdi_example_code, "-i image.v -f binarize:min=10 -p 5" }
};


inline void pad_down( unsigned& inout, unsigned pad)
{
       if (inout > pad)
              inout -= pad;
       else
              inout = 0;
};

inline void pad_up(unsigned& inout, unsigned pad, unsigned maximum)
{
       inout += pad;

       if (inout > maximum)
              inout = maximum;
}

int do_main( int argc, char *argv[] )
{
       string in_filename;
       P3DFilter prefilter;
       unsigned padding = 0;
       const auto& imageio = C3DImageIOPluginHandler::instance();
       CCmdOptionList options(g_description);
       options.set_group("File-IO");
       options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered",
                             CCmdOptionFlags::required_input, &imageio));
       options.set_group("Parameters");
       options.add(make_opt( prefilter, "", "filter", 'f', "Filter to be applied to the image before the "
                             "bounding box is evaluated. Must return a bit-valued image."));
       options.add(make_opt( padding, EParameterBounds::bf_min_closed, {0}, "padding", 'p',
                             "Padding of boundaries."));
       options.set_stdout_is_result();

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       auto image = load_image3d(in_filename);

       if (prefilter)
              image = prefilter->filter(*image);

       if (image->get_pixel_type() != it_bit) {
              if (!prefilter)
                     throw invalid_argument("The input image is not binary, and you didn't specify a prefilter "
                                            "that extracts a binary mask from the image");
              else
                     throw invalid_argument("The prefilter doesn't create a binary image.");
       }

       const C3DBitImage& mask = dynamic_cast<const C3DBitImage&>(*image);
       C3DBounds mask_start =  mask.get_size();
       C3DBounds mask_end = C3DBounds::_0;
       auto im = mask.begin_range(C3DBounds::_0, mask.get_size());
       auto em = mask.end_range(C3DBounds::_0, mask.get_size());

       while (im != em) {
              if (*im) {
                     if (mask_end.x <= im.pos().x) mask_end.x = im.pos().x + 1;

                     if (mask_end.y <= im.pos().y) mask_end.y = im.pos().y + 1;

                     if (mask_end.z <= im.pos().z) mask_end.z = im.pos().z + 1;

                     if (mask_start.x > im.pos().x) mask_start.x = im.pos().x;

                     if (mask_start.y > im.pos().y) mask_start.y = im.pos().y;

                     if (mask_start.z > im.pos().z) mask_start.z = im.pos().z;
              }

              ++im;
       }

       if (padding > 0) {
              pad_down(mask_start.x, padding);
              pad_down(mask_start.y, padding);
              pad_down(mask_start.z, padding);
              pad_up(mask_end.x, padding, mask.get_size().x);
              pad_up(mask_end.y, padding, mask.get_size().y);
              pad_up(mask_end.z, padding, mask.get_size().z);
       }

       std::cout << mask_start.x << " " << mask_start.y << " " << mask_start.z << " "
                 << mask_end.x << " " << mask_end.y << " " << mask_end.z << "\n";
       return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
