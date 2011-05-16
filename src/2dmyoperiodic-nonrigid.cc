/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


/*
  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dmyoperiodic-nonrigid}
  \label{mia-2dmyoperiodic-nonrigid}

  \begin{description} 
  \item [Description:] 
	This program runs the non-rigid motion compensation of an cardiac 
        perfusion image series preferable aquired letting the patient breath freely
	(cf.\citet{wollny10b}). 
	Note, this is a complete re-write of the software used in that paper, and therefore, 
        the optimal parameters reported there are not optimal for this implememntations. 
  
  The program is called like 
  \begin{lstlisting}
mia-2dmyoperiodic-nonrigid -i <input set> -o <output set> [options]
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{registered}{r}{string}{File name base for the registered images. Image type and numbering 
                                 scheme are taken from the input images.}
  \cmdopt{save-references}{}{string}{Save the synthetic reference images to files with the given name base}
				 
  \cmdgroup{Preconditions} 
  \cmdopt{skip}{k}{int}{Skip a number of frames at the beginning of the series}
  \cmdopt{max-candidates}{}{int}{Maximum number of candidates for global reference image}
  \cmdopt{cost-series}{S}{string}{Const function to use for the analysis of the series 
                                 (see sections \ref{sec:2dfullcost} and \ref{sec:cost2d})}
  \cmdopt{ref-idx}{}{string}{Save the obtained index of the global reference image to this file}

  \cmdgroup{Image registration} 
  \cmdopt{cost-subset}{1}{string}{Image similarity measure to optimize during the first registration 
                                  phase of the algorithm (see section \ref{sec:2dfullcost})}
  \cmdopt{cost-final}{2}{string}{Image similarity measure to optimize during the second (final) registration 
                                  phase of the algorithm (see section \ref{sec:2dfullcost})}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{interpolator}{p}{string}{Image interpolator to be used}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{passes}{P}{int}{Number of ICA+Registration passes to be run}
  \cmdopt{divcurl}{d}{float}{divcurl regularization weight}
  \cmdopt{transForm}{f}{string}{Transformation space as provided by the 
                                \hyperref[sec:2dtransforms]{transformation plug-ins.}}
  }

  \item [Example:]Register the perfusion series given in segment.set by optimizing a spline based 
                  transformation with a coefficient rate of 16 pixel ,skipping two images at the 
		  beginning and using \emph{normalized gradient fields} as initial cost measure 
                  and SSD as final measure. 
                  Penalize the transformation by using divcurl with aweight of 2.0. 
  \begin{lstlisting}
mia-2dmyoperiodic-nonrigid  -i segment.set -o registered.set -k 2 -F spline:rate=16 -d 2.0 \
                     -1 image:cost=[ngf:eval=ds] -2 image:cost=ssd
  \end{lstlisting}
  \item [See also:] \sa{mia-2dmyomilles}, \sa{mia-2dmyoserial-nonrigid}, 
                    \sa{mia-2dmyoica-nonrigid}, \sa{mia-2dmyopgt-nonrigid},
		    \sa{mia-2dsegseriesstats}
  \end{description}
  
  LatexEnd
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
#include <mia/core/factorycmdlineoption.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/2dimageio.hh>
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
		T2DImage<T> *result = new T2DImage<T>(a.get_size());
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

class Convert2Float {
public: 
	C2DFImage operator () (P2DImage image) const; 
private: 
	FConvert2DImage2float m_converter; 
}; 


class C2DMyocardPeriodicRegistration {
public: 
	class RegistrationParams {
	public: 
		RegistrationParams(); 
		PMinimizer minimizer;
		double divcurlweight; 
		P2DFullCost pass1_cost;  
		P2DFullCost pass2_cost;
		P2DFullCost series_select_cost;  
		P2DTransformationFactory transform_creator; 
		shared_ptr<C2DInterpolatorFactory> interpolator; 
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
		cvmsg() << "Register " << *i << " to " << m_ref << "\n"; 
		P2DImage src = images[*i]; 
		P2DTransformation transform = nr.run(src, ref);
		images[*i] = (*transform)(*images[*i], *m_params.interpolator);
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
	
	for (size_t i = 0; i < images.size(); ++i) {

		if (i == *low_index)
			continue; 
		
		if (i == *high_index) {
			++low_index; 
			++high_index; 
			if (high_index != subset.end())
				continue;
		}

		// the last images may be registered using SSD without interpolating references 
		P2DImage ref; 
		if (high_index != subset.end()) {
			float w = float(*high_index - i)/(*high_index - *low_index);  
			FAddWeighted lerp(w);
			
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
		images[i] = (*transform)(*images[i], *m_params.interpolator);
		transforms[i] = transform; 
	}
}

C2DMyocardPeriodicRegistration::C2DMyocardPeriodicRegistration(const RegistrationParams& params):
	m_params(params)
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
	minimizer(CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1")), 
	divcurlweight(5),
	pass1_cost(C2DFullCostPluginHandler::instance().produce("image:cost=[ngf:eval=ds]")), 
	pass2_cost(C2DFullCostPluginHandler::instance().produce("image:cost=ssd")), 
	series_select_cost(C2DFullCostPluginHandler::instance().produce("image:cost=[ngf:eval=ds]")),
	transform_creator(C2DTransformCreatorHandler::instance().produce("spline")),
	mg_levels(3),
	max_candidates(20), 
	save_ref(false)
{
}

int do_main( int argc, const char *argv[] )
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
	EInterpolation interpolator = ip_bspline3;

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
	options.add(make_opt(params.series_select_cost, "cost-series", 'S',
				   "Const function to use for the analysis of the series")); 
	options.add(make_opt(reference_index_file, "ref-idx", 0, 
				   "save reference index number to this file"));  


	options.set_group("\nRegistration"); 


	options.add(make_opt( params.minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.add(make_opt( params.mg_levels, "mr-levels", 'l', "multi-resolution levels"));

	options.add(make_opt( params.divcurlweight, "divcurl", 'd', 
				    "divcurl regularization weight"));

	options.add(make_opt( params.transform_creator, "transForm", 'f', 
				    "transformation type"));

	options.add(make_opt(params.pass1_cost, "cost-subset", '1', 
				   "Cost function for registration during the subset registration")); 

	options.add(make_opt(params.pass2_cost, "cost-final", '2', 
				   "Cost function for registration during the final registration")); 
	

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	params.interpolator.reset(create_2dinterpolation_factory(interpolator));

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries in_images = input_set.get_images(); 
	if (skip >= in_images.size()) {
		THROW(invalid_argument, "Try to skip " << skip 
		      << " images, but input set has only " << in_images.size() << " images.");  
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
	return ::mia::filter(m_converter, *image); 
}
