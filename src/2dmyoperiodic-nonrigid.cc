/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define VSTREAM_DOMAIN "2dmyoperiodic"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/fullstats.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/fullcost.hh>
#include <mia/2d/similarity_profile.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 


const SProgramDescription g_general_help = {
        {pdi_group,  "Registration of series of 2D images"}, 
	{pdi_short, "Run a registration of a series of 2D images."}, 
	
	{pdi_description, "This program runs the non-rigid registration of an perfusion image series "
	 "preferable acquired letting the patient breath freely. " 
	 "The registration algorithm implementes "
	 "G. Wollny, M-J Ledesma-Cabryo, P.Kellman, and A.Santos, \"Exploiting "
	 "Quasiperiodicity in Motion Correction of Free-Breathing,\" "
	 "IEEE Transactions on Medical Imaging, 29(8), 2010\n"}, 

	{pdi_example_descr, "Register the perfusion series given in 'segment.set'. " 
	 "Skip two images at the beginning, usa spline transformation of a knot rate 16 pixels, "
	 "and penalize the transformation by divcurl with weight 5. "
	 "Store the result in 'registered.set'.\n"}, 
	
	{pdi_example_code, "  -i segment.set -o registered.set -k 2 -d 5 -f spline:rate=16,penalty=[divcurl:weight=5]"}
}; 

		
class C2DFImage2PImage {
public: 
	P2DImage operator () (const C2DFImage& image) const {
		return P2DImage(new C2DFImage(image)); 
	}
}; 


struct FAddWeighted: public TFilter<P2DImage> {
	FAddWeighted(float w):
		m_w(w)
	{
	}

	template <typename T, typename S>
	P2DImage operator() (const T2DImage<T>& a, const T2DImage<S>& b) const
	{
		if (a.get_size() != b.get_size()) {
			throw invalid_argument("input images cann not be combined because they differ in size");
		}
		T2DImage<T> *result = new T2DImage<T>(a.get_size(), a);
		auto r = result->begin();
		auto e = result->end();

		auto ia = a.begin();
		auto ib = b.begin();

		float w2 = 1.0 - m_w;

		// this should be properly clamped
		while ( r != e ) {
			*r = (T)(w2 * (float)*ia + m_w * (float)*ib);
			++r;
			++ia;
			++ib;
		}

		return P2DImage(result);
	}

private:
	float m_w;
};


class C2DMyocardPeriodicRegistration {
public: 
	class RegistrationParams {
	public: 
		RegistrationParams(); 
		PMinimizer minimizer;
		PMinimizer refinement_minimizer;
		P2DFullCost pass1_cost;  
		P2DFullCost pass2_cost;
		P2DFullCost series_select_cost;  
		P2DTransformationFactory transform_creator; 
		size_t mg_levels; 
		size_t max_candidates; 
		bool save_ref; 
	};

	C2DMyocardPeriodicRegistration(const RegistrationParams& params); 
	vector<P2DTransformation> run(C2DImageSeries& images); 
	size_t get_ref_idx()const; 

private: 
	vector<size_t>  get_high_contrast_candidates(const C2DImageSeries& images, 
						     size_t startidx, size_t endidx); 
	vector<size_t> get_prealigned_subset(const C2DImageSeries& images);  
	void run_initial_pass(C2DImageSeries& images, 
			      vector<P2DTransformation>& transforms, 
			      const vector<size_t>& subset); 
	void run_final_pass(C2DImageSeries& images, 
			    vector<P2DTransformation>& transforms, 
			    const vector<size_t>& subset); 
	
	RegistrationParams m_params; 
	size_t m_ref; 
}; 

struct IdxVariation {
	size_t index; 
	double variation; 
}; 

bool operator < (const IdxVariation& a, const IdxVariation& b) 
{
	return a.variation < b.variation; 
}

class CStatsEvaluator : public TFilter<CFullStats> {
public:
	template <typename T>
	CFullStats operator () (const T2DImage<T>& image) const {
		return CFullStats(image.begin(), image.end());
	}
};

vector<size_t>  C2DMyocardPeriodicRegistration::get_high_contrast_candidates(const C2DImageSeries& images, 
									     size_t startidx, size_t endidx)
{
	CStatsEvaluator sev; 
	priority_queue<IdxVariation> q; 
	while (startidx < endidx)  {
		IdxVariation v; 
		v.index = startidx; 
		v.variation = mia::filter(sev, *images[startidx]).sigma();  
		q.push(v); 
		++startidx; 
	}
	vector<size_t> result; 
	
	while (result.size() < m_params.max_candidates && !q.empty()) {
		IdxVariation v = q.top(); 
		result.push_back(v.index); 
		q.pop(); 
	}
	return result; 
}

vector<size_t> C2DMyocardPeriodicRegistration::get_prealigned_subset(const C2DImageSeries& images) 
{
	cvmsg() << "estimate prealigned subset ...\n"; 
	vector<size_t> candidates = get_high_contrast_candidates(images, 20, images.size()-2); 
	assert(!candidates.empty()); 

	C2DSimilarityProfile best_series(m_params.series_select_cost, images, candidates[0]); 
	m_ref = candidates[0]; 	

	// the skip values should be parameters 
	for (size_t i = 1; i < candidates.size(); ++i) {
		C2DSimilarityProfile sp(m_params.series_select_cost, images, candidates[i]); 
		if (sp.get_peak_frequency() > best_series.get_peak_frequency()) {
			m_ref = candidates[i]; 
			best_series = sp; 
		}
	}
	return best_series.get_periodic_subset(); 
}


void C2DMyocardPeriodicRegistration::run_initial_pass(C2DImageSeries& images, 
						      vector<P2DTransformation>& transforms, 
						      const vector<size_t>& subset) 
{
	cvmsg() << "run initial registration pass on "<< subset << "\n"; 
	C2DFullCostList costs; 
	// create costs
	costs.push(m_params.pass1_cost); 

	C2DNonrigidRegister nr(costs, 
			       m_params.minimizer, 
			       m_params.transform_creator, 
			       m_params.mg_levels);

	nr.set_refinement_minimizer(m_params.refinement_minimizer); 
	
	P2DImage ref = images[m_ref]; 
	for (auto i = subset.begin(); i != subset.end(); ++i) {
		if (m_ref == *i) 
			continue; 
		cvmsg() << "Register " << *i << " to " << m_ref << "\n"; 
		P2DImage src = images[*i]; 
		P2DTransformation transform = nr.run(src, ref);
		images[*i] = (*transform)(*images[*i]);
		transforms[*i] = transform; 
	}
}

void C2DMyocardPeriodicRegistration::run_final_pass(C2DImageSeries& images, 
						    vector<P2DTransformation>& transforms, 
						    const vector<size_t>& subset)
{
	cvmsg() << "run final registration pass ...\n"; 
	C2DFullCostList costs; 
	// create costs
	costs.push(m_params.pass2_cost); 
 	C2DNonrigidRegister nr(costs, 
			       m_params.minimizer, 
			       m_params.transform_creator, 
			       m_params.mg_levels);

	nr.set_refinement_minimizer(m_params.refinement_minimizer); 

	auto low_index = subset.begin(); 
	auto high_index = low_index + 1; 
	
	for (size_t i = 0; i < images.size(); ++i) {

		if (i == *low_index)
			continue; 

		// the last images may be registered using SSD without interpolating references 
		P2DImage ref; 
		if (high_index != subset.end()) {

			if (i == *high_index) {
				++low_index; 
				++high_index; 
				continue;
			}
				
			float w = float(*high_index - i)/(*high_index - *low_index);  
			FAddWeighted lerp(w);
			
			cvmsg() << "create " << i << " from [" << *low_index << ", " 
				<< *high_index << "] w = "<< w <<"\n"; 
			
			ref = mia::filter(lerp, *images[*high_index], *images[*low_index]); 
			if (m_params.save_ref) {
				stringstream refname; 
				refname << "ref" << setw(4) << setfill('0') << i << ".v"; 
				save_image(refname.str(), ref); 
				cvmsg() << "Save reference to " << refname.str() << "\n"; 
			}
		}else
			ref = images[*low_index]; 

		cvmsg() << "Register image " << i << "\n"; 		
		P2DTransformation transform = nr.run(images[i], ref);
		images[i] = (*transform)(*images[i]);
		transforms[i] = transform; 
	}
}

C2DMyocardPeriodicRegistration::C2DMyocardPeriodicRegistration(const RegistrationParams& params):
	m_params(params), 
	m_ref(0)
{
}

vector<P2DTransformation> C2DMyocardPeriodicRegistration::run(C2DImageSeries& images)
{
	vector<P2DTransformation> transforms(images.size()); 
	vector<size_t> subset = get_prealigned_subset(images); 
	run_initial_pass(images, transforms, subset); 
	run_final_pass(images, transforms, subset); 
	return transforms; 
}

size_t C2DMyocardPeriodicRegistration::get_ref_idx()const 
{
	return m_ref; 
}


C2DMyocardPeriodicRegistration::RegistrationParams::RegistrationParams():
	mg_levels(3),
	max_candidates(20), 
	save_ref(false)
{
}

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");
	string reference_index_file; 

	size_t skip = 0; 
	
	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;
	C2DMyocardPeriodicRegistration::RegistrationParams params;

	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO");
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "registered", 'r', 
				    "file name base for registered fiels")); 
	options.add(make_opt(params.save_ref,"save-references", 0, 
				   "Save synthetic references to files refXXXX.v")); 


	options.set_group("\nPreconditions"); 
	options.add(make_opt(skip, "skip", 'k', 
				   "Skip images at the begin of the series")); 
	options.add(make_opt(params.max_candidates, "max-candidates", 0, 
				   "maximum number of candidates for global reference image")); 
	options.add(make_opt(params.series_select_cost, "image:cost=[ngf:eval=ds]", "cost-series", 'S',
				   "Const function to use for the analysis of the series")); 
	options.add(make_opt(reference_index_file, "ref-idx", 0, 
				   "save reference index number to this file"));  


	options.set_group("\nRegistration"); 


	options.add(make_opt( params.minimizer, "gsl:opt=gd,step=0.01", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( params.refinement_minimizer, "", "refiner", 'R', "optimizer used for additional minimization"));
	options.add(make_opt( params.mg_levels, "mr-levels", 'l', "multi-resolution levels"));

	options.add(make_opt( params.transform_creator, "spline:rate=16,penalty=[divcurl:weight=0.01]", "transForm", 'f', 
				    "transformation type"));

	options.add(make_opt(params.pass1_cost, "image:cost=[ngf:eval=ds]", "cost-subset", '1', 
				   "Cost function for registration during the subset registration")); 

	options.add(make_opt(params.pass2_cost, "image:cost=ssd", "cost-final", '2', 
				   "Cost function for registration during the final registration")); 
	

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries in_images = input_set.get_images(); 
	if (skip >= in_images.size()) {
		throw create_exception<invalid_argument>( "Try to skip ", skip, " images, "
						"but input set has only ", in_images.size(), " images.");  
	}

	C2DImageSeries series(in_images.begin() + skip, in_images.end()); 

	C2DMyocardPeriodicRegistration mpr(params); 
	vector<P2DTransformation> transforms = mpr.run(series);

	if (!reference_index_file.empty()) {
		ofstream refidxfile(reference_index_file.c_str(), ios_base::out );
		refidxfile << mpr.get_ref_idx() + skip; 
	}
			
	
	copy(series.begin(), series.end(), in_images.begin() + skip); 
	input_set.set_images(in_images); 
	input_set.rename_base(registered_filebase); 
	input_set.save_images(out_filename); 

	// transform the segmentations
	CSegSet::Frames& frames = input_set.get_frames();
	for (size_t i = 0; i <  transforms.size(); ++i) {
		if (i != mpr.get_ref_idx()) 
			frames[i + skip].inv_transform(*transforms[i]);
	}
	
	input_set.set_preferred_reference(mpr.get_ref_idx() + skip); 
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
