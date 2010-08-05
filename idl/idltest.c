/* -*- mia-c++ -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#include <stdio.h>
#include <idl_export.h>

const char *program[] = {
	"MIA_SET_TEST",
	"dummy = MIA_DEBUG_LEVEL('message')", 
	"src_img = MIA_READ2DIMAGE('../testdata/circle.bmp')",
	"ref_img = MIA_READ2DIMAGE('../testdata/c.bmp')",
	"model='naviera'", 
	"timestep = 'fluid'", 
	"cost='ssd'", 
	"maxiter = 100", 
	"eps = 0.001", 
	"startsize = 16", 
	
	"field = MIA_NRREG2D(src_img, ref_img, model, timestep, cost, eps, startsize, maxiter)", 
	"PRINT, 'registration success'", 
	"defo = MIA_DEFORM2D(src_img, field, 'bspline3')", 
	"PRINT, 'deformation success'", 
}; 
	
int main(int argc, char **argv) 
{
	if (IDL_Init(0, &argc, argv)) {
		IDL_Execute(sizeof(program)/sizeof(char*), program); 
		IDL_Cleanup(IDL_FALSE); 
	}
	return -1; 
}
