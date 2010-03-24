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


/*
  This plug-in implements the navier-stokes operator like registration model 
  that accounts for linear elastic and fluid dynamic registration. 
  Which model is used depends on the selected time step. 
  
  To solve the PDE a full SOR is implemented.  
*/

#include <limits>

#include <mia/2d/model.hh>


NS_BEGIN(identity_regmodel)

class C2DIdentityRegModel: public mia::C2DRegModel {
private: 	
	virtual void do_solve(const mia::C2DFVectorfield& b, mia::C2DFVectorfield& v) const;
}; 

NS_END
