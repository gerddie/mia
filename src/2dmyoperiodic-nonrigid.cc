/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
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

#define VSTREAM_DOMAIN "2dmilles"

#include <fstream>
#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const char *g_general_help = 
	"This program runs the non-rigid registration of an perfusion image series\n"
	"preferable aquired letting the patient breath freely.\n" 
	"The registration algoritm implementes\n"
	"G. Wollny, M-J Ledesma-Cabryo, P.Kellman, and A.Santos, \"Exploiting \n"
	"Quasiperiodicity in Motion Correction of Free-Breathing,\" \n"
	"IEEE Transactions on Medical Imaging, 29(8), 2010\n\n"
	"Basic usage: \n"
	" mia-2dmyoperiodix-nonrigid [options] "; 


const TDictMap<EMinimizers>::Table g_minimizer_table[] = {
	{"cg-fr", min_cg_fr},
	{"cg-pr", min_cg_pr},
	{"bfgs", min_bfgs},
	{"bfgs2", min_bfgs2},
	{"gd", min_gd},
	{NULL, min_undefined}
};


class C2DFImage2PImage {
public: 
	P2DImage operator () (const C2DFImage& image) const {
		return P2DImage(new C2DFImage(image)); 
	}
}; 

class Convert2Float {
public: 
	C2DFImage operator () (P2DImage image) const; 
private: 
	FConvert2DImage2float _M_converter; 
}; 


class C2DMyocardPeriodicRegistration {
public: 
	class RegistrationParams {
	public: 
		RegistrationParams(); 
		EMinimizers minimizer;
		double divcurlweight; 
		P2DFullCost pass1_cost;  
		P2DFullCost pass2_cost;
		P2DFullCost series_select_cost;  
		P2DTransformationFactory transform_creator; 
		EInterpolation interpolator; 
		size_t mg_levels; 
	};

	C2DMyocardPeriodicRegistration(const RegistrationParams& params); 

private: 
	vector<size_t> get_prealigned_subset(const C2DImageSeries& images);  
	
	RegistrationParams m_registration_params; 
	size_t m_skip; 
	size_t m_ref; 

}; 

vector<size_t> C2DMyocardPeriodicRegistration::get_prealigned_subset(const C2DImageSeries& images) 
{
	size_t ref = skip + 20; 
	auto cost = C2DFullCostPluginHandler::instance().produce(series_select_cost);
	
	C2DSimilarityProfile best_series(images, cost, m_skip, m_skip + 20); 
	
	for (size_t i = m_skip + 21; dx.second < images.size()-2; ++dx.second) {
		C2DSimilarityProfile sp(images, cost, m_skip, i); 
		if (sp..get_peak_frequency() > best_series.get_peak_frequency()) {
			m_ref = i; 
			best_series = sp; 
		}
	}
	return best_series.get_periodic_subset(); 
}


void C2DMyocardPeriodicRegistration::run_initial_pass(const C2DImageSeries& images, const vector<int>& subset) 
{
	C2DFullCostList costs; 
	// create costs
	costs.push_back(pass1_cost); 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << divcurlweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 

	auto regularization = C2DFullCostPluginHandler::instance().produce(divcurl_descr); 
	

	C2DNonrigidRegister nr(costs, 
			       m_registration_params.minimizer, 
			       m_registration_params.transform_creation, 
			       m_registration_params.ipf,  
			       m_registration_params.mg_levels);
	
	P2DImage ref = images[ref]; 
	for (auto i = subset.begin(); i != subset.end(); ++i) {
		if (ref == *i) 
			continue; 
		P2DImage src = images[i]; 
		P2DTransformation transform = nr.run(src, ref);
		images[i] = (*transform)(images[i], ipd);
	}
}

C2DMyocardPeriodicRegistration(const RegistrationParams& params):

	m_registration_params(params), 
	
C2DMyocardPeriodicRegistration::RegistrationParams::RegistrationParams():
	minimizer(min_gd), 
	c_rate(8), 
	divcurlweight(5),
	pass1_cost("ngf:kernel=sd"), 
	pass2_cost("ssd"), 
	series_cost("ngf:kernel=dot"),
	transform_creator(C2DTransformCreatorHandler::instance().produce("spline")),
	interpolator(ip_bspline3), 
	mg_levels(3)
{
}

int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;


	C2DMyocardPeriodicRegistration::RegistrationParams params; 

	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.push_back(make_opt( in_filename, "in-file", 'i', "input perfusion data set", "input", true));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output perfusion data set", "output", true));
	options.push_back(make_opt( registered_filebase, "registered", 'r', "file name base for registered fiels", 
				    "registered", false)); 
	
	
	options.set_group("\nRegistration"); 
	options.push_back(make_opt( params.minimizer, TDictMap<EMinimizers>(g_minimizer_table),
				    "optimizer", 'O', "Optimizer used for minimization", "optimizer", false));
	options.push_back(make_opt( params.interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.push_back(make_opt( params.mg_levels, "mg-levels", 'l', "multi-resolution levels", "mg-levels", false));
	options.push_back(make_opt( params.transform_creator, "transForm", 'f', 
				    "transformation type", "transform", false));

	options.push_back(make_opt(params.pass1_cost, '1', "cost-subset", 
				   "Cost function for registration during the subset registration", "cost-subset", 
				   false)); 

	options.push_back(make_opt(params.pass2_cost, '2', "cost-final", 
				   "Cost function for registration during the final registration", "cost-final", 
				   false)); 
	options.push_back(make_opt(params.series_select_cost, 'S',"cost-series", 
				   "Const function to use for the analysis of the series", "cost-series",
				   false)); 

	

	options.parse(argc, argv, false);
	


	// this cost will always be used 

	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(interpolator));

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 
	
	cvmsg() << "skipping " << skip_images << " images\n"; 
	vector<C2DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
		  series.begin(), Convert2Float()); 
	

	// run ICA
	C2DPerfusionAnalysis ica(components, !no_normalize, !no_meanstrip); 
	if (max_ica_iterations) 
		ica.set_max_ica_iterations(max_ica_iterations); 
	if (!ica.run(series)) 
		throw runtime_error("ICA analysis didn't result in usable components"); 
	vector<C2DFImage> references_float = ica.get_references(); 
	
	C2DImageSeries references(references_float.size()); 
	transform(references_float.begin(), references_float.end(), references.begin(), C2DFImage2PImage()); 

	// crop if requested
	if (box_scale) {
		segment_and_crop_input(input_set, ica, box_scale, segmethod, references, save_crop_feature); 
		input_images = input_set.get_images(); 
	}

	// save cropped images if requested
	if (!cropped_filename.empty()) {
		bfs::path cf(cropped_filename);
		cf.replace_extension(); 
		input_set.rename_base(cf.filename()); 
		input_set.save_images(cropped_filename);

		unique_ptr<xmlpp::Document> test_cropset(input_set.write());
		ofstream outfile(cropped_filename, ios_base::out );
		if (outfile.good())
			outfile << test_cropset->write_to_string_formatted();
		else 
			THROW(runtime_error, "unable to save to '" << cropped_filename << "'"); 

	}

	bool do_continue = ica.has_periodic(); 
	while (do_continue){
		++current_pass; 
		
		run_registration_pass(input_set, references,  skip_images,  minimizer, 
				      *ipfactory, mg_levels, c_rate, divcurlweight); 
		
		C2DPerfusionAnalysis ica2(components, !no_normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
	
		transform(input_set.get_images().begin() + skip_images, 
			  input_set.get_images().end(), series.begin(), Convert2Float()); 

		if (ica2.run(series) ) {
			divcurlweight /= divcurlweight_divider; 
			if (c_rate > 1) 
				c_rate /= c_rate_divider; 
			references_float = ica2.get_references(); 
			transform(references_float.begin(), references_float.end(), 
				  references.begin(), C2DFImage2PImage()); 
			run_registration_pass(input_set, references,  skip_images,  minimizer, 
					      *ipfactory, mg_levels, c_rate, divcurlweight); 
		} else {
			cvmsg() << "Stopping registration in pass " << current_pass 
				<< " because ICA didn't return useful results\n"; 
			break; 
		}
		do_continue =  (!pass || current_pass < pass) && ica2.has_periodic(); 
	}

	input_set.rename_base(registered_filebase); 
	input_set.save_images(out_filename); 
	
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;

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

inline C2DFImage Convert2Float::operator () (P2DImage image) const
{
	return ::mia::filter(_M_converter, *image); 
}
