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

// $Id: 2dimagefilter.cc,v 1.12 2006-07-12 13:44:23 wollny Exp $

/*! \brief mia-2dimagefilter

\sa mia-2dimagefilter.cc

\file mask.cc
\author G. Wollny, wollny eva.mpg.de, 2005
*/

#include <sstream>
#include <mia/core/fullstats.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/2dimageio.hh>


NS_MIA_USE;
using namespace std;

class CStatsEvaluator : public TFilter<CFullStats> {
public:
	template <typename T>
	CFullStats operator () (const T2DImage<T>& image) const {
		return CFullStats(image.begin(), image.end());
	}
};

const char *g_description = 
	"This progranm is used to evaluate some statistics of an image. " 
	"Output is Mean, Variation, Median, Min and Max of the intensity values."
	; 

int main( int argc, const char *argv[] )
{

	string in_filename;
	try {


		CCmdOptionList options(g_description);
		options.push_back(make_opt( in_filename, "in-file", 'i', "input image", CCmdOption::required));

		options.parse(argc, argv);


		// read image
		P2DImage image = load_image2d(in_filename);


		CFullStats stats = mia::filter(CStatsEvaluator(), *image);
		cout << stats << "\n";

		return EXIT_SUCCESS;

	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
