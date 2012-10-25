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

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/rgbimageio.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;


const SProgramDescription g_general_help = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
	{pdi_short, "combine gray scale images to an rgb image."}, 
	{pdi_description, "This program combines up to three gray scale image to a "
	 "three channel RGB image with eight bit per color channel. The input images "
	 "must be gray scale of eight bit colordepth. If at input one channel is not "
	 "given, it will be set to zero. at least one input channel must be given."},
	{pdi_example_descr, "Combine the images red.png, blue.tiff, and green.bmp to "
	 "the output image rgb.jpg."}, 
	{pdi_example_code, "-r red.png -b blue.tiff -g green.bmp -o rgb.jpg"}
};

static void get_size(const C2DImage *img, C2DBounds& size) 
{
	if (img) {
		if ((size.x == 0) && (size.y == 0)) 
			size = img->get_size(); 
		else 
			if ((size.x != img->get_size().x) || (size.y != img->get_size().y)) 
				throw create_exception<invalid_argument>("Input images must be of the same size, but I got ", 
									size, " vs. ",  img->get_size()); 
	}
}

const C2DUBImage& cast_or_zero(P2DImage img, const C2DUBImage& zero, const char * const channel)
{
	if (img) {
		if (img->get_pixel_type() != it_ubyte)
			throw create_exception<invalid_argument>("Input image for channel '", 
								 channel, "' is not of type 'unsigned byte'"); 
		return dynamic_cast<const C2DUBImage&>(*img); 
	}else 
		return zero; 
}

CRGB2DImage combine_channels(const C2DUBImage& red_ub, const C2DUBImage& green_ub, const C2DUBImage& blue_ub)
{
	CRGB2DImage result(red_ub.get_size()); 

	auto p = result.pixel(); 
	for (auto r = red_ub.begin(), g = green_ub.begin(), b = blue_ub.begin(); 
	     r != red_ub.end(); ++r, ++g, ++b) {
		*p++ = *r;  
		*p++ = *g;  
		*p++ = *b;  
	}
	return result; 
}

int do_main( int argc, char *argv[] )
{

	string red_filename;
	string blue_filename;
	string green_filename;
	string out_filename;

	const auto& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_general_help);
	options.add(make_opt( blue_filename, "blue", 'b', "input image for blue channel", 
			      CCmdOption::not_required, &imageio));
	options.add(make_opt( green_filename, "green", 'g', "input image for green channel", 
			      CCmdOption::not_required, &imageio));
	options.add(make_opt( red_filename, "red", 'r', "input image for red channel", 
			      CCmdOption::not_required, &imageio));

	options.add(make_opt( out_filename, "out-file", 'o', "combined output image", CCmdOption::required, &imageio));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS;

	P2DImage blue, green, red; 
	if (!blue_filename.empty()) 
		blue = load_image<P2DImage>(blue_filename); 

	if (!green_filename.empty()) 
		green = load_image<P2DImage>(green_filename); 

	if (!red_filename.empty()) 
		red = load_image<P2DImage>(red_filename); 
	
	if (!( red || green || blue)) 
		throw invalid_argument("No input available. At least one color channel must be given!");
	
	C2DBounds size; 
	get_size(red.get(), size); 
	get_size(green.get(), size); 
	get_size(blue.get(), size);

	const C2DUBImage zero(size); 
	const C2DUBImage& red_ub = cast_or_zero(red, zero, "red"); 
	const C2DUBImage& green_ub = cast_or_zero(green, zero, "green"); 
	const C2DUBImage& blue_ub = cast_or_zero(blue, zero, "blue");

	auto output = combine_channels(red_ub, green_ub, blue_ub); 

	return save_image(out_filename, output) ? EXIT_SUCCESS : EXIT_FAILURE; 
	
}; 

MIA_MAIN(do_main); 
