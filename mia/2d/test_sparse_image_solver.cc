/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/internal/autotest.hh>
#include <mia/2d/sparse_image_solver.hh>

NS_MIA_USE; 

class C2DTestSolverAmultxNoBoundary: public C2DImageSolverAmultx {
public: 
	C2DTestSolverAmultxNoBoundary(const C2DBounds& size); 
	
	virtual C2DImageSparseSolver::value_type operator () (C2DImageSparseSolver::const_field_iterator ix) const;
	
	virtual int get_boundary_size() const; 

}; 


BOOST_AUTO_TEST_CASE( test_mult_A_x )
{
	C2DBounds size(2,3); 
	C2DFImage x(size); 
	C2DFImage t(size); 
	int i = -2; 
	for (auto ix = x.begin(), it = t.begin(); ix != x.end(); ++ix, ++it, ++i) {
		*ix = i; 
		*it = 0.5 * i; 
	}
	C2DTestSolverAmultxNoBoundary A(size); 
	
	C2DFImage r = A * x; 

	BOOST_CHECK_EQUAL(r.get_size(), x.get_size()); 
	for (auto ir = r.begin(), it = t.begin(); ir != r.end(); ++ir, ++it) {
		BOOST_CHECK_EQUAL(*ir, *it); 
	}
}

C2DTestSolverAmultxNoBoundary::C2DTestSolverAmultxNoBoundary(const C2DBounds& size):
	C2DImageSolverAmultx(size) 
{
}
	
C2DImageSparseSolver::value_type 
C2DTestSolverAmultxNoBoundary::operator () (C2DImageSparseSolver::const_field_iterator ix) const
{
	return 0.5 * *ix; 
}
	
int C2DTestSolverAmultxNoBoundary::get_boundary_size() const
{
	return 0; 
}

