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
#include <mia/2d/fullcost.hh>
#include <mia/2d/similarity_profile.hh>

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


struct FAddWeighted: public TFilter<P2DImage> {
	FAddWeighted(float w):
		_M_w(w)
	{
	}

	template <typename T, typename S>
	P2DImage operator() (const T2DImage<T>& a, const T2DImage<S>& b) const
	{
		if (a.get_size() != b.get_size()) {
			throw invalid_argument("input images cann not be combined because they differ in size");
		}
		T2DImage<T> *result = new T2DImage<T>(a.get_size());
		auto r = result->begin();
		auto e = result->end();

		auto ia = a.begin();
		auto ib = b.begin();

		float w2 = 1.0 - _M_w;

		while ( r != e ) {
			*r = (T)(w2 * *ia + _M_w * (float)*ib);
			++r;
			++ia;
			++ib;
		}

		return P2DImage(result);
	}

private:
	float _M_w;
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
		shared_ptr<C2DInterpolatorFactory> interpolator; 
		size_t mg_levels; 
		size_t skip; 
	};

	C2DMyocardPeriodicRegistration(const RegistrationParams& params); 
	void run(C2DImageSeries& images); 
private: 
	vector<size_t> get_prealigned_subset(const C2DImageSeries& images);  
	void run_initial_pass(C2DImageSeries& images, const vector<size_t>& subset); 
	void run_final_pass(C2DImageSeries& images, const vector<size_t>& subset); 
	
	RegistrationParams m_params; 
	size_t m_ref; 
}; 

vector<size_t> C2DMyocardPeriodicRegistration::get_prealigned_subset(const C2DImageSeries& images) 
{
	cvmsg() << "estimate prealigned subset ...\n"; 
	size_t ref = m_params.skip + 20; 
	
	C2DSimilarityProfile best_series(m_params.series_select_cost, images, m_params.skip, ref); 
	
	// the skip values should be parameters 
	for (size_t i = m_params.skip + 21; i < images.size()-2; ++i) {
		C2DSimilarityProfile sp(m_params.series_select_cost, images,  m_params.skip, i); 
		if (sp.get_peak_frequency() > best_series.get_peak_frequency()) {
			m_ref = i; 
			best_series = sp; 
		}
	}
	return best_series.get_periodic_subset(); 
}


void C2DMyocardPeriodicRegistration::run_initial_pass(C2DImageSeries& images, const vector<size_t>& subset) 
{
	cvmsg() << "run initial registration pass ...\n"; 
	C2DFullCostList costs; 
	// create costs
	costs.push(m_params.pass1_cost); 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << m_params.divcurlweight; 
	costs.push(C2DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 

	C2DNonrigidRegister nr(costs, 
			       m_params.minimizer, 
			       m_params.transform_creator, 
			       *m_params.interpolator,  
			       m_params.mg_levels);
	
	P2DImage ref = images[m_ref]; 
	for (auto i = subset.begin(); i != subset.end(); ++i) {
		if (m_ref == *i) 
			continue; 
		P2DImage src = images[*i]; 
		P2DTransformation transform = nr.run(src, ref);
		images[*i] = (*transform)(*images[*i], *m_params.interpolator);
	}
}

void C2DMyocardPeriodicRegistration::run_final_pass(C2DImageSeries& images, const vector<size_t>& subset)
{
	cvmsg() << "run final registration pass ...\n"; 
	C2DFullCostList costs; 
	// create costs
	costs.push(m_params.pass2_cost); 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << m_params.divcurlweight; 
	costs.push(C2DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 


	C2DNonrigidRegister nr(costs, 
			       m_params.minimizer, 
			       m_params.transform_creator, 
			       *m_params.interpolator,  
			       m_params.mg_levels);

	auto low_index = subset.begin(); 
	auto high_index = low_index + 1; 
	float delta = *high_index - *low_index; 
	assert(delta > 0.0); 
	
	for (size_t i = m_params.skip; i < images.size(); ++i) {
		if (i == *low_index)
			continue; 
		
		if (i == *high_index) {
			++low_index; 
			++high_index; 
			assert(high_index != subset.end());
			continue;
		}

		float w = *high_index - i;  
		FAddWeighted lerp(w);
		
		P2DImage ref = mia::filter(lerp, *images[*low_index], *images[*high_index]); 
		P2DImage src = images[i]; 
		P2DTransformation transform = nr.run(src, ref);
		images[i] = (*transform)(*images[i], *m_params.interpolator);
	}
}

C2DMyocardPeriodicRegistration::C2DMyocardPeriodicRegistration(const RegistrationParams& params):
	m_params(params)
{
}

void C2DMyocardPeriodicRegistration::run(C2DImageSeries& images)
{
	vector<size_t> subset = get_prealigned_subset(images); 
	run_initial_pass(images, subset); 
	run_final_pass(images, subset); 
}


C2DMyocardPeriodicRegistration::RegistrationParams::RegistrationParams():
	minimizer(min_gd), 
	divcurlweight(5),
	pass1_cost(C2DFullCostPluginHandler::instance().produce("image:cost=[ngf:eval=ds]")), 
	pass2_cost(C2DFullCostPluginHandler::instance().produce("image:cost=ssd")), 
	series_select_cost(C2DFullCostPluginHandler::instance().produce("image:cost=[ngf:eval=dot]")),
	transform_creator(C2DTransformCreatorHandler::instance().produce("spline")),
	mg_levels(3),
	skip(2)
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
	EInterpolation interpolator = ip_bspline3;

	C2DMyocardPeriodicRegistration::RegistrationParams params;

	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO");
	options.push_back(make_opt( in_filename, "in-file", 'i', "input perfusion data set", 
				    "input", true));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output perfusion data set", 
				    "output", true));
	options.push_back(make_opt( registered_filebase, "registered", 'r', 
				    "file name base for registered fiels", "registered", false)); 
	
	options.set_group("\nRegistration"); 
	options.push_back(make_opt( params.minimizer, TDictMap<EMinimizers>(g_minimizer_table),
				    "optimizer", 'O', "Optimizer used for minimization", 
				    "optimizer", false));
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.push_back(make_opt( params.mg_levels, "mg-levels", 'l', "multi-resolution levels", 
				    "mg-levels", false));
	options.push_back(make_opt( params.transform_creator, "transForm", 'f', 
				    "transformation type", "transform", false));

	options.push_back(make_opt(params.pass1_cost, "cost-subset", '1', 
				   "Cost function for registration during the subset registration", 
				   "cost-subset", false)); 

	options.push_back(make_opt(params.pass2_cost, "cost-final", '2', 
				   "Cost function for registration during the final registration", 
				   "cost-final", false)); 
	options.push_back(make_opt(params.series_select_cost, "cost-series", 'S',
				   "Const function to use for the analysis of the series", 
				   "cost-series", false)); 

	

	options.parse(argc, argv, false);
	params.interpolator.reset(create_2dinterpolation_factory(interpolator));

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries images = input_set.get_images(); 
	
	C2DMyocardPeriodicRegistration mpr(params); 
	mpr.run(images);
	
	input_set.set_images(images); 
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
