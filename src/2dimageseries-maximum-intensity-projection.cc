/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mia/core.hh>
#include <mia/core/revision.hh>
#include <mia/2d/imageio.hh>



NS_MIA_USE
using namespace std;
using namespace boost;

const SProgramDescription g_description = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
	{pdi_short, "Evaluate maximum per-pixel intensities of an image series."}, 
	{pdi_description, "This program is used to evaluate the per-pixel maximum intensity "
         "of an image series."}, 
}; 

struct C2DMax : public TFilter<bool> {

	C2DMax()
	{
	}

	template <typename T>
	bool operator ()(const T2DImage<T>& image, T2DImage<T>& inout) const 
	{
		if (image.get_size() != inout.get_size())
			throw invalid_argument("Input images differ in size");

                auto iio = inout.begin();
		auto ii = image.begin();
		auto ei = image.end();

		while (ii != ei) {
                        if (*iio < *ii) 
                                *iio = *ii; 
                        ++iio; 
                        ++ii; 
                }
                return true;
	}
};

int do_main( int argc, char *argv[] )
{

	string out_filename;

	const auto& image2dio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( out_filename, "out-file", 'o', "output image", CCmdOptionFlags::required_output, &image2dio));

	if (options.parse(argc, argv, "image") != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	if (options.get_remaining().empty())
		throw runtime_error("You must give the image to be combined as free parameters");

        if (options.get_remaining().size() < 2)
                throw runtime_error("You must give at least two images to combine");


	CHistory::instance().append(argv[0], LIBMIA_REVISION, options);
	char new_line = cverb.show_debug() ? '\n' : '\r';

	C2DMax max_op; 

        P2DImage result = load_image2d(options.get_remaining()[0]); 


	for (auto iname = options.get_remaining().begin() + 1; iname != options.get_remaining().end(); ++iname) {
                auto image = load_image2d(*iname); 
                cvmsg() << "Combine image '" << *iname << new_line; 
                mia::filter_equal_inplace(max_op, *image, *result); 
	}
	cvmsg() << "\n";

        save_image(out_filename, result); 

	return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
