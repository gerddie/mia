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

#include <cstdio>
#include <idl_export.h>

enum IDX_nrreg {
	idx_nrreg_src  = 0, 
	idx_nrreg_ref,  
	idx_nrreg_model, 
	idx_nrreg_timestep, 
	idx_nrreg_cost, 
	idx_nrreg_epsilon,
	idx_nrreg_startsize, 
	idx_nrreg_maxiter, 
	idx_nrreg_numarg }; 

enum IDX_deform {
	idx_deform_src  = 0, 
	idx_deform_defo,  
	idx_deform_interp, 
	idx_deform_numarg }; 


IDL_VPTR reg2d_callback(int argc, IDL_VPTR *argv); 
IDL_VPTR deform2d_callback(int argc, IDL_VPTR *argv); 

IDL_VPTR reg3d_callback(int argc, IDL_VPTR *argv); 
IDL_VPTR deform3d_callback(int argc, IDL_VPTR *argv); 
