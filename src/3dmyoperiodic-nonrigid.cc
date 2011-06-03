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
  
  \subsection{mia-3dmyoperiodic-nonrigid}
  \label{mia-3dmyoperiodic-nonrigid}

  \begin{description} 
  \item [Description:] 
	This program runs the non-rigid motion compensation of an cardiac 
        perfusion image series preferable aquired letting the patient breath freely
	(cf.\citet{wollny10b}). 
	Note, this is a complete re-write of the software used in that paper, and therefore, 
        the optimal parameters reported there are not optimal for this implememntations. 
  
  The program is called like 
  \begin{lstlisting}
mia-3dmyoperiodic-nonrigid -i <input set> -o <output set> [options]
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{File name base for the registered images. }
  \cmdopt{save-references}{}{string}{Save the synthetic reference images to files with the given name base}
				 
  \cmdgroup{Preconditions} 
  \cmdopt{skip}{k}{int}{Skip a number of frames at the beginning of the series}
  \cmdopt{max-candidates}{}{int}{Maximum number of candidates for global reference image}
  \cmdopt{cost-series}{S}{string}{Const function to use for the analysis of the series 
                                 (see sections \ref{sec:3dfullcost} and \ref{sec:cost3d})}
  \cmdopt{ref-idx}{}{string}{Save the obtained index of the global reference image to this file}

  \cmdgroup{Image registration} 
  \cmdopt{cost-subset}{1}{string}{Image similarity measure to optimize during the first registration 
                                  phase of the algorithm (see section \ref{sec:3dfullcost})}
  \cmdopt{cost-final}{2}{string}{Image similarity measure to optimize during the second (final) registration 
                                  phase of the algorithm (see section \ref{sec:3dfullcost})}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{interpolator}{p}{string}{Image interpolator to be used}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{passes}{P}{int}{Number of ICA+Registration passes to be run}
  \cmdopt{divcurl}{d}{float}{divcurl regularization weight}
  \cmdopt{transForm}{f}{string}{Transformation space as provided by the 
                                \hyperref[sec:3dtransforms]{transformation plug-ins.}}
  }

  \item [Example:]Register the perfusion series given in segment.set by optimizing a spline based 
                  transformation with a coefficient rate of 16 pixel ,skipping two images at the 
		  beginning and using \emph{normalized gradient fields} as initial cost measure 
                  and SSD as final measure. 
                  Penalize the transformation by using divcurl with aweight of 2.0. 
  \begin{lstlisting}
mia-3dmyoperiodic-nonrigid  -i segment.set -o registered.set -k 2 -F spline:rate=16 -d 2.0 \
                     -1 image:cost=[ngf:eval=ds] -2 image:cost=ssd
  \end{lstlisting}
  \item [See also:] \sa{mia-3dmyomilles}, \sa{mia-3dmyoserial-nonrigid}, 
                    \sa{mia-3dmyoica-nonrigid}, \sa{mia-3dmyopgt-nonrigid},
		    \sa{mia-3dsegseriesstats}
  \end{description}
  
  LatexEnd
*/

#define VSTREAM_DOMAIN "3dmyoperiodic"

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
#include <mia/core/filetools.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/fullcost.hh>
#include <mia/3d/similarity_profile.hh>

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
	" mia-3dmyoperiodix-nonrigid [options] "; 


class C3DFImage2PImage {
public: 
	P3DImage operator () (const C3DFImage& image) const {
		return P3DImage(new C3DFImage(image)); 
	}
}; 


struct FAddWeighted: public TFilter<P3DImage> {
	FAddWeighted(float w):
		m_w(w)
	{
	}

	template <typename T, typename S>
	P3DImage operator() (const T3DImage<T>& a, const T3DImage<S>& b) const
	{
		if (a.get_size() != b.get_size()) {
			throw invalid_argument("input images cann not be combined because they differ in size");
		}
		T3DImage<T> *result = new T3DImage<T>(a.get_size(), a);
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

		return P3DImage(result);
	}

private:
	float m_w;
};

class Convert2Float : public TFilter<C3DFImage>{
public: 
	C3DFImage operator () (P3DImage image) const; 

	template <typename T> 
	C3DFImage operator () (const T3DImage<T> image) const {
		C3DFImage result(image.get_size()); 
		copy(image.begin(), image.end(), result.begin()); 
		return result; 
	}
}; 


class C3DMyocardPeriodicRegistration {
public: 
	class RegistrationParams {
	public: 
		RegistrationParams(); 
		PMinimizer minimizer;
		double divcurlweight; 
		P3DFullCost pass1_cost;  
		P3DFullCost pass2_cost;
		P3DFullCost series_select_cost;  
		P3DTransformationFactory transform_creator; 
		shared_ptr<C3DInterpolatorFactory> interpolator; 
		size_t mg_levels; 
		size_t max_candidates; 
		bool save_ref; 
	};

	C3DMyocardPeriodicRegistration(const RegistrationParams& params); 
	vector<P3DTransformation> run(C3DImageSeries& images); 
	size_t get_ref_idx()const; 

private: 
	vector<size_t>  get_high_contrast_candidates(const C3DImageSeries& images, 
						     size_t startidx, size_t endidx); 
	vector<size_t> get_prealigned_subset(const C3DImageSeries& images);  
	void run_initial_pass(C3DImageSeries& images, 
			      vector<P3DTransformation>& transforms, 
			      const vector<size_t>& subset); 
	void run_final_pass(C3DImageSeries& images, 
			    vector<P3DTransformation>& transforms, 
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
	CFullStats operator () (const T3DImage<T>& image) const {
		return CFullStats(image.begin(), image.end());
	}
};

vector<size_t>  C3DMyocardPeriodicRegistration::get_high_contrast_candidates(const C3DImageSeries& images, 
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

vector<size_t> C3DMyocardPeriodicRegistration::get_prealigned_subset(const C3DImageSeries& images) 
{
	cvmsg() << "estimate prealigned subset ...\n"; 
	vector<size_t> candidates = get_high_contrast_candidates(images, 20, images.size()-2); 
	assert(!candidates.empty()); 

	C3DSimilarityProfile best_series(m_params.series_select_cost, images, candidates[0]); 
	m_ref = candidates[0]; 

	// the skip values should be parameters 
	for (size_t i = 1; i < candidates.size(); ++i) {
		C3DSimilarityProfile sp(m_params.series_select_cost, images, candidates[i]); 
		if (sp.get_peak_frequency() > best_series.get_peak_frequency()) {
			m_ref = candidates[i]; 
			best_series = sp; 
		}
	}
	return best_series.get_periodic_subset(); 
}


void C3DMyocardPeriodicRegistration::run_initial_pass(C3DImageSeries& images, 
						      vector<P3DTransformation>& transforms, 
						      const vector<size_t>& subset) 
{
	cvmsg() << "run initial registration pass on "<< subset << "\n"; 
	C3DFullCostList costs; 
	// create costs
	costs.push(m_params.pass1_cost); 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << m_params.divcurlweight; 
	costs.push(C3DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 

	C3DNonrigidRegister nr(costs, 
			       m_params.minimizer, 
			       m_params.transform_creator, 
			       *m_params.interpolator,  
			       m_params.mg_levels);
	
	P3DImage ref = images[m_ref]; 
	for (auto i = subset.begin(); i != subset.end(); ++i) {
		if (m_ref == *i) 
			continue; 
		cvmsg() << "Register " << *i << " to " << m_ref << "\n"; 
		P3DImage src = images[*i]; 
		P3DTransformation transform = nr.run(src, ref);
		images[*i] = (*transform)(*images[*i], *m_params.interpolator);
		transforms[*i] = transform; 
	}
}

void C3DMyocardPeriodicRegistration::run_final_pass(C3DImageSeries& images, 
						    vector<P3DTransformation>& transforms, 
						    const vector<size_t>& subset)
{
	cvmsg() << "run final registration pass ...\n"; 
	C3DFullCostList costs; 
	// create costs
	costs.push(m_params.pass2_cost); 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << m_params.divcurlweight; 
	costs.push(C3DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 

	C3DNonrigidRegister nr(costs, 
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
		P3DImage ref; 
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
		P3DTransformation transform = nr.run(images[i], ref);
		images[i] = (*transform)(*images[i], *m_params.interpolator);
		transforms[i] = transform; 
	}
}

C3DMyocardPeriodicRegistration::C3DMyocardPeriodicRegistration(const RegistrationParams& params):
	m_params(params), 
	m_ref(0)
{
}

vector<P3DTransformation> C3DMyocardPeriodicRegistration::run(C3DImageSeries& images)
{
	vector<P3DTransformation> transforms(images.size()); 
	vector<size_t> subset = get_prealigned_subset(images); 
	run_initial_pass(images, transforms, subset); 
	run_final_pass(images, transforms, subset); 
	return transforms; 
}

size_t C3DMyocardPeriodicRegistration::get_ref_idx()const 
{
	return m_ref; 
}


C3DMyocardPeriodicRegistration::RegistrationParams::RegistrationParams():
	minimizer(CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1")), 
	divcurlweight(5),
	pass1_cost(C3DFullCostPluginHandler::instance().produce("image:cost=[ngf:eval=ds]")), 
	pass2_cost(C3DFullCostPluginHandler::instance().produce("image:cost=ssd")), 
	series_select_cost(C3DFullCostPluginHandler::instance().produce("image:cost=[ngf:eval=ds]")),
	transform_creator(C3DTransformCreatorHandler::instance().produce("spline")),
	mg_levels(3),
	max_candidates(20), 
	save_ref(false)
{
}

int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string registered_filebase("reg%04d.v");
	string reference_index_file; 

	size_t skip = 0; 
	
	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;
	EInterpolation interpolator = ip_bspline3;

	C3DMyocardPeriodicRegistration::RegistrationParams params;

	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO");
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-file", 'o', 
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

	params.interpolator.reset(create_3dinterpolation_factory(interpolator));

		size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	C3DImageSeries in_images; 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P3DImage image = load_image<P3DImage>(src_name);
		if (!image)
			THROW(runtime_error, "image " << src_name << " not found");

		cvdebug() << "read '" << src_name << "\n";
		in_images.push_back(image);
	}

	if (skip >= in_images.size()) {
		THROW(invalid_argument, "Try to skip " << skip 
		      << " images, but input set has only " << in_images.size() << " images.");  
	}

	C3DImageSeries series(in_images.begin() + skip, in_images.end()); 

	C3DMyocardPeriodicRegistration mpr(params); 
	vector<P3DTransformation> transforms = mpr.run(series);

	if (!reference_index_file.empty()) {
		ofstream refidxfile(reference_index_file.c_str(), ios_base::out );
		refidxfile << mpr.get_ref_idx() + skip; 
	}
			
	
	copy(series.begin(), series.end(), in_images.begin() + skip); 

	bool success = true; 
	auto ii = in_images.begin(); 
	for (size_t i = start_filenum; i < end_filenum; ++i, ++ii) {
		string out_name = create_filename(registered_filebase.c_str(), i);
		cvmsg() << "Save image " << i << " to " << out_name << "\n"; 
		success &= save_image(out_name, *ii); 
	}
	return success ? EXIT_SUCCESS : EXIT_FAILURE;

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

inline C3DFImage Convert2Float::operator () (P3DImage image) const
{
	return ::mia::filter(*this, *image); 
}
