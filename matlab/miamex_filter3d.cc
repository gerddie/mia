/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <sstream>
#include "mextypemap.hh"

using namespace std; 
using namespace mia; 

static mxArray *filter3d(const mxArray *input, const C3DFilter& filter)
{
	P3DImage image = mex_get_image3d(input); 
	P3DImage pr = filter.filter(*image);  
	MxResultGetter mrg; 
	return mia::filter(mrg, *pr); 

}

void mex_filter3d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs < 1)
		throw invalid_argument("mex_filter3d: takes at least one argument"); 

	const char *filter_descr = mex_get_string(prhs[0]);
	
	cvdebug() << "Get plugin handler\n"; 
	const C3DFilterPluginHandler::Instance& ph = C3DFilterPluginHandler::instance();
	
	cvdebug() << "Create filter: '" << filter_descr <<"'\n"; 
	C3DFilterPlugin::ProductPtr f = ph.produce(filter_descr); 
	
	if (string("help") == string(filter_descr))
		return; 

	if (!f) {
		stringstream s; 
		s << "unable to create filter from  '"<< filter_descr << "'"; 
		throw invalid_argument(s.str()); 
	}
	
	if (nrhs != 2)
		throw invalid_argument("mex_filter3d: unknown number of input arguments"); 
	
	if (nlhs != 1)	
		throw invalid_argument("mex_filter3d: unknown number of output arguments"); 

	plhs[0] = filter3d(prhs[1], *f); 

	cvdebug() << "Filter done\n";
}
