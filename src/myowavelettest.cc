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

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <mia/core/gsl_wavelet.hh>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/slopestatistics.hh>
#include <mia/core/waveletslopeclassifier.hh>

using namespace std;

NS_MIA_USE;
using namespace gsl;

const SProgramDescription g_general_help = {
       {pdi_group, "Miscellaneous programs"},
       {pdi_short, "Wavelet based curve analysis"},
       {
              pdi_description, "This program runs the wavelet based slope-statistics on the mixing matrix "
              "of a ICA perfusion series."
       }
};

const TDictMap<EWaveletType>::Table wavelet_dict[] = {
       {"haar", wt_haar, "HAAR wavelet"},
       {"c-haar", wt_haar_centered, "HAAR wavelet (centered)"},
       {"daubechies", wt_daubechies, "Daubechies wavelet"},
       {"c-daubechies", wt_daubechies_centered, "Daubechies wavelet (centered)"},
       {"bspline", wt_bspline, "B-Spline wavelet"},
       {"c-bspline", wt_bspline_centered, "B-Spline wavelet (centered)"},
       {NULL, wt_none, ""}
};

const TDictMap<EWaveletType> g_wavelet_dict(wavelet_dict);


int do_main( int argc, char *argv[] )
{
       CCmdOptionList options(g_general_help);
       string in_filename;
       string out_filename;
       options.set_group("File-IO");
       options.add(make_opt( in_filename, "in-file", 'i', "input data set",
                             CCmdOptionFlags::required_input));
       options.set_stdout_is_result();

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       // read first line to estimate number of columns
       ifstream input(in_filename.c_str());
       char s[1024];
       input.getline(s, 1024);
       istringstream first_row(s);
       cvdebug() << "First row is '" << s << "'\n";
       vector<float> vx;

       while (first_row.good()) {
              float x;
              first_row >> x;

              if (!first_row.fail())
                     vx.push_back(x);
       }

       if ( vx.empty() )
              throw runtime_error("Empty input file");

       CWaveletSlopeClassifier::Columns table(vx.size());

       for (size_t i = 0; i < vx.size(); ++i)
              table[i].push_back(vx[i]);

       cvdebug() << "Read " << vx.size() << " columns\n";

       // read the reminder of the table
       while (input.good()) {
              for (size_t i = 0; i < vx.size() && input.good(); ++i) {
                     float x;
                     input >> x;

                     if (input.good())
                            table[i].push_back(x);
              }
       }

       size_t nrows = table[0].size();
       cvdebug() << "got " << nrows << "  rows\n";

       for (size_t i = 1; i < vx.size(); ++i)
              if (table[i].size() != nrows) {
                     throw create_exception<runtime_error>( "bogus input table, expect ", nrows, ", but column "
                                                            , i, " has ", table[i].size(), " rows");
              }

       CWaveletSlopeClassifier classifier(table, false, -1);

       switch (classifier.result()) {
       case CWaveletSlopeClassifier::wsc_fail:
              throw invalid_argument("The input data could not be classified into LV, RV and additional components");

       case CWaveletSlopeClassifier::wsc_no_movement:
              cout << "The input data set doesn't contain significant motion\n";
              cout << "RV: idx = " << classifier.get_RV_idx() << " with peak at " << classifier.get_RV_peak() << "\n";
              cout << "LV: idx = " << classifier.get_LV_idx() << " with peak at " << classifier.get_LV_peak() << "\n";
              cout << "Baseline: " << classifier.get_baseline_idx() << "\n";
              cout << "Perfusion:" << classifier.get_perfusion_idx() << " (uncertain) \n";
              break;

       case CWaveletSlopeClassifier::wsc_low_movement:
              cout << "The input data set contains some motion\n";
              cout << "RV: idx = " << classifier.get_RV_idx() << " with peak at " << classifier.get_RV_peak() << "\n";
              cout << "LV: idx = " << classifier.get_LV_idx() << " with peak at " << classifier.get_LV_peak() << "\n";
              cout << "Baseline: " << classifier.get_baseline_idx() << "\n";
              cout << "Motion:   " << classifier.get_movement_idx() << " with energy " << classifier.get_movement_indicator() << "\n";
              cout << "Perfusion:" << classifier.get_perfusion_idx() << " (uncertain) \n";
              break;

       case CWaveletSlopeClassifier::wsc_normal:
              cout << "The input data set contains motion\n";
              cout << "RV: idx = " << classifier.get_RV_idx() << " with peak at " << classifier.get_RV_peak() << "\n";
              cout << "LV: idx = " << classifier.get_LV_idx() << " with peak at " << classifier.get_LV_peak() << "\n";
              cout << "Baseline: " << classifier.get_baseline_idx() << "\n";
              cout << "Motion:   " << classifier.get_movement_idx() << " with energy " << classifier.get_movement_indicator() << "\n";
              cout << "Perfusion:" << classifier.get_perfusion_idx() << " (uncertain) \n";
              break;

       default:
              cout << "hu?\n";
       }

       return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main);


