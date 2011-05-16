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
  
  \subsection{mia-2dmyomilles}
  \label{mia-2dmyomilles}

  \begin{description} 
  \item [Description:] 
        This program is used to run a modified version of the ICA based rigid registration approach 
        described in Milles et al. \cite{MvdG+08}. Changes include the extraction 
	of the quasi-periodic movement in free breathingly acquired data sets and the option to run 
	affine registration instead of the optimization of translations only.
	The program takes a segmentation set as input and writes the registered image to the given output 
         and corrects the segmentation according to the obtained registration. 

  The program is called like 
  \begin{lstlisting}
mia-2dmyomilles -i <input set> -o <output set> [options]
  \end{lstlisting}


  \item [Options:] $\:$

  \optiontable{
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{registered}{r}{string}{File name base for the registered images. Image type and numbering 
                                 scheme are taken from the input images.}
  \cmdopt{skip}{k}{int}{Skip a number of frames at the beginning of the series}
  \cmdopt{save-crop}{}{string}{If given, save the images that result from the intermediate segmentation step}
  \cmdopt{save-feature}{}{string}{If given, save the feature images from the successfull ICA.}
  \cmdopt{cost}{c}{string}{Cost function as provided by the \hyperref[sec:cost2d]{cost plug-ins}}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{transForm}{f}{string}{Transformation space as provided by the 
                                \hyperref[sec:2dtransforms]{transformation plug-ins.}}
  \cmdopt{interpolator}{p}{string}{Image interpolator to be used}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{passes}{P}{int}{Number of ICA+Registration passes to be run}
  \cmdopt{components}{C}{int}{Number of  ICA components to be used, 0 = automatic estimation}
  \cmdopt{no-normalize}{}{}{don't normalized ICs}
  \cmdopt{no-meanstrip}{}{}{don't strip the mean from the mixing curves}
  \cmdopt{guess}{g}{}{use initial guess for myocardial perfusion (experimental)}
  \cmdopt{segscale}{s}{float}{segment and scale the crop box around the LV (0=no segmentation)}
  \cmdopt{skip}{k}{int}{skip images at the beginning of the series (e.g. because they are of other modalities)}
  \cmdopt{max-ica-iter}{m}{int}{maximum number of iterations within ICA}
  \cmdopt{segmethod}{E}{string}{Segmentation method - (delta-feature|delta-peak|features)}
  }

  \item [Example:]Register the perfusion series given in segment.set by using automatic ICA estimation and 
       affine registration. Run 3 passes and use SSD as cost function.
       Store the result in affine.set. 
  \begin{lstlisting}
mia-2dmyomilles  -i segment.set -o affine.set -F affine -P 3 
  \end{lstlisting}
  \item [Remark:] It is not tested whether the original method of Milles et al. \cite{MvdG+08} 
                  would work. It should with "-C 3" and input images that do not stem from 
                  free-breathingly aquired images. 
  \item [See also:] \sa{mia-2dmyoica-nonrigid}, \sa{mia-2dmyoperiodic-nonrigid}, 
                    \sa{mia-2dmyoserial-nonrigid}, \sa{mia-2dmyopgt-nonrigid},
		    \sa{mia-2dsegseriesstats}

  \end{description}
  
  LatexEnd
*/


#define VSTREAM_DOMAIN "2dmilles"

#include <fstream>
#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp>
#include <itpp/signal/fastica.h>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/rigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/core/factorycmdlineoption.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 


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
	FConvert2DImage2float m_converter; 
}; 


const char *g_description = 
	"This program is use to run a modified version of the ICA based rigid registration approach "
	"described in Milles et al. 'Fully Automated Motion Correction in First-Pass Myocardial Perfusion "
	"MR Image Sequences', Trans. Med. Imaging., 27(11), 1611-1621, 2008. Changes include the extraction " 
	"of the quasi-periodic movement in free breathingly acquired data sets and the option to run "
	"affine registration instead of the optimization of translations only." 
	;

int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");

	// debug options: save some intermediate steps 
	string cropped_filename;
	string save_crop_feature; 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;
	
	// registration parameters
	string cost_function("ssd"); 
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=simplex,step=1.0");
	auto transform_creator = C2DTransformCreatorHandler::instance().produce("rigid"); 
	EInterpolation interpolator = ip_bspline3;
	size_t mg_levels = 3; 
	
	// ICA parameters 
	size_t components = 0;
	bool no_normalize = false; 
	bool no_meanstrip = false; 
	bool use_guess_model = false; 
	float box_scale = 1.4;
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 
	C2DPerfusionAnalysis::EBoxSegmentation segmethod=C2DPerfusionAnalysis::bs_features; 
	
	size_t current_pass = 0; 
	size_t pass = 2; 
	
	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "registered", 'r', "file name base for registered fiels")); 
	
	options.add(make_opt( cropped_filename, "save-cropped", 0, "save cropped set to this file", NULL)); 
	options.add(make_opt( save_crop_feature, "save-feature", 0, "save segmentation feature images", NULL)); 

	options.add(make_opt( cost_function, "cost", 'c', "registration criterion")); 
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( transform_creator, "transForm", 'f', "transformation type"));
	options.add(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));

	options.add(make_opt( pass, "passes", 'P', "registration passes", "passes")); 


	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation", NULL));
	options.add(make_opt( no_normalize, "no-normalize", 0, "don't normalized ICs", NULL));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves", NULL));
	options.add(make_opt( use_guess_model, "guess", 'g', "use initial guess for myocardial perfusion", 
				    NULL)); 
	options.add(make_opt( box_scale, "segscale", 's', 
				    "segment and scale the crop box around the LV (0=no segmentation)"));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	options.add(make_opt(segmethod , C2DPerfusionAnalysis::segmethod_dict, "segmethod", 'E', 
				   "Segmentation method")); 
				    

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// prepare registration class
	
	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(interpolator));
	C2DRigidRegister rigid_register(C2DImageCostPluginHandler::instance().produce("ssd"), 
					minimizer, transform_creator, *ipfactory, mg_levels); 
	
	cvwarn() << "save_crop_feature:" << save_crop_feature << "\n"; 
	
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
	if (use_guess_model) 
		ica.set_use_guess_model(); 
	if (!ica.run(series)) 
		ica.set_approach(FICA_APPROACH_SYMM); 
	if (!ica.run(series) )
		cvwarn() << "ICA analysis didn't converge, results might by bougus";
	
	input_set.set_RV_peak(ica.get_RV_peak_idx()); 
	input_set.set_LV_peak(ica.get_LV_peak_idx());

	vector<C2DFImage> references_float = ica.get_references();
	
	C2DImageSeries references(references_float.size()); 
	transform(references_float.begin(), references_float.end(), references.begin(), C2DFImage2PImage()); 

	// crop if requested
	if (box_scale) {
		C2DBounds crop_start; 
		auto cropper = ica.get_crop_filter(box_scale, crop_start, segmethod, save_crop_feature); 
		if (!cropper) {
			THROW(runtime_error, "Cropping was requested, but segmentation failed"); 
		}
		
		for(auto i = input_images.begin(); i != input_images.end(); ++i)
			*i = cropper->filter(**i); 

		for (auto i = references.begin(); i != references.end(); ++i) 
			*i = cropper->filter(**i); 

		auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
		P2DTransformation shift = tr_creator->create(C2DBounds(1,1)); 
		auto p = shift->get_parameters(); 
		p[0] = (float)crop_start.x; 
		p[1] = (float)crop_start.y; 
		shift->set_parameters(p); 
		
		input_set.transform(*shift);
		input_set.set_images(input_images);  
	}
	
	if (!cropped_filename.empty()) {
		bfs::path cf(cropped_filename);
		cf.replace_extension(); 
		input_set.rename_base(__bfs_get_filename(cf)); 
		input_set.save_images(cropped_filename);

		unique_ptr<xmlpp::Document> test_cropset(input_set.write());
		ofstream outfile(cropped_filename, ios_base::out );
		if (outfile.good())
			outfile << test_cropset->write_to_string_formatted();
		else 
			THROW(runtime_error, "unable to save to '" << cropped_filename << "'"); 

	}
	

	CSegSetWithImages::Frames& frames = input_set.get_frames();

	for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
		cvmsg() << "Register 1st pass, frame " << i << "\n"; 
		P2DTransformation transform = rigid_register.run(input_images[i + skip_images], references[i]);
		input_images[i + skip_images] = (*transform)(*input_images[i + skip_images], *ipfactory);
		P2DTransformation inverse(transform->invert()); 
		frames[i + skip_images].transform(*inverse);
	}

	// run the specified number of passes 
	// break early if ICA fails
	while (++current_pass < pass) {
		C2DPerfusionAnalysis ica2(components, !no_normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
	
		transform(input_images.begin() + skip_images, 
			  input_images.end(), series.begin(), Convert2Float()); 
		if (!ica2.run(series))
			ica2.set_approach(FICA_APPROACH_SYMM); 
		if (ica2.run(series) ) {
			references_float = ica2.get_references(); 
			transform(references_float.begin(), references_float.end(), 
				  references.begin(), C2DFImage2PImage()); 
			
			for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
				cvmsg() << "Register " << current_pass + 1 <<  " pass, frame " << i << "\n"; 
				P2DTransformation transform = rigid_register.run(input_images[i + skip_images] , 
										 references[i]); 
				input_images[i + skip_images] = (*transform)(*input_images[i + skip_images], *ipfactory);
				P2DTransformation inverse(transform->invert()); 
				frames[i + skip_images].transform(*inverse);
			}
		} else {
			cvmsg() << "Stopping registration in pass " << current_pass 
				<< " because ICA didn't return useful results\n"; 
			break; 
		}
	}


	input_set.set_images(input_images); 
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
	return ::mia::filter(m_converter, *image); 
}
