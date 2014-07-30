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

#ifndef mia_mesh_filter_hh
#define mia_mesh_filter_hh

#include <mia/mesh/triangularMesh.hh>
#include <mia/core/factory.hh>
#include <mia/template/filter_chain.hh>

NS_MIA_BEGIN

/**   
      \ingroup mesh
      mesh image filter 
*/
typedef TDataFilter<CTriangleMesh> CMeshFilter;

/**
   \ingroup filtering 
   2D image filter plugin 
*/
typedef TDataFilterPlugin<CTriangleMesh> CMeshFilterPlugin;

/**   
      \ingroup filtering 
      shared pointer type for the C2DFilter class 
*/
typedef std::shared_ptr<CMeshFilter > PMeshFilter;

/**
   \ingroup filtering 
   2D filter plugin handler
*/
typedef THandlerSingleton<TFactoryPluginHandler<CMeshFilterPlugin> > CMeshFilterPluginHandler;

/// @cond NEVER 
FACTORY_TRAIT(CMeshFilterPluginHandler); 
/// @endcond 

/**
   Convenience function to create a filter from its string description
 */
inline PMeshFilter produce_mesh_filter(const char* descr) 
{
	return CMeshFilterPluginHandler::instance().produce(descr); 
}


/**
   \ingroup filtering 
   2D filter chain to apply various filters in one run 
*/
typedef TFilterChain<CMeshFilterPluginHandler> CMeshFilterChain; 
/**
   \ingroup filtering 
   
   convenience function: run a filter chain on an mesh 
   @param mesh input mesh 
   @param filters vector of strings defining the filter to be applied 
   @returns the filtered mesh 
*/
PTriangleMesh  EXPORT_MESH run_filter_chain(PTriangleMesh mesh, const std::vector<const char *>& filters);

/**
   \ingroup filtering 
   convenience function: create and run a filter on an mesh 
   @param mesh input mesh 
   @param filter string defining the filter to be applied 
   @returns the filtered mesh 
*/
PTriangleMesh  EXPORT_MESH run_filter(const CTriangleMesh& mesh, const char *filter);

/**
   \ingroup filtering 
   convenience function: create and run a filter on an mesh 
   @param mesh input mesh 
   @param filter string defining the filter to be applied 
   @returns the filtered mesh 
*/
inline PTriangleMesh run_filter(PTriangleMesh mesh, const char *filter)
{
	return run_filter(*mesh, filter); 
}



NS_MIA_END


#endif
