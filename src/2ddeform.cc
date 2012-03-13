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

#include <mia/2d/deformer.hh>
#include <mia/internal/main.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

typedef std::shared_ptr<C2DFVectorfield > P2DFVectorfield;

const SProgramDescription g_description = {
	"Registration, Comparison, and Transformation of 2D images", 

	"Transform a 2D image by means of a vector field.", 
	"This program is used to deform a 2D image usinga deformation vector field. "
	"Input image and deformation field must be of the same size. The transformation "
	"formula is 'x -> x - v(x)'", 

	"Deform image input.png by using the transformation given in transform.v and "
	"store the result in deformed.png using linear interpolation", 

	"-i input.png -t transform.v -o deformed.png -p bspline:d=1"
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);
	string src_filename;
	string out_filename;
	string vf_filename;
	PSplineKernel interpolator_kernel;

	options.add(make_opt( src_filename, "in-file", 'i', "input image", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "reference image", CCmdOption::required));
	options.add(make_opt( vf_filename, "transformation", 't', "transformation vector field", 
				    CCmdOption::required));
	options.add(make_opt( interpolator_kernel ,"bspline:d=3", "interpolator", 'p', "image interpolator kernel"));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	typedef C2DImageIOPluginHandler::Instance::PData PImageVector;
	typedef C2DVFIOPluginHandler::Instance::PData P2DVF;
	PImageVector source    = imageio.load(src_filename);
	P2DVF transformation   = C2DVFIOPluginHandler::instance().load(vf_filename);

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!transformation) {
		cerr << "no vector field found in " << vf_filename << "\n";
		return EXIT_FAILURE;
	}

	P2DInterpolatorFactory ipf(new C2DInterpolatorFactory(interpolator_kernel, "mirror"));

	FDeformer2D deformer(*transformation,*ipf);


	for (C2DImageIOPluginHandler::Instance::Data::iterator i = source->begin();
	     i != source->end(); ++i)
		*i = filter(deformer, **i);

	if ( !imageio.save(out_filename, *source) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};
	return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
