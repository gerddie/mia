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
#include <mia/3d.hh>

#include <mia/3d/deformer.hh>

NS_MIA_USE
using namespace std;
using namespace boost;

const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 3D images"}, 
	{pdi_short, "Transform a 3D image by means of a vector field."}, 
	{pdi_description, "Transform a 3D image by applying a given 3D transformation that is defined "
	"by a 3D vector field v according to x:=x-v(x)"}, 
	{pdi_example_descr, "Transform an image input.v by the transformation defined by the vector field field.v "
        "by using bspline interpolation of degree 4 and store the result in output.v"}, 
	{pdi_example_code, "-i input.v -t field.v  -o output.v  -p bspline:d=4" }

}; 

typedef std::shared_ptr<C3DFVectorfield > P3DFVectorfield;


int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

	string src_filename;
	string out_filename;
	string vf_filename;
	PSplineKernel interpolator_kernel;

	const auto& imageio = C3DImageIOPluginHandler::instance();
	const auto& vfioh = C3DVFIOPluginHandler::instance();

	options.add(make_opt( src_filename, "in-image", 'i', "input image", CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-image", 'o', "transformed image", CCmdOption::required, &imageio));
	options.add(make_opt( vf_filename, "transformation", 't', "transformation vector field", 
			      CCmdOption::required, &vfioh));
	options.add(make_opt( interpolator_kernel, "bspline:d=3", "interpolator", 'p', "image interpolator kernel"));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	


	auto source    = imageio.load(src_filename);
	auto transformation   = vfioh.load(vf_filename);

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!transformation) {
		cerr << "no vector field found in " << vf_filename << "\n";
		return EXIT_FAILURE;
	}

	P3DInterpolatorFactory ipfactory(new C3DInterpolatorFactory(interpolator_kernel, "mirror"));

	FDeformer3D deformer(*transformation,*ipfactory);


	for (auto i = source->begin();  i != source->end(); ++i)
		*i = filter(deformer, **i);

	if ( !imageio.save(out_filename, *source) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};
	return EXIT_SUCCESS;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
