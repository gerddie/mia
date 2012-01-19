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

#include <string>
#include <mia/2d.hh>
#include <mia/3d.hh>
#include <cstdio>
#include <idl_export.h>

#ifndef idl_helper_hh
#define idl_helper_hh

std::string idl_get_string(IDL_VPTR s); 
std::string idl_get_string(const std::string& name,  IDL_VPTR s); 


mia::P2DImage idl2mia_image2d(IDL_VPTR s); 
mia::P3DImage idl2mia_image3d(IDL_VPTR s); 
IDL_VPTR mia2idl_image2d(const mia::C2DImage& image); 
IDL_VPTR mia2idl_image3d(const mia::C3DImage& image); 
mia::C2DFVectorfield  idl2mia_field2d(IDL_VPTR s); 
IDL_VPTR  mia2idl_field2d(const mia::C2DFVectorfield& s); 
mia::C3DFVectorfield  idl2mia_field3d(IDL_VPTR s); 
IDL_VPTR  mia2idl_field3d(const mia::C3DFVectorfield& s); 

int  idl_get_int(IDL_VPTR s); 
double  idl_get_double(IDL_VPTR s); 

#endif
