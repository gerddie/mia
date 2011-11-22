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

#ifndef mia_2d_sparse_image_solver_hh
#define mia_2d_sparse_image_solver_hh

#include <mia/core/factory.hh>
#include <mia/core/sparse_solver.hh>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

/// base type for the sparse solvers for 2D images 
typedef TSparseSolver<C2DFImage> C2DImageSparseSolver; 

/// pointer type for the sparse solvers for 2D images 
typedef std::shared_ptr<C2DImageSparseSolver > P2DImageSparseSolver;

/// Plugin handler for sparse image solver plug-ins 
typedef THandlerSingleton<TFactoryPluginHandler<TFactory<C2DImageSparseSolver> > > C2DImageSparseSolverPluginHandler;

/**   
      \ingroup tests 
      Class to set up the plug-in search path for spline kernels when running tests
      in the build tree 
*/
struct EXPORT_2D C2DImageSparseSolverTestPath {
	C2DImageSparseSolverTestPath(); 
}; 


FACTORY_TRAIT(C2DImageSparseSolverPluginHandler); 


typedef C2DImageSparseSolver::A_mult_x C2DImageSolverAmultx; 

typedef std::shared_ptr<C2DImageSolverAmultx> P2DImageSolverAmultx;

typedef TFactory<C2DImageSolverAmultx> C2DImageSolverAmultxPlugin;

typedef THandlerSingleton<TFactoryPluginHandler<C2DImageSolverAmultxPlugin> > C2DImageSolverAmultxPluginHandler;

FACTORY_TRAIT(C2DImageSolverAmultxPluginHandler); 


/**
   Specialize the implementation of a Matrix with an Image, 
   \param A the matrix 
   \param x the input image 
   \returns A * x, (works like a matrix filter) 
   \remark maybethis should go into the filter section 
 */

template <typename T>
struct multiply<T2DImage<T> > {
	/**
	   The function to apply the multiplication 
	   \param[out] result location to store the result, ususally it should be allocated and of the same size like x 
	   \param[in] A sparse Matrix 
	   \param[in] x the filed to multiply with the matrix 
	 */
	static void apply(T2DImage<T>& result, const typename TSparseSolver<T2DImage<T> >::A_mult_x& A, const T2DImage<T>& x);
}; 

template <typename T>
void multiply<T2DImage<T> >::apply(T2DImage<T>& result, const typename TSparseSolver<T2DImage<T> >::A_mult_x& A, const T2DImage<T>& X)
{
	assert(result.get_size() == X.get_size()); 
	assert(result.get_size() == A.get_size());

	long b = A.get_boundary_size(); 
	long nx = X.get_size().x - 2 * b; 
	long ny = X.get_size().y - 2 * b; 
	copy(X.begin(), X.begin() + b * X.get_size().x + b, result.begin()); 
	auto ir = result.begin() + b * X.get_size().x + b; 
	auto ix = X.begin() + b * X.get_size().x + b; 
		
	for (int y = 0; y < ny; ++y) {
		int x = 0; 
		for (; x < nx; ++x, ++ix, ++ir)
			*ir = static_cast<T>(A(ix));
		for (; x < (int)X.get_size().x; ++x, ++ix, ++ir) 
			*ir = *ix; 
	}
	copy(ix, X.end(), ir); 
}

NS_MIA_END

#endif
