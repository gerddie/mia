/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <fstream>
#include <boost/filesystem.hpp>

#include <mia/core/tools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/ica.hh>
#include <mia/2d/rigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/segsetwithimages.hh>
#include <mia/2d/transformfactory.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const SProgramDescription g_description = {
        {pdi_group, "Registration of series of 2D images"}, 
	{pdi_short, "Run a registration of a series of 2D images."}, 
	{pdi_description, "This program is use to run a modified version of the ICA based registration approach "
	 "described in Milles et al. 'Fully Automated Motion Correction in First-Pass Myocardial Perfusion "
	 "MR Image Sequences', Trans. Med. Imaging., 27(11), 1611-1621, 2008. Changes include the extraction " 
	 "of the quasi-periodic movement in free breathingly acquired data sets and the option to run "
	 "affine or rigid registration instead of the optimization of translations only.\n"}, 
	{pdi_example_descr, "Register the perfusion series given in 'segment.set' by using "
	 "automatic ICA estimation. Skip two images at the beginning and otherwiese use the default parameters. "
	 "Store the result in 'registered.set'.\n"}, 
	{pdi_example_code, "  -i segment.set -o registered.set -k 2"}
}; 

void save_references(const string& save_ref, int current_pass, int skip_images, const C2DImageSeries& references)
{
	for(size_t i = 0 ; i < references.size(); ++i) {
		stringstream filename; 
		filename << save_ref << current_pass << "-" 
			 << setw(4) << setfill('0') << skip_images + i << ".v"; 
		save_image(filename.str(), references[i]);
	}
}


void run_registration_pass(const C2DRigidRegister& rigid_register, C2DImageSeries& input_images, const C2DImageSeries& references, 
			   CSegSetWithImages::Frames& frames, 
			   int skip_images, int global_reference) 
{
	P2DTransformation align_to_global; 
	for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
		cvmsg() << "Register 1st pass, frame " << i << "\n"; 
		
		if ((int) i == global_reference - skip_images) {
			align_to_global = rigid_register.run(references[i], input_images[i + skip_images]);
		}else {
			P2DTransformation transform = rigid_register.run(input_images[i + skip_images], references[i]);
			input_images[i + skip_images] = (*transform)(*input_images[i + skip_images]);
			P2DTransformation inverse(transform->invert()); 
			frames[i + skip_images].transform(*inverse);
		}
	}
	if (align_to_global) {
		P2DTransformation inverse(align_to_global->invert()); 
		for (size_t i = 0; i < input_images.size(); ++i) {
			input_images[i] = (*inverse)(*input_images[i]);
			frames[i].transform(*inverse);
		}
	}
}

class FInsertData : public TFilter< P2DImage >  {
public: 
	FInsertData(const C2DBounds& start, const C2DBounds& end): 
		m_start(start), m_end(end){}
	
	template <typename T>
	void operator () ( const T2DImage<T>& a, T2DImage<T>& b) const {
		copy(a.begin(), a.end(), b.begin_range(m_start,m_end));
	}
private: 
		
	C2DBounds m_start; 
	C2DBounds m_end; 
}; 

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase;
	string ref_filebase;

	// debug options: save some intermediate steps 
	string cropped_filename;
	string save_crop_feature; 

	int global_reference = -1; 
	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;
	
	// registration parameters
	string cost_function("ssd"); 
	PMinimizer minimizer;
	P2DTransformationFactory transform_creator; 
	size_t mg_levels = 3; 
	
	// ICA parameters 
	size_t components = 0;
	bool normalize = false; 
	bool no_meanstrip = false; 
	bool use_guess_model = false; 
	float box_scale = 1.4;
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 
	C2DPerfusionAnalysis::EBoxSegmentation segmethod=C2DPerfusionAnalysis::bs_features; 
	PIndepCompAnalysisFactory icatool;
	size_t current_pass = 0; 
	size_t pass = 2; 

	CCmdOptionList options(g_description);

	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', "output perfusion data set", CCmdOptionFlags::required_output));
	options.add(make_opt( registered_filebase, "registered", 'r', "file name base for registered files")); 
	options.add(make_opt( ref_filebase, "save-references", 0, "save synthetic reference images to this file base")); 
	
	options.add(make_opt( cropped_filename, "save-cropped", 0, "save cropped image set to this file")); 
	options.add(make_opt( save_crop_feature, "save-feature", 0, "save the features images resulting from the ICA and "
			      "some intermediate images used for the RV-LV segmentation with the given file name base to PNG files. "
			      "Also save the coefficients of the initial best and the final IC mixing matrix.")); 

	
	options.set_group("Registration"); 
	options.add(make_opt( cost_function, "cost", 'c', "registration criterion")); 
	options.add(make_opt( minimizer, "gsl:opt=simplex,step=1.0", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( transform_creator, "rigid", "transForm", 'f', "transformation type"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( global_reference, "reference", 'R', "Global reference all image should be aligned to. If set "
			      "to a non-negative value, the images will be aligned to this references, and the cropped output image date "
			      "will be injected into the original images. Leave at -1 if "
			      "you don't care. In this case all images with be registered to a mean position of the movement")); 
	options.add(make_opt( pass, "passes", 'P', "registration passes")); 


	options.set_group("ICA");
	options.add(make_opt( icatool, "internal", "fastica", 0, "FastICA implementationto be used"));
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation"));
	options.add(make_opt( normalize, "normalize", 0, "normalized ICs"));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves"));
	options.add(make_opt( use_guess_model, "guess", 'g', "use initial guess for myocardial perfusion")); 
	options.add(make_opt( box_scale, "segscale", 's', 
				    "segment and scale the crop box around the LV (0=no segmentation)"));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	options.add(make_opt(segmethod , C2DPerfusionAnalysis::segmethod_dict, "segmethod", 'E', 
				   "Segmentation method")); 
				    

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// prepare registration class
	C2DRigidRegister rigid_register(C2DImageCostPluginHandler::instance().produce("ssd"), 
					minimizer, transform_creator, mg_levels); 
	
	cvwarn() << "save_crop_feature:" << save_crop_feature << "\n"; 
	
	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 
	
	C2DImageSeries original_images; 
	if (global_reference >= 0) 
		original_images = input_set.get_images(); 

	cvmsg() << "skipping " << skip_images << " images\n"; 
	vector<C2DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
		  series.begin(), FCopy2DImageToFloatRepn()); 

	
	// run ICA
	C2DPerfusionAnalysis ica(components, normalize, !no_meanstrip); 
	if (max_ica_iterations) 
		ica.set_max_ica_iterations(max_ica_iterations); 
	if (use_guess_model) 
		ica.set_use_guess_model(); 

	if (!ica.run(series, *icatool)) {
		ica.set_approach(CIndepCompAnalysis::appr_symm);
		if (!ica.run(series, *icatool) )
			cvwarn() << "ICA analysis didn't converge, results might by bougus\n";
	}
	
	int RV_peak_idx = ica.get_RV_idx(); 
	int LV_peak_idx = ica.get_LV_idx(); 

	if (RV_peak_idx < 0 || LV_peak_idx < 0) 
		throw runtime_error("RV or LV identification failed"); 

	if( input_set.get_RV_peak() < 0) 
		input_set.set_RV_peak(ica.get_RV_peak_time() + skip_images); 
	if( input_set.get_LV_peak() < 0) 
		input_set.set_LV_peak(ica.get_LV_peak_time() + skip_images);



	vector<C2DFImage> references_float = ica.get_references();
	
	C2DImageSeries references(references_float.size()); 
	transform(references_float.begin(), references_float.end(), references.begin(), 
		  FWrapStaticDataInSharedPointer<C2DImage>() );

	C2DBounds crop_start; 	
	auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
	// crop if requested
	if (box_scale) {
		auto cropper = ica.get_crop_filter(box_scale, crop_start, segmethod, save_crop_feature); 
		if (!cropper) {
			throw create_exception<runtime_error>( "Cropping was requested, but segmentation failed"); 
		}
		
		for(auto i = input_images.begin(); i != input_images.end(); ++i)
			*i = cropper->filter(**i); 

		for (auto i = references.begin(); i != references.end(); ++i) 
			*i = cropper->filter(**i); 

		P2DTransformation shift = tr_creator->create(C2DBounds(1,1)); 
		auto p = shift->get_parameters(); 
		p[0] = (float)crop_start.x; 
		p[1] = (float)crop_start.y; 
		shift->set_parameters(p); 

		
		input_set.transform(*shift);
		input_set.set_images(input_images);  
	} else if (!save_crop_feature.empty()) {
		stringstream cfile; 
		cfile << save_crop_feature << "-coeff.txt"; 
		ica.save_coefs(cfile.str()); 
		ica.save_feature_images(save_crop_feature); 
	}
	
	if (!cropped_filename.empty()) {
		bfs::path cf(cropped_filename);
		cf.replace_extension(); 
		input_set.rename_base(cf.filename().string()); 
		input_set.save_images(cropped_filename);

		ofstream outfile(cropped_filename, ios_base::out );
		if (outfile.good())
			outfile << input_set.write().write_to_string();
		else 
			throw create_exception<runtime_error>( "unable to save to '", cropped_filename, "'"); 

	}
	

	CSegSetWithImages::Frames& frames = input_set.get_frames();


	run_registration_pass(rigid_register, input_images, references, frames, skip_images, global_reference); 

	// run the specified number of passes 
	// break early if ICA fails
	while (++current_pass < pass) {
		C2DPerfusionAnalysis ica2(components, normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
	
		transform(input_images.begin() + skip_images, 
			  input_images.end(), series.begin(), FCopy2DImageToFloatRepn()); 
        if (!ica2.run(series, *icatool))
            ica2.set_approach(CIndepCompAnalysis::appr_symm);
        if (ica2.run(series, *icatool) ) {
			references_float = ica2.get_references(); 

			transform(references_float.begin(), references_float.end(), 
				  references.begin(), FWrapStaticDataInSharedPointer<C2DImage>()); 

			if (!ref_filebase.empty())
				save_references(ref_filebase, current_pass, skip_images, references); 

			run_registration_pass(rigid_register, input_images, references, frames, skip_images, global_reference); 
			
		} else {
			cvmsg() << "Stopping registration in pass " << current_pass 
				<< " because ICA didn't return useful results\n"; 
			break; 
		}
	}

	// re-insert the registered sub-images if we have a global reference
	if (global_reference >= 0 && box_scale) {
		const FInsertData id(crop_start, crop_start + input_images[0]->get_size()); 
		transform(original_images.begin(), original_images.end(), input_images.begin(), 
			  original_images.begin(), 
			  [&id](P2DImage orig, P2DImage part) {
				  filter_equal_inplace(id, *part, *orig); 
				  return orig; 
			  }); 
		
		P2DTransformation shift = tr_creator->create(C2DBounds(1,1)); 
		auto p = shift->get_parameters(); 
		p[0] = -(float)crop_start.x; 
		p[1] = -(float)crop_start.y; 
		shift->set_parameters(p); 
		
		input_set.transform(*shift);
		input_set.set_images(original_images);  

	}else {
		input_set.set_images(input_images); 
	}
	
	if (!registered_filebase.empty()) 
		input_set.rename_base(registered_filebase); 
	
	input_set.save_images(out_filename); 
	
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << input_set.write().write_to_string();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;

}


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
