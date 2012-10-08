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


#ifndef mia_3d_transformfactory_hh
#define mia_3d_transformfactory_hh

#include <set>
#include <mia/3d/transform.hh>
#include <mia/template/transformfactory.hh>

NS_MIA_BEGIN

/**
   \ingroup io
   \brief base class for the creators of 3D transformations 
    \todo rename this to factory 
 */
typedef TTransformCreator<C3DTransformation>  C3DTransformCreator; 

/**
   \ingroup io
   \brief pointer type of the class for the creators of 3D transformations 
*/
typedef std::shared_ptr<C3DTransformCreator > P3DTransformationFactory;

/**
   \ingroup io
   \brief Base class for the transform creator plugins 
*/
typedef TTransformCreatorPlugin<C3DTransformation>  C3DTransformCreatorPlugin; 

/**
   \ingroup io
   \brief Transform creator plugin handler 
*/
typedef THandlerSingleton<TFactoryPluginHandler<C3DTransformCreatorPlugin> > C3DTransformCreatorHandler;


/**
   Create a transformation creator from the given description
   \param descr
   \returns the transformation creator 
 */
inline 
P3DTransformationFactory produce_3dtransform_factory(const std::string& descr) 
{
	return C3DTransformCreatorHandler::instance().produce(descr); 
}

/** 
    @cond INTERNAL 
    @ingroup test 
    @brief class to initialize the plug-in path for tests on the uninstalled library 
*/
struct EXPORT_3D C3DTransformCreatorHandlerTestPath {
	C3DTransformCreatorHandlerTestPath(); 
private: 
	CSplineKernelTestPath spktp; 
	CSplineBoundaryConditionTestPath spctp;
}; 
/// @endcond

/// @cond NEVER 
FACTORY_TRAIT(C3DTransformCreatorHandler); 
/// @endcond

NS_MIA_END


#endif
