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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <string>
#include <stdexcept>
#include <dlfcn.h>

#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/imageio.hh>


using namespace std;
NS_MIA_USE

enum EBinops  {
       bin_or,
       bin_nor,
       bin_and,
       bin_nand,
       bin_xor,
       bin_nxor,
       bin_unknown
};

struct logical_xor : public binary_function<bool, bool, bool> {
       bool operator() (bool a, bool b) const
       {
              return (a ^ b);
       }
};

P2DImage binary_op(const C2DBitImage& a, const C2DBitImage& b, EBinops op)
{
       assert(a.get_size() == b.get_size());
       C2DBitImage *result = new C2DBitImage(a.get_size());

       switch (op) {
       case bin_or  :
              transform(a.begin(), a.end(), b.begin(), result->begin(), logical_or<bool>());
              break;

       case bin_nor :
              transform(a.begin(), a.end(), b.begin(), result->begin(), not2(logical_or<bool>()));
              break;

       case bin_and :
              transform(a.begin(), a.end(), b.begin(), result->begin(), logical_and<bool>());
              break;

       case bin_nand:
              transform(a.begin(), a.end(), b.begin(), result->begin(), not2(logical_and<bool>()));
              break;

       case bin_xor :
              transform(a.begin(), a.end(), b.begin(), result->begin(), logical_xor());
              break;

       case bin_nxor:
              transform(a.begin(), a.end(), b.begin(), result->begin(), not2(logical_xor()));
              break;

       default:
              throw invalid_argument("Unknown binary operation requested");
       }

       return P2DImage(result);
}


const TDictMap<EBinops>::Table g_binops_table[] = {
       {"or", bin_or, "logocal or"},
       {"nor", bin_nor, "logocal nor"},
       {"and", bin_and, "logocal and"},
       {"nand", bin_nand, "logocal nand"},
       {"xor", bin_xor, "logocal xor"},
       {"nxor", bin_nxor, "logocal nxor"},
       {NULL, bin_unknown, ""},
};

const TDictMap<EBinops> g_binops_dict(g_binops_table);

const SProgramDescription g_description = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"},
       {pdi_short, "Combine two binary images."},
       {pdi_description, "This program is used to combine two binary images by some kind of operation."},
       {
              pdi_example_descr, "Combine the binary images b1.png and b2.png by using the 'nor' operation "
              "and store the result in b1nor2.png."
       },
       {pdi_example_code, "-1 b1.png -2 b2.png -p nor -o b1nor2.png"}
};

int do_main( int argc, char *argv[] )
{
       string filename1;
       string filename2;
       string out_filename;
       EBinops op = bin_nor;
       const auto& imageio = C2DImageIOPluginHandler::instance();
       CCmdOptionList options(g_description);
       options.add(make_opt( filename1, "file1", '1', "input mask image 1", CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( filename2, "file2", '2', "input mask image 2", CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( out_filename, "out-file", 'o', "output mask image", CCmdOptionFlags::required_output, &imageio));
       options.add(make_opt( op, g_binops_dict, "operation", 'p', "Operation to be applied"));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       // read images
       P2DImage image1 = load_image2d(filename1);
       P2DImage image2 = load_image2d(filename2);

       try {
              const C2DBitImage& img1 = dynamic_cast<const C2DBitImage&>(*image1);
              const C2DBitImage& img2 = dynamic_cast<const C2DBitImage&>(*image2);
              P2DImage result = binary_op(img1, img2, op);

              if ( !save_image(out_filename, result) ) {
                     throw create_exception<runtime_error>( "cannot save result to ", out_filename);
              }
       } catch (bad_cast& x) {
              throw invalid_argument("Input images are not binary masks");
       }

       return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main);

