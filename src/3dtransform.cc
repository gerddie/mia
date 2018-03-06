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


#include <mia/core/cmdlineparser.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/imageio.hh>


NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
       {
              pdi_group,
              "Registration, Comparison, and Transformation of 3D images"
       },

       {
              pdi_short,
              "Transform a 3D image."
       },

       {
              pdi_description,
              "Transform a 3D image by applying a given 3D transformation."
       },

       {
              pdi_example_descr,
              "Transform an image input.v by the transfromation stored in trans.v "
              "by using nearest neighbour interpolation ans store the result in output.v"
       },

       {
              pdi_example_code,
              "-i input.v -t trans.v  -o output.v  -p bspline:d=0"
       }
};

int do_main(int argc, char **argv)
{
       CCmdOptionList options(g_description);
       string src_filename;
       string out_filename;
       string trans_filename;
       string interpolator_kernel;
       string interpolator_bc("mirror");
       const auto& imageio = C3DImageIOPluginHandler::instance();
       const auto& transio = C3DTransformationIOPluginHandler::instance();
       options.add(make_opt( src_filename, "in-file", 'i', "input image", CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( out_filename, "out-file", 'o', "transformed image", CCmdOptionFlags::required_output, &imageio));
       options.add(make_opt( trans_filename, "transformation", 't', "transformation file", CCmdOptionFlags::required_input, &transio));
       options.add(make_opt( interpolator_kernel, "interpolator", 'p',
                             "override the interpolator provided by the transformation"));
       options.add(make_opt( interpolator_bc, "boundary", 'b', "override the boundary conditions provided "
                             "by the transformation. This is only used if the interpolator is also overridden."));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       auto transformation = transio.load(trans_filename);
       auto source = imageio.load(src_filename);

       if (!source || source->size() < 1)
              throw create_exception<runtime_error>("No image found in '", src_filename, ",");

       if (!transformation) {
              throw create_exception<runtime_error>("No transformation found in '", trans_filename, "'");
       }

       if (!interpolator_kernel.empty()) {
              cvdebug() << "override the interpolator by '"
                        << interpolator_kernel << "' and boundary conditions '"
                        << interpolator_bc << "'\n";
              C3DInterpolatorFactory ipf(interpolator_kernel, interpolator_bc);
              transformation->set_interpolator_factory(ipf);
       }

       for (auto i = source->begin(); i != source->end(); ++i)
              *i = (*transformation)(**i);

       if ( !imageio.save(out_filename, *source) )
              throw create_exception<runtime_error>("unable to save result to '", out_filename, "'");

       return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
