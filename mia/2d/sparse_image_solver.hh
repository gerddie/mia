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

typedef TSparseSolver<C2DFImage> C2DImageSparseSolver; 

typedef std::shared_ptr<C2DImageSparseSolver > P2DImageSparseSolver;

typedef TFactory<C2DImageSparseSolver> C2DImageSparseSolverPlugin; 

typedef THandlerSingleton<TFactoryPluginHandler<C2DImageSparseSolverPlugin> > C2DImageSparseSolverPluginHandler;

FACTORY_TRAIT(C2DImageSparseSolverPluginHandler); 


typedef C2DImageSparseSolver::A_mult_x C2DImageSolverAmultx; 

typedef std::shared_ptr<C2DImageSolverAmultx> P2DImageSolverAmultx;

typedef TFactory<C2DImageSolverAmultx> C2DImageSolverAmultxPlugin;

typedef THandlerSingleton<TFactoryPluginHandler<C2DImageSolverAmultxPlugin> > C2DImageSolverAmultxPluginHandler;

FACTORY_TRAIT(C2DImageSolverAmultxPluginHandler); 


/**
   Implement the multiplication of a Matrix with an Image, 
   \param A the matrix 
   \param x the input image 
   \returns A * x, (works like a matrix filter) 
   \remark maybethis should go into the filter section 
 */

C2DFImage operator * (const C2DImageSolverAmultx& A, const C2DFImage& x); 

NS_MIA_END

#endif
