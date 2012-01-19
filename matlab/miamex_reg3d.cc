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

#include <sstream>
#include <cstring>
#include <stdexcept>

#include "mextypemap.hh"

using namespace std; 
using namespace mia; 
using namespace boost; 

mxArray *mex_create_field3d(const C3DFVectorfield& field); 

void mex_reg3d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	enum IDX {
		idx_src  = 0, 
		idx_ref,  
		idx_model, 
		idx_timestep, 
		idx_cost, 
		idx_epsilon,
		idx_startsize, 
		idx_maxiter, 
		idx_numarg }; 
	
	if (nrhs != idx_numarg)
		throw invalid_argument("mex_reg3d: unknown number of input arguments"); 
	
	if (nlhs != 1)	
		throw invalid_argument("mex_reg3d: unknown number of output arguments"); 

	

	const int startsize = (int)mxGetScalar(prhs[idx_startsize]); 
	const int maxiter = (int)mxGetScalar(prhs[idx_maxiter]); 
	const double epsilon = mxGetScalar(prhs[idx_epsilon]); 

	const char *model_descr = mex_get_string(prhs[idx_model]);
	const char *timestep_descr = mex_get_string(prhs[idx_timestep]);
	const char *cost_descr = mex_get_string(prhs[idx_cost]);
		
		
	P3DImageCost cost = C3DImageCostPluginHandler::instance().produce(cost_descr);
	P3DRegModel model = C3DRegModelPluginHandler::instance().produce(model_descr); 
	P3DRegTimeStep time_step = C3DRegTimeStepPluginHandler::instance().produce(timestep_descr); 
	auto_ptr<C3DInterpolatorFactory> ipf(create_3dinterpolation_factory(ip_bspline3)); 
	
	C3DImageRegister reg(startsize, *cost, maxiter, *model, *time_step, *ipf, epsilon, false);
		

	P3DImage src_image = mex_get_image3d(prhs[idx_src]);
	P3DImage ref_image = mex_get_image3d(prhs[idx_ref]);
	
	if (src_image->get_size() != ref_image->get_size())
		throw invalid_argument("study and referenence image must be of the same size"); 

	P3DFVectorfield regfield = reg(*src_image, *ref_image);
	plhs[0] = mex_create_field3d(*regfield); 
	
	cvdebug() << "registration done\n";
}

void mex_deform3d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	enum IDX {
		idx_src  = 0, 
		idx_defo,  
		idx_interp, 
		idx_numarg }; 
	
	if (nrhs != idx_numarg)
		throw invalid_argument("mex_deform3d: unknown number of input arguments"); 
	
	if (nlhs != 1)	
		throw invalid_argument("mex_deform3d: unknown number of output arguments"); 
	
	
	P3DImage src_image = mex_get_image3d(prhs[idx_src]);
	C3DFVectorfield field = mex_get_field3d(prhs[idx_defo]); 
	const char *interpolator = mex_get_string(prhs[idx_interp]); 

	auto_ptr<C3DInterpolatorFactory> ipf(create_3dinterpolation_factory(
						     GInterpolatorTable.get_value(interpolator))); 
	
	FDeformer3D deform(field, *ipf); 
	
	P3DImage result = mia::filter(deform, *src_image); 
	plhs[0] = mia::filter(MxResultGetter(), *result); 
}
