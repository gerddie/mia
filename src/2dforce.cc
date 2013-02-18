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


#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>
#include <mia/2d/cost.hh>
#include <mia/2d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 2D images"}, 
	{pdi_short, "Registering force between two 2D images."}, 
	{pdi_description, "This image evaluate the force field between two images "
	 "based on a given cost function."}, 
	{pdi_example_descr, "Evaluate the force between test.v and reference.v by "
	 "using SSD ans save the norm of the force to forcenorm.v"}, 
	{pdi_example_code, "-i test.v -r reference.v -c ssd -o forcenorm.v"}
}; 

typedef std::shared_ptr<C2DFVectorfield > P2DFVectorfield;

struct FVectorNorm {

	FVectorNorm():m_max_norm(0.0f) {
	}

	float operator ()(const C2DFVector& v) {
		float n = v.norm();
		if (m_max_norm < n)
			m_max_norm = n;
		return n;
	}
	float get_max()const {
		return m_max_norm;
	}
private:
	float m_max_norm;
};

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);
	string src_filename;
	string out_filename;
	string ref_filename;
	string cost_descr("ssd");

	const auto& imageio = C2DImageIOPluginHandler::instance();

	options.add(make_opt( src_filename, "src-file", 'i', "input image", CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "output force norm image", CCmdOption::required, &imageio));
	options.add(make_opt( ref_filename, "ref-file", 'r', "reference image", CCmdOption::required, &imageio));
	options.add(make_opt( cost_descr, "cost", 'c', "cost function to use", CCmdOption::required));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	


	auto  cost = C2DImageCostPluginHandler::instance().produce(cost_descr.c_str());
	auto  source    = imageio.load(src_filename);
	auto  ref    = imageio.load(ref_filename);


	if (!source || source->empty()) {
		throw invalid_argument(string("no image found in ") + src_filename);
	}
	if (!ref || ref->empty())
		throw invalid_argument(string("no image found in ") + ref_filename);

	C2DFVectorfield forcefield((*source->begin())->get_size());
	cost->set_reference(**ref->begin()); 
	cvmsg() << "Cost = " << cost->evaluate_force(**source->begin(), forcefield) << "\n"; 

	C2DFImage *presult = new C2DFImage(forcefield.get_size());
	P2DImage result(presult);

	FVectorNorm vnorm;
	transform(forcefield.begin(), forcefield.end(), presult->begin(), vnorm);

	cvmsg() << "max norm: " << vnorm.get_max() << "\n";

	if (!save_image(out_filename, result)) 
		throw create_exception<runtime_error>("Unable to save result to '", out_filename, "'"); 
	
	return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
