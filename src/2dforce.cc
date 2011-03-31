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
#include <mia/2d/cost.hh>
#include <mia/2d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;


const char *g_description = 
	"This program is used to evaluate a registration force norm image between two images."
	;

typedef std::shared_ptr<C2DFVectorfield > P2DFVectorfield;

struct FVectorNorm {

	FVectorNorm():_M_max_norm(0.0f) {
	}

	float operator ()(const C2DFVector& v) {
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
	CCmdOptionList options(g_description);
	string src_filename;
	string out_filename;
	string ref_filename;
	string cost_descr("ssd");

	options.push_back(make_opt( src_filename, "src-file", 'i', "input image", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output force norm image", CCmdOption::required));
	options.push_back(make_opt( ref_filename, "ref-file", 'r', "reference image", CCmdOption::required));
	options.push_back(make_opt( cost_descr, "cost", 'c', "cost function to use", CCmdOption::required));


	options.parse(argc, args);

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	typedef C2DImageIOPluginHandler::Instance::PData PImageVector;

	P2DImageCost cost = C2DImageCostPluginHandler::instance().produce(cost_descr.c_str());
	PImageVector source    = imageio.load(src_filename);
	PImageVector ref    = imageio.load(ref_filename);


	if (!source || source->empty()) {
		throw invalid_argument(string("no image found in ") + src_filename);
	}
	if (!ref || ref->empty())
		throw invalid_argument(string("no image found in ") + ref_filename);

	C2DFVectorfield forcefield((*source->begin())->get_size());
	cost->set_reference(**ref->begin()); 
	cvmsg() << "Cost = " << cost->evaluate_force(**source->begin(), 1.0, forcefield) << "\n"; 

	C2DFImage *presult = new C2DFImage(forcefield.get_size());
	P2DImage result(presult);

	FVectorNorm vnorm;
	transform(forcefield.begin(), forcefield.end(), presult->begin(), vnorm);

	cvmsg() << "max norm: " << vnorm.get_max() << "\n";

	C2DImageIOPluginHandler::Instance::Data vout;
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
