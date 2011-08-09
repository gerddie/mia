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

#define VSTREAM_DOMAIN ""

#include <sstream>
#include <cstring>
#include <stdexcept>

#include <mia/2d/register.hh>

#include "mextypemap.hh"

using namespace std; 
using namespace mia; 
using namespace boost; 

mxArray *mex_create_field2d(const C2DFVectorfield& field); 

void mex_reg2d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
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
		throw invalid_argument("mex_reg2d: unknown number of input arguments"); 
	
	if (nlhs != 1)	
		throw invalid_argument("mex_reg2d: unknown number of output arguments"); 

	const int startsize = (int)mxGetScalar(prhs[idx_startsize]); 
	const int maxiter = (int)mxGetScalar(prhs[idx_maxiter]); 
	const double epsilon = mxGetScalar(prhs[idx_epsilon]); 

	const char *model_descr = mex_get_string(prhs[idx_model]);
	const char *timestep_descr = mex_get_string(prhs[idx_timestep]);
	const char *cost_descr = mex_get_string(prhs[idx_cost]);
		
		
	P2DImageCost cost = C2DImageCostPluginHandler::instance().produce(cost_descr);
	P2DRegModel model = C2DRegModelPluginHandler::instance().produce(model_descr); 
	P2DRegTimeStep time_step = C2DRegTimeStepPluginHandler::instance().produce(timestep_descr); 
	auto_ptr<C2DInterpolatorFactory> ipf(create_2dinterpolation_factory(ip_bspline3)); 
	
	C2DImageRegister reg(startsize, *cost, maxiter, *model, *time_step, *ipf, epsilon, false, P2DImage());
		

	P2DImage src_image = mex_get_image2d(prhs[idx_src]);
	P2DImage ref_image = mex_get_image2d(prhs[idx_ref]);
	
	if (src_image->get_size() != ref_image->get_size()) {
		stringstream errmsg; 
		errmsg << "study and referenence image must be of the same size" <<
		    src_image->get_size() << " vs. " << ref_image->get_size(); 
		throw invalid_argument(errmsg.str()); 
	}
	P2DFVectorfield regfield = reg(*src_image, *ref_image);
	plhs[0] = mex_create_field2d(*regfield); 
	cvdebug() << "registration done\n";
}

static mxArray *mex_create_field2d_from_trans(const C2DTransformation& trans) 
{
	const C2DGridTransformation& gt = dynamic_cast<const C2DGridTransformation&>(trans); 

	const mwSize mxsize[3] = {2, gt.get_size().x, gt.get_size().y }; 
	const size_t n = gt.get_size().x * gt.get_size().y * 2; 
	mxArray * result = mxCreateNumericArray( 3, mxsize, mxSINGLE_CLASS, mxREAL );

//#warning need to see if this is supported by the C++ standard 
	memcpy(mxGetData(result), &gt.field_begin()->x, n * sizeof(float)); 
	return result; 
}

void mex_reg2d_multisource(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	enum IDX {
		idx_model  = 0, 
		idx_timestep, 
		idx_epsilon,
		idx_startsize, 
		idx_maxiter,
		idx_costs,
		idx_numfixargs 
	};

	if ( nrhs != idx_numfixargs )
		throw invalid_argument("mex_reg2d_multisource: unknown number of input arguments"); 

	if (nlhs != 1)	
		throw invalid_argument("mex_reg2d: unknown number of output arguments"); 


	const int startsize = (int)mxGetScalar(prhs[idx_startsize]);
	const int maxiter = (int)mxGetScalar(prhs[idx_maxiter]);
	const double epsilon = mxGetScalar(prhs[idx_epsilon]);

	const char *model_descr = mex_get_string(prhs[idx_model]);
	const char *timestep_descr = mex_get_string(prhs[idx_timestep]);

	P2DRegModel model = C2DRegModelPluginHandler::instance().produce(model_descr); 
	P2DRegTimeStep time_step = C2DRegTimeStepPluginHandler::instance().produce(timestep_descr); 
	P2DInterpolatorFactory ipf(create_2dinterpolation_factory(ip_bspline3)); 
	
	C2DImageFatCostList cost;
	
	int num_costs = mxGetNumberOfFields(prhs[idx_costs]);
	
	for (int i = 0; i < num_costs; ++i ) {

		const string plugin = string(mex_get_string(mxGetFieldByNumber(prhs[idx_costs], 0, i)));
		P2DImage src_image   = mex_get_image2d(mxGetFieldByNumber(prhs[idx_costs], 1, i));
		P2DImage ref_image   = mex_get_image2d(mxGetFieldByNumber(prhs[idx_costs], 2, i));
		double weight        = mxGetScalar(mxGetFieldByNumber(prhs[idx_costs], 3, i));

		P2DImageFatCost c = C2DFatImageCostPluginHandler::instance().create_directly(plugin, src_image, ref_image, weight); 

		if (!c) 
			throw invalid_argument(string("unknown cost function ") + plugin); 

		cost.push_back(c); 
	}

	C2DMultiImageRegister reg(startsize, maxiter, *model, *time_step, epsilon);
	P2DTransformation trans = reg(cost, ipf);

	plhs[0] = mex_create_field2d_from_trans(*trans); 
	cvdebug() << "registration done\n";
}

void mex_deform2d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	enum IDX {
		idx_src  = 0, 
		idx_defo,  
		idx_interp, 
		idx_numarg }; 
	
	if (nrhs != idx_numarg)
		throw invalid_argument("mex_deform2d: unknown number of input arguments"); 
	
	if (nlhs != 1)	
		throw invalid_argument("mex_deform2d: unknown number of output arguments"); 
	
	
	P2DImage src_image = mex_get_image2d(prhs[idx_src]);
	C2DFVectorfield field = mex_get_field2d(prhs[idx_defo]); 
	const char *interpolator = mex_get_string(prhs[idx_interp]); 

	auto_ptr<C2DInterpolatorFactory> ipf(create_2dinterpolation_factory(
						     GInterpolatorTable.get_value(interpolator))); 
	
	FDeformer2D deform(field, *ipf); 
	
	P2DImage result = mia::filter(deform, *src_image); 
	plhs[0] = mia::filter(MxResultGetter(), *result); 
}

