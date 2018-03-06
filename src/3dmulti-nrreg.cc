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


/*
  This program implements 3D gray scale image registration.
  Depending on the available plug-ins it can accomodate various models and cost-functions.
  The input images must be of the same dimensions and gray scale (whatever bit-depth).
*/

#include <mia/core.hh>
#include <mia/3d.hh>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/minmax_element.hpp>
#include <mia/3d/multireg.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescription g_description = {
       {pdi_group, "3D image registration"},
       {pdi_short, "Non-linear registration of  3D images."},
       {
              pdi_description, "This program implements 3D gray scale image registration "
              "by optimizing a dense vector field that defines a transformation for each pixel. "
              "The input images must be of the same size and dimensions. "
              "The registration can be achieved by optimizing a combined cost function. "
       }
};

// set op the command line parameters and run the registration
int do_main(int argc, char **argv)
{
       CCmdOptionList options(g_description);
       string out_filename;
       string regmodel("navier");
       string timestep("fluid");
       int start_size = 16;
       EInterpolation interpolator = ip_bspline3;
       int max_iter = 200;
       float epsilon = 0.01;
       options.add(make_opt( out_filename, "out-file", 'o', "output vector field",
                             CCmdOptionFlags::required_output));
       options.add(make_opt( regmodel, "regmodel", 'm', "registration model"));
       options.add(make_opt( timestep, "timestep", 't', "time setp"));
       options.add(make_opt( start_size, "mgsize", 's', "multigrid start size"));
       options.add(make_opt( max_iter, "max-iter", 'n', ",maximum number of iterations"));
       options.add(make_opt( interpolator, GInterpolatorTable, "interpolator", 'p',
                             "image interpolator"));
       options.add(make_opt( epsilon, "epsilon", 'e', "relative accuracy to stop registration "
                             "at a multi-grid level"));
       options.parse(argc, argv, true);

       if (out_filename.empty()) {
              cvfatal() << "No output filename given\n";
              return EXIT_FAILURE;
       }

       vector<const char *> cost_chain = options.get_remaining();

       if (cost_chain.empty()) {
              cvfatal() << "require cost functions given as extra parameters\n";
              return EXIT_FAILURE;
       }

       P3DRegModel model = C3DRegModelPluginHandler::instance().produce(regmodel.c_str());
       P3DRegTimeStep time_step = C3DRegTimeStepPluginHandler::instance().produce(timestep.c_str());

       if (!model || !time_step)
              return EXIT_FAILURE;

       std::shared_ptr<C3DInterpolatorFactory > ipf(create_3dinterpolation_factory(interpolator));

       if (!ipf)
              throw invalid_argument("unknown interpolator requested");

       C3DImageFatCostList cost_list;

       for (vector<const char *>::const_iterator i = cost_chain.begin(); i != cost_chain.end(); ++i) {
              P3DImageFatCost c = C3DFatImageCostPluginHandler::instance().produce(*i);

              if (c)
                     cost_list.push_back(c);
       }

       if (cost_list.empty()) {
              cerr << "Could not create a single cost function\n";
              return EXIT_FAILURE;
       }

       C3DMultiImageRegister reg(start_size, max_iter, *model,  *time_step, epsilon);

       if (!reg(cost_list, ipf)->save(out_filename, "")) {
              cerr << "Unable to save result vector field to " << out_filename << "\n";
              return EXIT_FAILURE;
       }

       return EXIT_SUCCESS;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)
