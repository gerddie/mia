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

#define VSTREAM_DOMAIN "2dmyocard"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <boost/lambda/lambda.hpp>
#include <boost/filesystem.hpp>


//#include <mia/core/fft1d_r2c.hh>
#include <queue>
#include <libxml++/libxml++.h>


#include <mia/core.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/ica.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/transformfactory.hh>
NS_MIA_USE;
using namespace std; 

namespace bfs=boost::filesystem; 

const SProgramDescription g_description = {
        {pdi_group, "Tools for Myocardial Perfusion Analysis"}, 
	{pdi_short, "Run an ICA analysis on a series of 2D images"}, 
	{pdi_description, "This program is used to run a ICA on a series of myocardial perfusion "
	 "images given in a segmentation set in order to create sythetic references "
	 "that can be used for motion correction by image registration. "
	 "If the aim is to run a full motion compensation then it is better run "
	 "mia-2dmyoica-nonrigid, since this program is essentially the same without "
	 "the registration bits."}, 
	{pdi_example_descr, "Evaluate the synthetic references from set segment.set and save "
	 "them to refXXXX.??? by using five independend components, and skipping 2 images. "
	 "Per default a bounding box around the LV will be segmented and scaled by 1.4 and "
	 "the cropped images will be saved to cropXXXX.??? and a segmentation set cropped.set "
	 " is created. The image file type ??? is deducted from the input images in segment.set."}, 
	{pdi_example_code, "-i segment.set -r ref -o ref -k 2 -C 5"}
}; 

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string reference_filename; 

	// debug options: save some intermediate steps 
	string cropped_filename("cropped.set");
	string save_crop_feature; 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// ICA parameters 
	size_t components = 0;
	bool no_normalize = false; 
	bool no_meanstrip = false; 
	float box_scale = 1.4;
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 
	C2DPerfusionAnalysis::EBoxSegmentation segmethod=C2DPerfusionAnalysis::bs_features; 

	CCmdOptionList options(g_description);
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( reference_filename, "references", 'r', "File name base for the reference images. "
			      "Image type and numbering scheme are taken from the input images.")); 
	options.add(make_opt( cropped_filename, "save-cropped", 'c', "save cropped set of the original set to this file, "
			      "the image files will use the stem of the name as file name base")); 
	options.add(make_opt( save_crop_feature, "save-feature", 0, "save segmentation feature images and initial ICA mixing matrix")); 
	
	options.set_group("ICA");
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation"));
	options.add(make_opt( no_normalize, "no-normalize", 0, "don't normalized ICs"));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves"));
	options.add(make_opt( box_scale, "segscale", 's', 
				    "segment and scale the crop box around the LV (0=no segmentation)"));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 
	options.add(make_opt( segmethod , C2DPerfusionAnalysis::segmethod_dict, "segmethod", 'E', 
				    "Segmentation method")); 
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 
	
	vector<C2DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
		  series.begin(), FCopy2DImageToFloatRepn()); 
	

	// run ICA
	C2DPerfusionAnalysis ica(components, !no_normalize, !no_meanstrip); 
	if (max_ica_iterations) 
		ica.set_max_ica_iterations(max_ica_iterations); 
	

	if (!ica.run(series)) {
		// ICA + classifictaion failed, 
		// save the mixing matrix if requested 
		if (!save_crop_feature.empty()) 
			ica.save_coefs(save_crop_feature + ".txt");
		throw runtime_error("ICA + Classification failed"); 
	}
	vector<C2DFImage> references_float = ica.get_references(); 
	
	C2DImageSeries references(references_float.size() + skip_images); 
	transform(references_float.begin(), references_float.end(), 
		  references.begin() + skip_images, FWrapStaticDataInSharedPointer<C2DImage>()); 
	copy(input_images.begin(), input_images.begin() + skip_images, references.begin()); 

	// crop if requested
	if (box_scale > 0) {
		C2DBounds crop_start; 
		auto cropper = ica.get_crop_filter(box_scale, crop_start, segmethod, save_crop_feature); 
		if (!cropper)
			cropper = ica.get_crop_filter(box_scale, crop_start, 
						      C2DPerfusionAnalysis::bs_delta_feature, save_crop_feature); 
		if (cropper) {
		
			for(auto i = input_images.begin(); i != input_images.end(); ++i)
				*i = cropper->filter(**i); 
			
			for (auto i = references.begin(); i != references.end(); ++i) 
				*i = cropper->filter(**i); 
			
			auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
			P2DTransformation shift = tr_creator->create(C2DBounds(1,1)); 
			auto p = shift->get_parameters(); 
			p[0] = -(float)crop_start.x; 
			p[1] = -(float)crop_start.y; 
			shift->set_parameters(p); 
			
			input_set.transform(*shift);
			input_set.set_images(input_images);
			
		}else 
			cverr() << "no crop box created\n"; 
	}


	if (!cropped_filename.empty()) {
		bfs::path cf(cropped_filename);
		cf.replace_extension(); 
		input_set.rename_base(cf.string()); 
		input_set.save_images(cropped_filename);
		
		unique_ptr<xmlpp::Document> test_cropset(input_set.write());
		ofstream outfile(cropped_filename, ios_base::out );
		if (outfile.good())
			outfile << test_cropset->write_to_string_formatted();
		else 
			throw create_exception<runtime_error>( "unable to save to '", cropped_filename, "'"); 
	}
	
	//
	if (!reference_filename.empty()) {
		bfs::path reff(reference_filename);
		reff.replace_extension(); 
		input_set.set_images(references);  
		input_set.rename_base(__bfs_get_filename(reff)); 
		input_set.save_images(reference_filename);
		
		
		unique_ptr<xmlpp::Document> test_regset(input_set.write());
		ofstream outfile2(reference_filename, ios_base::out );
		if (outfile2.good())
			outfile2 << test_regset->write_to_string_formatted();
		else 
			throw create_exception<runtime_error>( "unable to save to '", cropped_filename, "'"); 
	}
	return EXIT_SUCCESS; 

};


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

