/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2011
 *
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

  LatexBeginProgramDescription{Prostate motion compensation}
  
  \subsection{mia-3dmotioncompica-nonrigid}
  \label{mia-3dmotioncompica-nonrigid}

  \begin{description} 
  \item [Description:] 
        This program is used to run a ICA based non-linear registration approach 
        for motion compensation in myocardial perfusion imaging described in Wollny at al. 
        \cite{wollny11bvm}. 
	Specifically, the non-linear transformation is defined in terms of B-splines that 
        is regularized by a DivCurl operator that penalizes the gradients of divergence and 
        curl of the transformation. 
	In addition, the code supports the automatic extraction of a region of interest 
        around the left heart ventricle to speed up computation.
  
  The program is called like 
  \begin{lstlisting}
mia-3dmotioncompica-nonrigid -i <input set> -o <output set> [options]
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{registered}{r}{string}{File name base for the registered images. Image type and numbering 
                                 scheme are taken from the input images.}
  \cmdopt{save-cropped}{}{string}{save cropped set to this file, the image files will use the stem of the 
                                 name as file name base}
  \cmdopt{save-feature}{}{string}{save segmentation feature images and initial ICA mixing matrix}
  \cmdopt{save-refs}{}{string}{for each registration pass save the reference images to files with the given name base}
  \cmdopt{save-regs}{}{string}{for each registration pass save intermediate registered images}
				 
  \cmdgroup{Independent component analysis} 
  \cmdopt{components}{C}{int}{Number of  ICA components to be used, 0 = automatic estimation}
  \cmdopt{skip}{k}{int}{Skip a number of frames at the beginning of the series}
  \cmdopt{no-normalize}{}{}{don't normalized ICs}
  \cmdopt{no-meanstrip}{}{}{don't strip the mean from the mixing curves}
  \cmdopt{segscale}{s}{float}{segment and scale the crop box around the LV (0=no segmentation)}
  \cmdopt{max-ica-iter}{m}{int}{maximum number of iterations within ICA}
  \cmdopt{segmethod}{E}{string}{Segmentation method - (delta-feature|delta-peak|features)}

  \cmdgroup{Image registration} 
  \cmdopt{imagecost}{w}{string}{Image similarity measure used (see section \ref{sec:3dfullcost})}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{interpolator}{p}{string}{Image interpolator to be used}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{passes}{P}{int}{Number of ICA+Registration passes to be run}
  \cmdopt{start-c-rate}{a}{float}{start coefficinet rate in spines, gets divided by \texttt{-{}-c-rate-divider} 
                                for each new registration pass}
  \cmdopt{c-rate-divider}{}{float}{cofficient rate divider for each pass}
  \cmdopt{start-divcurl}{d}{float}{start divcurl weight, gets divided by \texttt{-{}-divcurl-divider}
                                for each new registration pass}
  \cmdopt{ivcurl-divider}{}{float}{divcurl weight scaling with each new pass}
  }

  \item [Example:]Register the perfusion series given in segment.set by using automatic ICA estimation. 
        Skip two images at the beginning and otherwiese use the default parameters. 
	Store the result in registered.set. 
  \begin{lstlisting}
mia-3dmotioncompica-nonrigid  -i segment.set -o registered.set -k 2
  \end{lstlisting}
  \item [See also:] \sa{mia-3dmyomilles}, \sa{mia-3dmyoperiodic-nonrigid}, 
                    \sa{mia-3dmyoserial-nonrigid}, \sa{mia-3dmyopgt-nonrigid},
		    \sa{mia-3dsegseriesstats}
  \end{description}
  
  LatexEnd
*/


#define VSTREAM_DOMAIN "3dmotioncompica"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/core/filetools.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/3d/ica.hh>
#include <mia/3d/transformfactory.hh>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
using namespace tbb;


using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const char *g_general_help = 
	"This program runs the non-rigid registration of a 3D perfusion image series.\n"
	"Basic usage: \n"
	" mia-3dmotioncompica-nonrigid [options] "; 


typedef vector<P3DImage> C3DImageSeries; 
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
	PMinimizer minimizer; 
	C3DInterpolatorFactory& ipfactory; 
	size_t mg_levels; 
	double divcurlweight; 
	P3DTransformationFactory transform_creator; 
	string imagecostbase; 
	int skip_images; 
	
	SeriesRegistration(C3DImageSeries&  _input_images, 
			   const C3DImageSeries& _references, 
			   PMinimizer _minimizer, 
			   C3DInterpolatorFactory& _ipfactory, 
			   size_t _mg_levels, 
			   double _divcurlweight, 
			   P3DTransformationFactory _transform_creator, 
			   string _imagecostbase, 
			   int _skip_images):
		input_images(_input_images), 
		references(_references), 
		minimizer(_minimizer), 
		ipfactory(_ipfactory), 
		mg_levels(_mg_levels), 
		divcurlweight(_divcurlweight), 
		transform_creator(_transform_creator), 
		imagecostbase(_imagecostbase), 
		skip_images(_skip_images)
		{
		}
	void operator()( const blocked_range<int>& range ) const {
		for( int i=range.begin(); i!=range.end(); ++i ) {
			cout << "register " << i << "\n"; 
			auto costs  = create_costs(divcurlweight, imagecostbase, i); 
			C3DNonrigidRegister nrr(costs, minimizer,  transform_creator, ipfactory, mg_levels, i);
			P3DTransformation transform = nrr.run(input_images[i + skip_images], references[i]);
			input_images[i + skip_images] = (*transform)(*input_images[i + skip_images], ipfactory);
		}
	}
};  

void run_registration_pass(C3DImageSeries& input_images, const C3DImageSeries& references,  
			   int skip_images,  PMinimizer minimizer, 
			   C3DInterpolatorFactory& ipfactory, 
			   size_t mg_levels, double c_rate, double divcurlweight, 
			   const string&   imagecost) 
{
	SeriesRegistration sreg(input_images,references, minimizer, ipfactory, 
				mg_levels, divcurlweight, create_transform_creator(c_rate), 
				imagecost, skip_images); 
	blocked_range<int> range( 0, references.size(), 1 ); 
	sreg(range); 
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

int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg%04d.vista");
	string save_mixing_matrix; 

	string save_ref_filename;
	string save_reg_filename;

	// registration parameters
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1");
	string imagecost = "image:weight=1,cost=ssd,";
	double c_rate = 32; 
	double c_rate_divider = 4; 
	double divcurlweight = 20.0; 
	double divcurlweight_divider = 4.0; 

	EInterpolation interpolator = ip_bspline3;
	size_t mg_levels = 3; 

	// ICA parameters 
	size_t components = 0;
	bool no_normalize = false; 
	bool no_meanstrip = false; 
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 

	size_t current_pass = 0; 
	size_t pass = 3; 

	CCmdOptionList options(g_general_help);
	
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "registered", 'r', 
				    "file name base for registered fiels")); 
	
	options.add(make_opt( save_ref_filename, "save-refs", 0, 
				    "save reference images", NULL)); 
	options.add(make_opt( save_reg_filename, "save-regs", 0, 
				    "save intermediate registered images", NULL)); 

	options.add(make_opt(save_mixing_matrix, "save-mix-matrix", 0, 
			     "save mixing matrix", NULL)); 
	
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
	options.add(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( pass, "passes", 'P', "registration passes")); 

	options.set_group("ICA"); 
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation", NULL));
	options.add(make_opt( no_normalize, "no-normalize", 0, "don't normalized ICs", NULL));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves", NULL));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "e.g. because as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// this cost will always be used 

	unique_ptr<C3DInterpolatorFactory>   ipfactory(create_3dinterpolation_factory(interpolator));

	//task_scheduler_init init;

	
	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	C3DImageSeries input_images; 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P3DImage image = load_image<P3DImage>(src_name);
		if (!image)
			THROW(runtime_error, "image " << src_name << " not found");

		cvdebug() << "read '" << src_name << "\n";
		input_images.push_back(image);
	}

	cvmsg() << "skipping " << skip_images << " images\n"; 
	vector<C3DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
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
	
	C3DImageSeries references(references_float.size()); 
	transform(references_float.begin(), references_float.end(), references.begin(), C3DFImage2PImage()); 

	/*
	if (!save_mixing_matrix.empty()) 
		ica.save_coefs(save_mixing_matrix); 
	*/
	
	bool do_continue=true; 
	do {
		++current_pass; 
		cvmsg() << "Registration pass " << current_pass << "\n"; 

		if (!save_ref_filename.empty())
			save_references(save_ref_filename, current_pass, skip_images, references); 

		run_registration_pass(input_images, references,  skip_images,  minimizer, 
				      *ipfactory, mg_levels, c_rate, divcurlweight, imagecost); 
		
		if (!save_reg_filename.empty()) 
			save_references(save_reg_filename, current_pass, 0, input_images); 

		transform(input_images.begin() + skip_images, 
			  input_images.end(), series.begin(), Convert2Float()); 

		
		
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
			  references.begin(), C3DFImage2PImage()); 
		do_continue =  (current_pass < pass); 
	} while (do_continue); 


	bool success = true; 
	auto ii = input_images.begin(); 
	for (size_t i = start_filenum; i < end_filenum; ++i, ++ii) {
		string out_name = create_filename(registered_filebase.c_str(), i);
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

Convert2Float::Convert2Float()
{
	m_converter = C3DFilterPluginHandler::instance().produce("convert:repn=float,map=copy"); 
}

inline C3DFImage Convert2Float::operator () (P3DImage image) const
{
	auto res = m_converter->filter(*image); 
	
	return dynamic_cast<C3DFImage&>(*res); 
}
