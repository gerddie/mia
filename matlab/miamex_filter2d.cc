/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <sstream>

#include "mextypemap.hh"

using namespace std; 
using namespace mia; 

static mxArray *filter2d(const mxArray *input, const C2DFilter& filter)
{
	P2DImage image = mex_get_image2d(input); 
	P2DImage pr = filter.filter(*image);  
	MxResultGetter mrg; 
	return mia::filter(mrg, *pr); 

}

void mex_filter2d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs < 1)
		throw invalid_argument("mex_filter2d: takes at least one argument"); 

	const char *filter_descr = mex_get_string(prhs[0]);
	
	cvdebug() << "Get plugin handler\n"; 
	const C2DFilterPluginHandler::Instance& ph = C2DFilterPluginHandler::instance();
	
	cvdebug() << "Create filter: '" << filter_descr <<"'\n"; 
	C2DFilterPlugin::ProductPtr f = ph.produce(filter_descr); 
	
	if (string("help") == string(filter_descr))
		return; 

	if (!f) {
		stringstream s; 
		s << "unable to create filter from  '"<< filter_descr << "'"; 
		throw invalid_argument(s.str()); 
	}
	
	if (nrhs != 2)
		throw invalid_argument("mex_filter2d: unknown number of input arguments"); 
	
	if (nlhs != 1)	
		throw invalid_argument("mex_filter2d: unknown number of output arguments"); 

	plhs[0] = filter2d(prhs[1], *f); 

	cvdebug() << "Filter done\n";
}
