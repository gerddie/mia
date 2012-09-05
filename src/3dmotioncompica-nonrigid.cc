/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "3dmotioncompica"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/filetools.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/3d/ica.hh>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
using namespace tbb;


using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const SProgramDescription g_description = {
	"Registration of series of 3D images", 

	"Non-linear registration of a series of 3D images.",
	
	"This program implements a 3D version of the motion compensation algorithm described in "
	"Wollny G, Kellman P, Santos A, Ledesma-Carbayo M-J, \"Automatic Motion Compensation of "
	"Free Breathing acquired Myocardial Perfusion Data by using Independent Component Analysis\", "
	"Medical Image Analysis, 2012, DOI:10.1016/j.media.2012.02.004.", 

	"Register the perfusion series given in images imagesXXXX.v by using 4-class ICA estimation. "
        "Skip two images at the beginning, use at most 4 registration threads, a nlopt based optimizer "
	"and otherwiese use the default parameters. "
	"Store the result in registeredXXXX.v ", 
	
	"-i images0000.v -o  registered%04d.v  -k 2 -C 4 -t 4 "
	"-O nlopt:opt=ld-var1,xtola=0.001,ftolr=0.001,maxiter=300"
}; 


class C3DFImage2PImage {
public: 
	P3DImage operator () (const C3DFImage& image) const {
		return P3DImage(new C3DFImage(image)); 
	}
}; 

class Convert2Float {
public: 
	Convert2Float(); 
	C3DFImage operator () (P3DImage image) const; 
private: 
	C3DFilterPluginHandler::ProductPtr m_converter; 
}; 


C3DFullCostList create_costs(double divcurlweight, const string& imagecostbase, int idx)
{
	stringstream cost_descr; 
	cost_descr << imagecostbase << "src=src" << idx << ".@,ref=ref" << idx << ".@"; 
	auto imagecost = C3DFullCostPluginHandler::instance().produce(cost_descr.str()); 

	C3DFullCostList result; 
	result.push(imagecost); 

	stringstream divcurl_descr; 
	divcurl_descr << "divcurl:weight=" << divcurlweight; 
	result.push(C3DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 
	return result; 
}

P3DTransformationFactory create_transform_creator(size_t c_rate)
{
	stringstream transf; 
	transf << "spline:rate=" << c_rate; 
	return C3DTransformCreatorHandler::instance().produce(transf.str()); 
}


struct SeriesRegistration {
	C3DImageSeries&  input_images; 
	const C3DImageSeries& references; 
	string minimizer; 
	size_t mg_levels; 
	double divcurlweight; 
	P3DTransformationFactory transform_creator; 
	string imagecostbase; 
	int skip_images; 
	
	SeriesRegistration(C3DImageSeries&  _input_images, 
			   const C3DImageSeries& _references, 
			   const string& _minimizer, 
			   size_t _mg_levels, 
			   double _divcurlweight, 
			   P3DTransformationFactory _transform_creator, 
			   string _imagecostbase, 
			   int _skip_images):
		input_images(_input_images), 
		references(_references), 
		minimizer(_minimizer), 
		mg_levels(_mg_levels), 
		divcurlweight(_divcurlweight), 
		transform_creator(_transform_creator), 
		imagecostbase(_imagecostbase), 
		skip_images(_skip_images)
		{
		}
	void operator()( const blocked_range<int>& range ) const {
		CThreadMsgStream thread_stream;
		TRACE_FUNCTION; 
		auto m =  CMinimizerPluginHandler::instance().produce(minimizer);
		for( int i=range.begin(); i!=range.end(); ++i ) {
			auto costs  = create_costs(divcurlweight, imagecostbase, i); 
			C3DNonrigidRegister nrr(costs, m,  transform_creator, mg_levels, i);
			P3DTransformation transform = nrr.run(input_images[i + skip_images], references[i]);
			input_images[i + skip_images] = (*transform)(*input_images[i + skip_images]);
		}
	}
};  

void run_registration_pass(C3DImageSeries& input_images, const C3DImageSeries& references,  
			   int skip_images,  const string& minimizer, 
			   size_t mg_levels, double c_rate, double divcurlweight, 
			   const string&   imagecost) 
{

	SeriesRegistration sreg(input_images,references, minimizer, 
				mg_levels, divcurlweight, create_transform_creator(c_rate), 
				imagecost, skip_images); 
	parallel_for(blocked_range<int>( 0, references.size()), sreg);
}

void save_references(const string& save_ref, int current_pass, int skip_images, const C3DImageSeries& references)
{
	for(size_t i = 0 ; i < references.size(); ++i) {
		stringstream filename; 
		filename << save_ref << current_pass << "-" 
			 << setw(4) << setfill('0') << skip_images + i << ".v"; 
		save_image(filename.str(), references[i]);
	}
}

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg%04d.v");
	string save_mixing_matrix; 
	string save_features; 

	string save_ref_filename;
	string save_reg_filename;

	// registration parameters
	string minimizer("gsl:opt=gd,step=0.1");
	string imagecost("image:weight=1,cost=ssd,");
	double c_rate = 32; 
	double c_rate_divider = 4; 
	double divcurlweight = 20.0; 
	double divcurlweight_divider = 4.0; 

	size_t mg_levels = 3; 

	// ICA parameters 
	size_t components = 0;
	bool no_normalize = false; 
	bool no_meanstrip = false; 
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 

	size_t current_pass = 0; 
	size_t pass = 3; 
	
	int max_threads = task_scheduler_init::automatic;

	CCmdOptionList options(g_description);
	
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input images (file pattern)", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-file", 'o', "output images (file name pattern)"));
	
	options.add(make_opt( save_ref_filename, "save-refs", 0, "save reference images")); 
	options.add(make_opt( save_reg_filename, "save-regs", 0, 
				    "save intermediate registered images")); 
	options.add(make_opt( save_mixing_matrix, "save-coeffs", 0, "save mixing matrix")); 
	options.add(make_opt( save_features, "save-features", 0, "save feature images")); 


	options.set_group("Registration"); 
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( c_rate, "start-c-rate", 'a', 
				    "start coefficinet rate in spines,"
				    " gets divided by --c-rate-divider with every pass"));
	options.add(make_opt( c_rate_divider, "c-rate-divider", 0, 
				    "cofficient rate divider for each pass"));
	options.add(make_opt( divcurlweight, "start-divcurl", 'd',
				    "start divcurl weight, gets divided by"
				    " --divcurl-divider with every pass")); 
	options.add(make_opt( divcurlweight_divider, "divcurl-divider", 0,
				    "divcurl weight scaling with each new pass")); 
	options.add(make_opt( imagecost, "imagecost", 'w', "image cost")); 
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( pass, "passes", 'P', "registration passes")); 

	options.set_group("ICA"); 
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation"));
	options.add(make_opt( no_normalize, "no-normalize", 0, "don't normalized ICs"));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves"));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "e.g. because as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	options.set_group("Processing"); 
	options.add(make_opt(max_threads, "threads", 'T', "Maxiumum number of threads to use for running the registration," 
			     "This number should be lower or equal to the number of processing cores in the machine"
			     " (default: automatic estimation)."));  

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	task_scheduler_init init(max_threads);
	
	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	P3DImageSeries input_images(new C3DImageSeries); 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P3DImage image = load_image<P3DImage>(src_name);
		if (!image)
			throw create_exception<runtime_error>( "image ", src_name, " not found");

		cvdebug() << "read '" << src_name << "\n";
		input_images->push_back(image);
	}

	cvmsg() << "skipping " << skip_images << " images\n"; 
	vector<C3DFImage> series(input_images->size() - skip_images); 
	transform(input_images->begin() + skip_images, input_images->end(), 
		  series.begin(), Convert2Float()); 
	

	// run ICA
	C3DImageSeriesICA ica(series, false); 
	if (max_ica_iterations) 
		ica.set_max_iterations(max_ica_iterations); 
	if (!ica.run(components, !no_meanstrip, !no_normalize)) {
		ica.set_approach(FICA_APPROACH_SYMM); 
		if (!ica.run(components, !no_meanstrip, !no_normalize))
			cvwarn() << "ICA not converged, but the SYMM approach has given something to work with ...\n";
	}

	vector<C3DFImage> references_float(series.size()); 
	C3DImageSeriesICA::IndexSet empty_set; 
	for (size_t i = 0; i < series.size(); ++i) {
		references_float[i] = ica.get_incomplete_mix(i, empty_set); 
		references_float[i].set_voxel_size(series[i].get_voxel_size()); 
	}
	
	P3DImageSeries references(new C3DImageSeries(references_float.size())); 
	transform(references_float.begin(), references_float.end(), references->begin(), C3DFImage2PImage()); 

	
	if (!save_mixing_matrix.empty()) {
		auto mix = ica.get_mixing_curves(); 
		ofstream coef_file(save_mixing_matrix.c_str());
		
		for (size_t r = 0; r < mix[0].size(); ++r) {
			for (size_t c = 0; c < mix.size(); ++c) {
				coef_file   << setw(10) << mix[c][r] << " ";
			}
			coef_file << "\n";
		}
		if (!coef_file.good()) 
			cverr() << "Unable to save mixing matrix to '" <<  save_mixing_matrix << "\n"; 
	}

	if (!save_features.empty()) {
		for (size_t i = 0; i < components; ++i) {
			stringstream fname; 
			fname << save_features << "-" << i << ".v"; 
			auto feat = ica.get_feature_image(i); 
			save_image(fname.str(), feat); 
		}
	}
	
	bool do_continue=true; 
	do {
		++current_pass; 
		cvmsg() << "Registration pass " << current_pass << "\n"; 

		if (!save_ref_filename.empty())
			save_references(save_ref_filename, current_pass, skip_images, *references); 

		run_registration_pass(*input_images, *references,  skip_images,  minimizer, 
				      mg_levels, c_rate, divcurlweight, imagecost); 
		
		if (!save_reg_filename.empty()) 
			save_references(save_reg_filename, current_pass, 0, *input_images); 

		transform(input_images->begin() + skip_images, 
			  input_images->end(), series.begin(), Convert2Float()); 

		
		
		C3DImageSeriesICA ica2(series, false); 
		if (max_ica_iterations) 
			ica2.set_max_iterations(max_ica_iterations); 
		if (!ica2.run(components, !no_meanstrip, !no_normalize)) {
			ica2.set_approach(FICA_APPROACH_SYMM); 
			ica2.run(components, !no_meanstrip, !no_normalize); 
		}
		
		divcurlweight /= divcurlweight_divider; 
		if (c_rate > 1) 
			c_rate /= c_rate_divider; 
		
		for (size_t i = 0; i < series.size(); ++i) {
			references_float[i] = ica.get_incomplete_mix(i, empty_set); 
			references_float[i].set_voxel_size(series[i].get_voxel_size()); 
		}

		transform(references_float.begin(), references_float.end(), 
			  references->begin(), C3DFImage2PImage()); 
		do_continue =  (current_pass < pass); 
	} while (do_continue); 


	bool success = true; 
	auto ii = input_images->begin(); 
	for (size_t i = start_filenum; i < end_filenum; ++i, ++ii) {
		string out_name = create_filename(registered_filebase.c_str(), i);
		cvmsg() << "Save image " << i << " to " << out_name << "\n"; 
		success &= save_image(out_name, *ii); 
	}
	
	return success ? EXIT_SUCCESS : EXIT_FAILURE;

}

Convert2Float::Convert2Float()
{
	m_converter = C3DFilterPluginHandler::instance().produce("convert:repn=float,map=copy"); 
}

inline C3DFImage Convert2Float::operator () (P3DImage image) const
{
	auto res = m_converter->filter(*image); 
	
	return dynamic_cast<C3DFImage&>(*res); 
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)
