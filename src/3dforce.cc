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

#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>
#include <mia/3d/cost.hh>
#include <mia/3d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

static const char *program_info = 
	"This program is used to create an image comprising the pixel-wise norm\n"
	"of the ggradient of a given cost function\n"
	"Usage:\n"
	"  mia-3dforce -i <input image> -t <reference image > -o <output> [options]\n";


struct FVectorNorm {

	FVectorNorm():_M_max_norm(0.0f) {
	}

	float operator ()(const C3DFVector& v) {
		float n = v.norm();
		if (_M_max_norm < n)
			_M_max_norm = n;
		return n;
	}
	float get_max()const {
		return _M_max_norm;
	}
private:
	float _M_max_norm;
};

int do_main(int argc, const char **args)
{
	CCmdOptionList options(program_info);
	string src_filename;
	string out_filename;
	string ref_filename;
	string cost_descr("ssd");

	options.push_back(make_opt( src_filename, "src-file", 'i', "input image", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', "reference image", CCmdOption::required));
	options.push_back(make_opt( ref_filename, "ref-file", 'r', "output force norm image", CCmdOption::required));
	options.push_back(make_opt( cost_descr, "cost", 'c', "cost function to use"));


	options.parse(argc, args);


	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();

	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;

	PImageVector source    = imageio.load(src_filename);
	PImageVector ref    = imageio.load(ref_filename);
	P3DImageCost cost = C3DImageCostPluginHandler::instance().produce(cost_descr.c_str());

	if (!source || source->empty()) {
		throw invalid_argument(string("no image found in ") + src_filename);
	}
	if (!ref || ref->empty())
		throw invalid_argument(string("no image found in ") + ref_filename);

	C3DFVectorfield forcefield((*source->begin())->get_size());
	cvmsg() << "Cost = " << cost->value( **source->begin(), **ref->begin()) << "\n";

	cost->evaluate_force(**source->begin(), **ref->begin(), 1.0, forcefield);

	C3DFImage *presult = new C3DFImage(forcefield.get_size());
	P3DImage result(presult);

	FVectorNorm vnorm;
	transform(forcefield.begin(), forcefield.end(), presult->begin(), vnorm);

	cvmsg() << "max norm: " << vnorm.get_max() << "\n";

	C3DImageIOPluginHandler::Instance::Data vout;
	vout.push_back(result);

	imageio.save("", out_filename, vout);

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
