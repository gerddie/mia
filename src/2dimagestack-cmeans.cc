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
#include <ostream>
#include <fstream>
#include <mia/core.hh>
#include <mia/core/cmeans.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/internal/main.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {

       {pdi_group, "Processing of series of 2D images in a 3D fashion (out-of-core)"},
       {pdi_short, "Calculate the c-means classification for a series of images."},


       {
              pdi_description, "This program first evaluates a sparse histogram of an input image "
              "series, then runs a c-means classification over the histogram and then writes the "
              "probability mapping for thr original intensity values"
       },

       {
              pdi_example_descr, "Run the program over images imageXXXX.png with the sparse histogram, "
              "threshold the lower 5% bins (if available), run cmeans with three classes on the non-zero "
              "pixels."
       },

       {pdi_example_code, "-i image0000.png -o cmeans,txt --histogram-tresh=5 --classes 3"}
};



int do_main( int argc, char *argv[] )
{
       string out_probmap;
       string in_filename;
       float histogram_thresh = 5;
       CMeans::PInitializer class_center_initializer;
       const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
       CCmdOptionList options(g_description);
       options.set_group("File-IO");
       options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered",
                             CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( out_probmap, "out-probmap", 'o', "Save probability map to this file",
                             CCmdOptionFlags::required_output));
       options.set_group("Parameters");
       options.add(make_opt( histogram_thresh, EParameterBounds::bf_closed_interval, {0, 50},
                             "histogram-thresh", 'T',
                             "Percent of the extrem parts of the histogram to be collapsed "
                             "into the respective last histogram bin."));
       options.add(make_opt( class_center_initializer, "kmeans:nc=3", "classes", 'C',
                             "C-means class initializer"));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       size_t start_filenum = 0;
       size_t end_filenum  = 0;
       size_t format_width = 0;
       string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

       if (start_filenum >= end_filenum)
              throw invalid_argument(string("no files match pattern ") + src_basename);

       CSparseHistogram histo;
       size_t n_pixels = 0;

       for (size_t i = start_filenum; i < end_filenum; ++i) {
              string src_name = create_filename(src_basename.c_str(), i);
              C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);
              cvmsg() << "Read:" << src_name << "\r";

              if (in_image_list.get() && in_image_list->size()) {
                     for (auto k = in_image_list->begin(); k != in_image_list->end(); ++k)
                            n_pixels += accumulate(histo, **k);
              }
       }

       auto chistogram = histo.get_compressed_histogram();
       size_t n_cut_off = static_cast<size_t>(floor(n_pixels / 100.0 * histogram_thresh));
       size_t ni = 0;
       auto ii = chistogram.begin();

       while ( ni < n_cut_off && ii != chistogram.end()) {
              ni += ii->second;
              ++ii;
       }

       size_t ne = 0;
       auto ie = chistogram.end() - 1;

       while ( ne < n_cut_off && ie !=  ii) {
              ne += ie->second;
              --ie;
       }

       CSparseHistogram::Compressed threshed_histo(ii, ie);
       CMeans::DVector class_centers;
       CMeans cmeans(0.00001, class_center_initializer);
       CMeans::SparseProbmap pv = cmeans.run(threshed_histo,  class_centers);
       pv.save(out_probmap);
       return EXIT_SUCCESS;
}

MIA_MAIN(do_main);


