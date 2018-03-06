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
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mia/3d/filter.hh>
#include <mia/3d/imageio.hh>
#include <mia/core.hh>

using namespace std;
using namespace mia;

size_t log10(size_t x)
{
       int result = 0;

       while (x > 0) {
              ++result;
              x /= 10;
       }

       return result;
}

const SProgramDescription g_description = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"},

       {pdi_short, "Filter a series of 3D images."},

       {
              pdi_description, "This program is used to filter and convert a consecutive numbered series "
              "gray of scale images. File names must follow the pattern 'dataXXXX.v' "
              "(X being digits), i.e. the numbering comes right before the dot. "
       },

       {
              pdi_example_descr, "Run a mean-least-varaiance filter on a series of images that follow the "
              "numbering pattern imageXXXX.hdr and store the output in images filteredXXXX.hdr"
       },

       {pdi_example_code, "-i image0000.hdr -o filtered -t hdr mlv:w=2"}
};


int do_main( int argc, char *argv[] )
{
       string in_filename;
       string out_filename;
       string out_type;
       const auto& filter_plugins = C3DFilterPluginHandler::instance();
       const auto& imageio = C3DImageIOPluginHandler::instance();
       CCmdOptionList options(g_description);
       options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOptionFlags::required_input, &imageio));
       options.add(make_opt( out_filename, "out-file", 'o', "output file name base, numbers are added accorfing to the input "
                             "file pattern, and the file  extension is added according to the 'type' option.",
                             CCmdOptionFlags::required_output, &imageio));
       options.add(make_opt( out_type, imageio.get_set(), "type", 't', "output file type", CCmdOptionFlags::required));

       if (options.parse(argc, argv, "filter", &filter_plugins) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       auto filter_chain = options.get_remaining();
       cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
       cvdebug() << "supported filters: " << filter_plugins.get_plugin_names() << "\n";

       if ( filter_chain.empty() )
              cvwarn() << "no filters given, just copy\n";

       if ( in_filename.empty() )
              throw runtime_error("'--in-file' ('i') option required");

       if ( out_filename.empty() )
              throw runtime_error("'--out-base' ('o') option required");

       bool use_src_format = out_type.empty();
       string out_suffix = imageio.get_preferred_suffix(out_type);
       //	CHistory::instance().append(argv[0], revision, options);
       list<P3DFilter> filters;

       for (auto i = filter_chain.begin();  i != filter_chain.end(); ++i) {
              cvdebug() << "Prepare filter " << *i << endl;
              auto filter = filter_plugins.produce(*i);

              if (!filter) {
                     stringstream error;
                     error << "Filter " << *i << " not found";
                     throw invalid_argument(error.str());
              }

              filters.push_back(filter);
       }

       size_t start_filenum = 0;
       size_t end_filenum  = 0;
       size_t format_width = 0;
       string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

       if (start_filenum >= end_filenum)
              throw invalid_argument(string("no files match pattern ") + src_basename);

       char new_line = cverb.show_debug() ? '\n' : '\r';
#ifndef WIN32
       time_t start_time = time(NULL);
#endif

       for (size_t i = start_filenum; i < end_filenum; ++i) {
              string src_name = create_filename(src_basename.c_str(), i);
              cvmsg() << new_line << "Filter: " << i << " out of " << "[" << start_filenum << "," << end_filenum << "]" ;
              auto in_image_list = imageio.load(src_name);

              if (in_image_list.get() && in_image_list->size()) {
                     if (use_src_format)
                            out_type = in_image_list->get_source_format();

                     auto filter_name = filter_chain.begin();

                     for (auto f = filters.begin(); f != filters.end(); ++f, ++filter_name) {
                            cvdebug() << "Run filter: " << *filter_name << "\n";

                            for (auto i = in_image_list->begin();
                                 i != in_image_list->end(); ++i) {
                                   *i = (*f)->filter(**i);
                            }
                     }

                     stringstream ss;
                     ss << out_filename << setw(format_width) << setfill('0') << i << "." << out_suffix;
                     cvdebug() << "Save to " << ss.str() << ", format = " << out_type << "\n";

                     if ( !imageio.save(ss.str(), *in_image_list) ) {
                            string not_save = ("unable to save result to ") + ss.str();
                            throw runtime_error(not_save);
                     }
              }

#ifndef WIN32

              if (cverb.shows(vstream::ml_message)) {
                     char esttime[30];
                     time_t est_end = (( end_filenum - start_filenum) * (time(NULL) - start_time)) /
                                      (i - start_filenum + 1) + start_time;
                     ctime_r(&est_end, esttime);
                     char *est = esttime;

                     while (*est != '\n' && *est != 0)
                            ++est;

                     if (*est == '\n')
                            *est = ' ';

                     cvmsg() << ", estimated finish at: " <<  esttime;
              }

#endif
       }

       cvmsg() << "\n";
       return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
