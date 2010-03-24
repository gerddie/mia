/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/2d/register.hh>
#include <idl/helpers.hh>
#include <idl/reg2d.hh>

using namespace std; 
using namespace mia; 
using namespace boost; 


IDL_VPTR reg2d_callback(int argc, IDL_VPTR *argv)
{

	if (argc != idx_nrreg_numarg)
		throw invalid_argument("reg2d: unknown number of input arguments"); 
	

	const int startsize = idl_get_int(argv[idx_nrreg_startsize]); 
	const int maxiter = idl_get_int(argv[idx_nrreg_maxiter]); 
	const double epsilon = idl_get_double(argv[idx_nrreg_epsilon]); 

	const string model_descr = idl_get_string("model", argv[idx_nrreg_model]);
	const string timestep_descr = idl_get_string("timestep", argv[idx_nrreg_timestep]);
	const string cost_descr = idl_get_string("cost", argv[idx_nrreg_cost]);
		
		
	P2DImageCost cost = C2DImageCostPluginHandler::instance().produce(cost_descr.c_str());
	P2DRegModel model = C2DRegModelPluginHandler::instance().produce(model_descr.c_str()); 
	P2DRegTimeStep time_step = C2DRegTimeStepPluginHandler::instance().produce(timestep_descr.c_str()); 
	auto_ptr<C2DInterpolatorFactory> ipf(create_2dinterpolation_factory(ip_bspline3)); 
	
	C2DImageRegister reg(startsize, *cost, maxiter, *model, *time_step, *ipf, epsilon, false, P2DImage());
		

	P2DImage src_image = idl2mia_image2d(argv[idx_nrreg_src]);
	P2DImage ref_image = idl2mia_image2d(argv[idx_nrreg_ref]);
	
	if (src_image->get_size() != ref_image->get_size()) {
		stringstream errmsg; 
		errmsg << "study and referenence image must be of the same size" <<
		    src_image->get_size() << " vs. " << ref_image->get_size(); 
		throw invalid_argument(errmsg.str()); 
	}
	P2DFVectorfield regfield = reg(*src_image, *ref_image);
	return mia2idl_field2d(*regfield); 
	cvdebug() << "registration done\n";
}
IDL_VPTR deform2d_callback(int argc, IDL_VPTR *argv)
{
	if (argc != idx_deform_numarg)
		throw invalid_argument("idl_deform2d: unknown number of input arguments"); 
	
	
	P2DImage src_image = idl2mia_image2d(argv[idx_deform_src]);
	C2DFVectorfield field = idl2mia_field2d(argv[idx_deform_defo]); 
	const string interpolator = idl_get_string(argv[idx_deform_interp]); 

	auto_ptr<C2DInterpolatorFactory> ipf(create_2dinterpolation_factory(GInterpolatorTable.get_value(interpolator.c_str()))); 
	
	FDeformer2D deform(field, *ipf); 
	
	P2DImage result = mia::filter(deform, *src_image); 
	return mia2idl_image2d(*result); 
}


IDL_VPTR reg3d_callback(int argc, IDL_VPTR *argv)
{

	if (argc != idx_nrreg_numarg)
		throw invalid_argument("reg3d: unknown number of input arguments"); 
	

	const int startsize = idl_get_int(argv[idx_nrreg_startsize]); 
	const int maxiter = idl_get_int(argv[idx_nrreg_maxiter]); 
	const double epsilon = idl_get_double(argv[idx_nrreg_epsilon]); 

	const string model_descr = idl_get_string(argv[idx_nrreg_model]);
	const string timestep_descr = idl_get_string(argv[idx_nrreg_timestep]);
	const string cost_descr = idl_get_string(argv[idx_nrreg_cost]);
		
		
	P3DImageCost cost = C3DImageCostPluginHandler::instance().produce(cost_descr.c_str());
	P3DRegModel model = C3DRegModelPluginHandler::instance().produce(model_descr.c_str()); 
	P3DRegTimeStep time_step = C3DRegTimeStepPluginHandler::instance().produce(timestep_descr.c_str()); 
	auto_ptr<C3DInterpolatorFactory> ipf(create_3dinterpolation_factory(ip_bspline3)); 
	
	C3DImageRegister reg(startsize, *cost, maxiter, *model, *time_step, *ipf, epsilon, false);
		

	P3DImage src_image = idl2mia_image3d(argv[idx_nrreg_src]);
	P3DImage ref_image = idl2mia_image3d(argv[idx_nrreg_ref]);
	
	if (src_image->get_size() != ref_image->get_size()) {
		stringstream errmsg; 
		errmsg << "study and referenence image must be of the same size" <<
		    src_image->get_size() << " vs. " << ref_image->get_size(); 
		throw invalid_argument(errmsg.str()); 
	}
	P3DFVectorfield regfield = reg(*src_image, *ref_image);
	cvdebug() << "registration done\n";
	return mia2idl_field3d(*regfield); 
}

IDL_VPTR deform3d_callback(int argc, IDL_VPTR *argv)
{
	if (argc != idx_deform_numarg)
		throw invalid_argument("idl_deform3d: unknown number of input arguments"); 
	
	
	P3DImage src_image = idl2mia_image3d(argv[idx_deform_src]);
	C3DFVectorfield field = idl2mia_field3d(argv[idx_deform_defo]); 
	const string interpolator = idl_get_string(argv[idx_deform_interp]); 

	auto_ptr<C3DInterpolatorFactory> ipf(create_3dinterpolation_factory(GInterpolatorTable.get_value(interpolator.c_str()))); 
	
	FDeformer3D deform(field, *ipf); 
	
	P3DImage result = mia::filter(deform, *src_image); 
	return mia2idl_image3d(*result); 
}
