/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define VSTREAM_DOMAIN "2dmilles"

#include <fstream>
#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp>
#include <itpp/signal/fastica.h>

#include <mia/core/tools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/rigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/SegSetWithImages.hh>
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


int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");
	string ref_filebase;

	// debug options: save some intermediate steps 
	string cropped_filename;
	string save_crop_feature; 

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
	
	size_t current_pass = 0; 
	size_t pass = 2; 

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "registered", 'r', "file name base for registered files")); 
	options.add(make_opt( ref_filebase, "save-references", 0, "save reference images to this file base")); 
	
	options.add(make_opt( cropped_filename, "save-cropped", 0, "save cropped image set to this file")); 
	options.add(make_opt( save_crop_feature, "save-feature", 0, "save segmentation feature images to png "
			      " with the given file name base ")); 

	options.add(make_opt( cost_function, "cost", 'c', "registration criterion")); 
	options.add(make_opt( minimizer, "gsl:opt=simplex,step=1.0", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( transform_creator, "rigid", "transForm", 'f', "transformation type"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));

	options.add(make_opt( pass, "passes", 'P', "registration passes")); 


	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation"));
	options.add(make_opt( normalize, "no-normalize", 0, "don't normalized ICs"));
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
	if (!ica.run(series)) {
		ica.set_approach(FICA_APPROACH_SYMM); 
		if (!ica.run(series) )
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
	
	// crop if requested
	if (box_scale) {
		C2DBounds crop_start; 
		auto cropper = ica.get_crop_filter(box_scale, crop_start, segmethod, save_crop_feature); 
		if (!cropper) {
			throw create_exception<runtime_error>( "Cropping was requested, but segmentation failed"); 
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
			throw create_exception<runtime_error>( "unable to save to '", cropped_filename, "'"); 

	}
	

	CSegSetWithImages::Frames& frames = input_set.get_frames();

	for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
		cvmsg() << "Register 1st pass, frame " << i << "\n"; 
		P2DTransformation transform = rigid_register.run(input_images[i + skip_images], references[i]);
		input_images[i + skip_images] = (*transform)(*input_images[i + skip_images]);
		P2DTransformation inverse(transform->invert()); 
		frames[i + skip_images].transform(*inverse);
	}

	// run the specified number of passes 
	// break early if ICA fails
	while (++current_pass < pass) {
		C2DPerfusionAnalysis ica2(components, normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
	
		transform(input_images.begin() + skip_images, 
			  input_images.end(), series.begin(), FCopy2DImageToFloatRepn()); 
		if (!ica2.run(series))
			ica2.set_approach(FICA_APPROACH_SYMM); 
		if (ica2.run(series) ) {
			references_float = ica2.get_references(); 

			transform(references_float.begin(), references_float.end(), 
				  references.begin(), FWrapStaticDataInSharedPointer<C2DImage>()); 

			if (!ref_filebase.empty())
				save_references(ref_filebase, current_pass, skip_images, references); 

			
			for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
				cvmsg() << "Register " << current_pass + 1 <<  " pass, frame " << i << "\n"; 
				P2DTransformation transform = rigid_register.run(input_images[i + skip_images] , 
										 references[i]); 
				input_images[i + skip_images] = (*transform)(*input_images[i + skip_images]);
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


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
