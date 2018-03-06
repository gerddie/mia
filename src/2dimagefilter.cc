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

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;


const SProgramDescription g_general_help = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"},
       {pdi_short, "Run filters on a 2D image."},
       {
              pdi_description, "This program runs a series filters on a given input image. "
              "The filters are given as extra parameters on the command line and "
              "Ware run in the order in which they are given. To obtain a list of "
              "available filters you may run\n"
              "   'mia-plugin-help filter/2dimage'\n"
              "from the command line"
       },
       {
              pdi_example_descr, "Run a kmeans classification of 5 classes on input.png and then "
              "a binarization of the 4th class and store the result in result.png"
       },
       {pdi_example_code, "-i input.png -o result.png kmeans:c=5 binarize:min=4,max=4"}
};

int do_main( int argc, char *argv[] )
{
       string in_filename;
       string out_filename;
       const auto& filter_plugins = C2DFilterPluginHandler::instance();
       const auto& imageio = C2DImageIOPluginHandler::instance();
       CCmdOptionList options(g_general_help);
       options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered",
                             CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( out_filename, "out-file", 'o', "output image(s) that have been filtered",
                             CCmdOptionFlags::required_output, &imageio));

       if (options.parse(argc, argv, "filter", &filter_plugins) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
       cvdebug() << "supported filters: " << filter_plugins.get_plugin_names() << "\n";
       //CHistory::instance().append(argv[0], "unknown", options);
       const C2DImageFilterChain filter_chain(options.get_remaining());
       auto in_image_list = imageio.load(in_filename);

       if (!in_image_list || in_image_list->empty()) {
              throw create_exception<invalid_argument>( "No images found in ", in_filename);
       }

       transform(in_image_list->begin(), in_image_list->end(), in_image_list->begin(),
       [&filter_chain](const P2DImage & img) {
              return  filter_chain.run(img);
       });

       if ( !imageio.save(out_filename, *in_image_list) ) {
              throw create_exception<runtime_error>( "Unable to save result to ", out_filename);
       };

       return EXIT_SUCCESS;
}

MIA_MAIN(do_main);
