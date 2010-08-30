/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <mia/core.hh>
#include <mia/3d.hh>

#include <mia/3d/deformer.hh>

static const char *program_info = 
	"This program is used to transform an 3D image by using a given transformation.\n"
	"Basic usage:\n"
	"  mia-3dimagefilter -i <input> -t <transformation> -o <output> [<plugin>]\n"; 


NS_MIA_USE
using namespace boost;
using namespace std;

typedef std::shared_ptr<C3DFVectorfield > P3DFVectorfield;


int do_main(int argc, const char **args)
{
	CCmdOptionList options(program_info);

	string src_filename;
	string out_filename;
	string vf_filename;
	EInterpolation interpolator = ip_bspline3;

	options.push_back(make_opt( src_filename, "in-file", 'i', "input image", "input", true));
	options.push_back(make_opt( out_filename, "out-file", 'o', "reference image", "output", true));
	options.push_back(make_opt( vf_filename, "transformation", 't', "transformation vector field", "transform", true));
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
					"image interpolator", "interp"));


	options.parse(argc, args);

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();
	const C3DVFIOPluginHandler::Instance&  vfioh = C3DVFIOPluginHandler::instance();

	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;
	typedef C3DVFIOPluginHandler::Instance::PData P3DVF;
	PImageVector source    = imageio.load(src_filename);
	P3DVF transformation   = vfioh.load(vf_filename);

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!transformation) {
		cerr << "no vector field found in " << vf_filename << "\n";
		return EXIT_FAILURE;
	}

	std::shared_ptr<C3DInterpolatorFactory > ipf(create_3dinterpolation_factory(interpolator));

	FDeformer3D deformer(*transformation,*ipf);


	for (C3DImageIOPluginHandler::Instance::Data::iterator i = source->begin();
	     i != source->end(); ++i)
		*i = filter(deformer, **i);

	if ( !imageio.save("", out_filename, *source) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};
	return EXIT_SUCCESS;
}


int main(int argc, const char **args)
{
	try {
		return do_main(argc, args);
	}
	catch (invalid_argument& err) {
		cerr << "invalid argument: " << err.what() << "\n";
	}
	catch (runtime_error& err) {
		cerr << "runtime error: " << err.what() << "\n";
	}
	catch (std::exception& err) {
		cerr << "exception: " << err.what() << "\n";
	}
	catch (...) {
		cerr << "unknown exception\n";
	}
	return EXIT_FAILURE;
}
