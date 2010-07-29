/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/nonrigidregister.hh>
#include <gsl++/multimin.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/core/factorycmdlineoption.hh>

NS_MIA_USE;
using namespace std;
using namespace gsl;


const TDictMap<EMinimizers>::Table g_minimizer_table[] = {
	{"cg-fr", min_cg_fr},
	{"cg-pr", min_cg_pr},
	{"bfgs", min_bfgs},
	{"bfgs2", min_bfgs2},
	{"gd", min_gd},
	{NULL, min_undefined}
};


const char *g_general_help = 
	"This program runs the non-rigid registration of two images using certain"
	"cost measures and a given transformation model.\n"
	"Basic usage: \n"
	" mia-2dnonrigidreg [options] cost1 cost2 "; 

int do_main( int argc, const char *argv[] )
{
	string cost_function("ssd"); 
	string src_filename;
	string ref_filename;
	string out_filename;
	string trans_filename;
	string transform_type("spline");
	EMinimizers minimizer = min_bfgs2;

	cvdebug() << "auto transform_creator\n"; 
	auto transform_creator = C2DTransformCreatorHandler::instance().produce("spline"); 
	if (!transform_creator && transform_creator->get_init_string() != string("spline"))
		cverr() << "smthgs wrong\n"; 

	size_t mg_levels = 3;

	CCmdOptionList options;
	options.push_back(make_opt( src_filename, "in", 'i', "test image", "input", true));
	options.push_back(make_opt( ref_filename, "ref", 'r', "reference image", "input", true));
	options.push_back(make_opt( out_filename, "out", 'o', "registered output image", "output", true));
	options.push_back(make_opt( trans_filename, "trans", 't', "transformation", "transformation", false));
	options.push_back(make_opt( mg_levels, "levels", 'l', "multigrid levels", "levels", false));
	options.push_back(make_opt( minimizer, TDictMap<EMinimizers>(g_minimizer_table),
				    "optimizer", 'O', "Optimizer used for minimization", "optimizer", false));
	options.push_back(make_opt( transform_creator, "transForm", 'f', "transformation type", "transform", false));

	options.parse(argc, argv);
	
	auto cost_descrs = options.get_remaining(); 

	C2DFullCostList costs; 
	for (auto i = cost_descrs.begin(); i != cost_descrs.end(); ++i)
		costs.push(C2DFullCostPluginHandler::instance().produce(*i)); 

	P2DImage Model = load_image2d(src_filename);
	P2DImage Reference = load_image2d(ref_filename);

	C2DBounds GlobalSize = Model->get_size();
	if (GlobalSize != Reference->get_size())
		throw std::invalid_argument("Images have different size");

	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3));

	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, *ipfactory);

	P2DTransformation transform = nrr.run(Model, Reference, mg_levels);
	P2DImage result = (*transform)(*Model, *ipfactory);

	if (!trans_filename.empty()) {
		cvwarn() << "saving the transformation is not yet implemented";
	}

	return save_image2d(out_filename, result);
}


int main( int argc, const char *argv[] )
{
	try {
		return do_main(argc, argv);
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
