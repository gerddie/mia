/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include <mia/2d/rigidregister.hh>
#include <gsl++/multimin.hh>

NS_MIA_USE; 
using namespace std;
using namespace gsl; 


const TDictMap<EMinimizers>::Table g_minimizer_table[] = {
	{"simplex", min_nmsimplex}, 
	{"cg-fr", min_cg_fr}, 
	{"cg-pr", min_cg_pr}, 
	{"bfgs", min_bfgs}, 
	{"bfgs2", min_bfgs2}, 
	{"gd", min_gd},  
	{NULL, min_undefined}
}; 



int do_main( int argc, const char *argv[] )
{
	string src_filename; 
	string ref_filename; 
	string out_filename; 
	string trans_filename; 
	string transform_type("translate"); 
	EMinimizers minimizer = min_nmsimplex; 

	size_t mg_levels = 3;
	
	CCmdOptionList options;
	options.push_back(make_opt( src_filename, "in", 'i', "test image", "input", true)); 
	options.push_back(make_opt( ref_filename, "ref", 'r', "reference image", "input", true)); 
	options.push_back(make_opt( out_filename, "out", 'o', "registered output image", "output", true)); 
	options.push_back(make_opt( trans_filename, "trans", 't', "transformation", "transformation", false)); 
	options.push_back(make_opt( mg_levels, "levels", 'l', "multigrid levels", "levels", false));
	options.push_back(make_opt( minimizer, TDictMap<EMinimizers>(g_minimizer_table), 
				    "optimizer", 'O', "Optimizer used for minimization", "optimizer", false));
	options.push_back(make_opt( transform_type, "transForm", 'f', "transformation typo", "transform", false));

	options.parse(argc, argv); 
	
	P2DImage Model = load_image2d(src_filename); 
	P2DImage Reference = load_image2d(ref_filename); 
	
	C2DBounds GlobalSize = Model->get_size(); 
	if (GlobalSize != Reference->get_size()){
		throw std::invalid_argument("Images have different size");
	}
			
	auto cost = C2DImageCostPluginHandler::instance().produce("ssd");
	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3));
	
	C2DRigidRegister rr(cost, minimizer,  transform_type, *ipfactory);

	P2DTransformation transform = rr.run(Model, Reference, mg_levels); 
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
