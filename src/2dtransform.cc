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

#include <mia/core.hh>
#include <mia/2d.hh>

#include <mia/2d/transformio.hh>

NS_MIA_USE
using namespace boost;
using namespace std;


const SProgramDescription g_description = {
	{pdi_group, "Registration, Comparison, and Transformation of 2D images"}, 

	{pdi_short, "Transform a 2D image."}, 

	{pdi_description, "This program is used to deform a 2D image using a given transformation."}, 

	{pdi_example_descr, "Transform an image input.png by the transfromation stored in trans.v "
	 "by using linear interpolation and zero boundary conditions and store the result in output.png"}, 

	{pdi_example_code, "-i input.png -t trans.v  -o output.png  -p bspline:d=1 -b zero"},
	
};

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);
	string src_filename;
	string out_filename;
	string trans_filename;
	string interpolator_kernel;
	string interpolator_bc("mirror");

	const auto& imageio = C2DImageIOPluginHandler::instance();

	options.add(make_opt( src_filename, "in-file", 'i', "input image", CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "output image", CCmdOption::required, &imageio));
	options.add(make_opt( trans_filename, "transformation", 't', "transformation file name", 
			      CCmdOption::required, &C2DTransformationIOPluginHandler::instance()));

	options.add(make_opt( interpolator_kernel, "interpolator", 'p', "override the interpolator provided by the transformation"));
	options.add(make_opt( interpolator_bc, "boundary", 'b', "override the boundary conditions provided by the transformation."
			      " This is only used if the interpolator is overridden."));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto source    = imageio.load(src_filename);
	auto transformation   = C2DTransformationIOPluginHandler::instance().load(trans_filename);

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!transformation) {
		cerr << "no transformation field found in " << trans_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!interpolator_kernel.empty()) { 
		cvdebug() << "override the interpolator by '" 
			  << interpolator_kernel << "' and boundary conditions '" 
			  << interpolator_bc << "'\n"; 
		C2DInterpolatorFactory ipf(interpolator_kernel, interpolator_bc);
		transformation->set_interpolator_factory(ipf); 
	}
	
	for (auto i = source->begin();  i != source->end(); ++i)
		*i = (*transformation)(**i);
	
	if ( !imageio.save(out_filename, *source) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};
	return EXIT_SUCCESS;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
