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

#ifndef mia_internal_nonrigidregister_hh
#define mia_internal_nonrigidregister_hh

#include <mia/core/minimizer.hh>
#include <mia/internal/multicost.hh>
#include <mia/internal/transformfactory.hh>
#include <mia/internal/dimtrait.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN

template <int dim> 
struct TNonrigidRegisterImpl; 

/**
   \ingroup registration

   \brief Class for non-rigid registration. 
   \tparam dim dimension of the input data 

   The registration approach of this class lies in the optimization of a weighted sum of cost functions. 
   These cost functions may include image based cost functions, penalty functions on the transformation, 
   and others. The registration makes use of a multi-resolution scheme and support various transformation 
   models.  All cost functions implement the C2DFullCost interface.  
   Currently supported minimizers are those available in the GNU scientific library.
*/

template <int dim> 
class EXPORT_HANDLER TNonrigidRegister {
public:
	/// the trait to handle dimension based typedefs 
	typedef dimension_traits<dim> this_dim_traits;

	/// the pointer type of the transformation 
	typedef typename this_dim_traits::PTransformation PTransformation; 
	
	/// the type of the image data 
	typedef typename this_dim_traits::Image Image; 
	
	/// the pointer type of the image data 
	typedef typename this_dim_traits::PImage PImage; 
	
	/// the pointer type of the transformation creation factory 
	typedef typename this_dim_traits::PTransformationFactory PTransformationFactory; 

	/// the type of the cost function list 
	typedef typename this_dim_traits::FullCostList FullCostList; 

	/// filter type for the image data used in this non-rigid registration algorithm 
	typedef typename this_dim_traits::Filter Filter; 

	/// the plug-in handler for the filters of the used dimensionality 
	typedef typename this_dim_traits::FilterPluginHandler FilterPluginHandler;

	/**
	   Constructor for the registration tool
	   \param costs cost function model
	   \param minimizer GSL provided minimizer
	   \param transform_creation string describing which transformation is supported
	   \param mg_levels multigrisd levels to be used 
	   \param idx image index  - only used in a multi-threading environment 
	 */

	TNonrigidRegister(FullCostList& costs, PMinimizer minimizer,
			  PTransformationFactory transform_creation,
			  size_t mg_levels, int idx = -1);
	
	
	~TNonrigidRegister();

	/**
	   Run the registration of an image pair. 
	   \param src source (moving) image 
	   \param ref reference (fixed) image 
	   \returns the transformation registering src to ref that minimizes the constructor given 
	   cost function 
	 */
	PTransformation  run(PImage src, PImage ref) const;


	/**
	   Run the registration by optimizing whatever is given with the cost functions 
	   \returns the transformation registering that minimizes the constructor given 
	   cost function 
	 */
	PTransformation  run() const;

private:
	struct TNonrigidRegisterImpl<dim> *impl;
};

NS_MIA_END

#endif
