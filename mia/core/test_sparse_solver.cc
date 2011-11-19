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

#define BOOST_TEST_DYN_LINK

#include <mia/internal/autotest.hh>
#include <mia/core/sparse_solver.hh>

/**
   This test only tests the general idea of the unified solver class, and 
   is a blue-print of how such a thing could be implemented 
*/

/**
   a simple fix-point iterator
*/
NS_MIA_USE; 
using namespace std; 

class CSolverMock: public TSparseSolver<vector<double> > {
public: 
	int solve(const Field& rhs, Field& x, const CSolverMock::A_mult_x& mult) const; 
}; 

class CAMultX : public CSolverMock::A_mult_x {
public: 
	CSolverMock::value_type operator ()(CSolverMock::const_field_iterator ix) const;
	int get_boundary_size() const; 

}; 

BOOST_AUTO_TEST_CASE( test_fixpoint_sparse_solver ) 
{
	CAMultX mult; 
	
	int boundary = mult.get_boundary_size(); 
	
	const vector<double> x = {1,2,3,2,4,3,4,6,4,7,4,3,2,1,5,3,9,1,2}; 
	vector<double> b(x.size()); 
	vector<double> y(x.size()); 
	
	b[0] = x[0]; 
	b[x.size()-1] = x[x.size()-1]; 
	auto ix = x.begin() + boundary; 
	for (auto ib = b.begin() + boundary; 
	     ix != x.end() - boundary; ++ix, ++ib) {
		*ib = mult(ix); 
	}
	cvdebug() << "x = " << x << "\n"; 
	cvdebug() << "b = " << b << "\n"; 

	BOOST_CHECK_EQUAL(CSolverMock().solve(b, y, mult), 0); 
	
	ix = x.begin(); 
	for (auto iy = y.begin(); ix != x.end(); ++ix, ++iy)
		BOOST_CHECK_CLOSE(*iy, *ix, 0.01); 
}



CSolverMock::value_type CAMultX::operator () (CSolverMock::const_field_iterator ix) const
{
	return *ix - 0.2 * (ix[-1] + ix[1]);
}
		
int CAMultX::get_boundary_size() const
{
	return 1; 
}


int CSolverMock::solve(const Field& rhs, Field& x, const CSolverMock::A_mult_x& mult) const
{
	assert(rhs.size() == x.size()); 
	
	Field y(x); 
	int boundary = mult.get_boundary_size(); 
	double res = 0.0; 
	int iter = 0; 

	do {
		res = 0.0; 
		auto ix = x.begin(); 
		auto ib = rhs.begin();
		auto iy = y.begin(); 
		for (size_t i = 0; i < x.size(); ++i, ++ix, ++ib, ++iy) {
			if (i >= boundary && i <  x.size() - boundary) { 
				double d = *ib - mult(ix); 
				*iy += d;
				res += d * d; 
			} else {
				*iy = *ib; 
			}
		}
		copy(y.begin(), y.end(), x.begin()); 
		cvdebug() << "[" << iter << "] res = " << res << ", x = " << x << "\n"; 
		++iter; 
	} while (res > 0.0000001 && iter < 100); 
	
	return iter < 100 ? 0 : 1; 
}




