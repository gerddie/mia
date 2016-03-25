/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <boost/filesystem.hpp>
#include <mia/core/export_handler.hh>
#include <mia/2d/sparse_image_solver.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>


NS_MIA_BEGIN

template class TSparseSolver<C2DFImage>; 


typedef C2DImageSparseSolver::A_mult_x C2DImageSolverAmultx; 


EXPORT_2D C2DFImage operator * (const C2DImageSolverAmultx& A, const C2DFImage& X)
{
	assert(A.get_size() == X.get_size());

	C2DFImage result(X.get_size()); 
	long b = A.get_boundary_size(); 
	long nx = X.get_size().x - 2 * b; 
	long ny = X.get_size().y - 2 * b; 
	copy(X.begin(), X.begin() + b * X.get_size().x + b, result.begin()); 
	auto ir = result.begin() + b * X.get_size().x + b; 
	auto ix = X.begin() + b * X.get_size().x + b; 
		
	for (int y = 0; y < ny; ++y) {
		int x = 0; 
		for (; x < nx; ++x, ++ix, ++ir)
			*ir = A(ix); 
		for (; x < (int)X.get_size().x; ++x, ++ix, ++ir) 
			*ir = *ix; 
	}
	copy(ix, X.end(), ir); 
	return result; 
}

template <> const char *  const 
TPluginHandler<TFactory<C2DImageSolverAmultx>>::m_help =  "These plug-ins define the multiplication of a sparse matrix "
				"with a vector that is used in the sparse image solver class.";


template <> const char *  const 
TPluginHandler<TFactory<C2DImageSparseSolver>>::m_help =  "These plug-ins define the solvers for sparse systems of linear equations that "
				"arise when solving certain image processing problems.";

EXPLICIT_INSTANCE_HANDLER(C2DImageSparseSolver); 
EXPLICIT_INSTANCE_HANDLER(C2DImageSolverAmultx); 

NS_MIA_END
