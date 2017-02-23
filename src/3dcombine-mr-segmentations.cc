/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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


#include <mia/3d/imageio.hh>
#include <mia/core/pixeltype.hh>
#include <mia/core/cmdlineparser.hh>


using namespace mia;
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Combine segmentations."}, 
	{pdi_description, "This programs takes various segmentation masks of the same data and "
	 "a tissue probability image set and combines them into one labeled image. "
	 "The masks are translated to labels according to the position on the command line. "
	 "If a voxel is set as belonging to more then one mask, the probabilities are consulted." 
   
	}
}; 

int do_main( int argc, char *argv[] )
{
	string in_filename;
	string out_filename;

	int label_offset = 1; 

	const auto& imageio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input probability images. The number of "
			      "mask images must correspond to the number of probability images given here.", 
			      CCmdOptionFlags::required_input, &imageio));

	
	options.add(make_opt( out_filename, "out-file", 'o', "output image that contains the labeling.", 
			      CCmdOptionFlags::required_output, &imageio));

	if (options.parse(argc, argv, "masks", &imageio) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto  in_image_list = imageio.load(in_filename);
	if (!in_image_list) 
		throw create_exception<runtime_error>("Unable to read 3D probability images from '", in_filename, "'");

	auto mask_image_files = options.get_remaining();

	if (mask_image_files.empty()) {
		throw invalid_argument("Nothing to do, since no mask image were given"); 
	}
	
	if (mask_image_files.size() != in_image_list->size()){
		throw create_exception<invalid_argument>("Expect the same number of mask images (got ",
							 mask_image_files.size(), ") like probability images (got ", 
							 in_image_list->size()); 
	}

	const int n_masks = mask_image_files.size(); 
		
	// load the mask images 
	vector<P3DImage> mask_pimages(mask_image_files.size()); 
	transform(mask_image_files.begin(), mask_image_files.end(), mask_pimages.begin(), 
		  [](const std::string&  filename){
			  auto img = load_image3d(filename); 
			  if (!img) 
				  throw create_exception<runtime_error>("Unable to load mask image '", filename, "'"); 
			  if (img->get_pixel_type() != it_bit) 
				  throw create_exception<invalid_argument>("Expected binary mask but got ", 
									   CPixelTypeDict.get_name(img->get_pixel_type()), 
									   " in '", filename, "'");
			  return img; 
		  });
	
	// here we know all mask-images are loaded and binary 
	vector<const C3DBitImage *> mask_images; 
	transform(mask_pimages.begin(), mask_pimages.end(), back_inserter(mask_images),
		  [](P3DImage img){return dynamic_cast<const C3DBitImage *>(img.get());}); 
	
	vector<C3DFImage*> probs; 

	transform(in_image_list->begin(), in_image_list->end(), back_inserter(probs), 
		  [](P3DImage image ){
			  if (image->get_pixel_type() != it_float)
				  throw create_exception<invalid_argument>("Probability images are expected to be of "
									   "type float but got ", 
									   CPixelTypeDict.get_name(image->get_pixel_type())); 
			  return dynamic_cast<C3DFImage*>(image.get()); 
		  }); 
	
	

	
	// now check the image sizes: 
	auto size = mask_images[0]->get_size(); 
	for (int i = 0; i < n_masks; ++i) {
		if (mask_images[i]->get_size() != size) 
			throw create_exception<invalid_argument>("Masks  are expected to be of the same size." , 
					       "Expected (", size, ") but got (", mask_images[i]->get_size(), 
					       ") for mask ", i+1); 
		if (probs[i]->get_size() != size) 
			throw create_exception<invalid_argument>("Probability images are expected to be of the same "
								 "size like the mask images." , 
								 "Expected (", size, ") but got (", mask_images[i]->get_size(), 
								 ") for probability image ", i+1);
	}

	C3DUBImage result(size, *probs[0]); 
	transform(mask_images[0]->begin(), mask_images[0]->end(), result.begin(), 
		  [label_offset](bool x){return x ? label_offset : 0; }); 
		
	C3DFImage& prob_shadow = *probs[0];
	++label_offset; 
	auto el = result.end(); 
	for (int i = 1; i < n_masks; ++i, ++label_offset) {
		auto ips = prob_shadow.begin(); 
		auto ip = probs[i]->begin(); 
		auto im = mask_images[i]->begin(); 
		auto ol = result.begin(); 
		while (ol != el) {
			if (*im) {
				// no label yet 
				if (!*ol || (*ip > *ips) ) {
					*ol = label_offset; 
					*ips = *ip; 
				} 
			}
			++ips; ++ip; ++im; ++ol; 
		}
	}
	return save_image(out_filename, result)? EXIT_SUCCESS : EXIT_FAILURE; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
