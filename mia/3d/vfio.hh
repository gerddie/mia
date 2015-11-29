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

#ifndef mia_3d_3dvfio_hh
#define mia_3d_3dvfio_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/3d/vectorfield.hh>

NS_MIA_BEGIN

/**
   @ingroup io 
   @brief An interface class to wrap a vector field into something, IO plugins can understand.
*/

class EXPORT_3D C3DIOVectorfield: public C3DFVectorfield, public CIOData {
public:

	typedef C3DIOVectorfield type; 
        typedef C3DFVectorfield plugin_data;
	
	C3DIOVectorfield();

        /** Constructor to create empty Datafield if given size */
        C3DIOVectorfield(const C3DBounds& _Size);

        /** copy - Constructor */
        C3DIOVectorfield(const C3DIOVectorfield& org);

	/** copy from normal vectorfield constructor */
	C3DIOVectorfield(const C3DFVectorfield& org);

	/// @returns a dynamically allocated copy of the IO object 
	C3DIOVectorfield *clone() const __attribute__((warn_unused_result));

};

/**
   @ingroup io 
   @brief Base class for vector field IO plug-ins 
*/
typedef TIOPlugin<C3DIOVectorfield> C3DVFIOPlugin;

template <> const char *  const TPluginHandler<C3DVFIOPlugin>::m_help; 

extern template class EXPORT_3D TPlugin<C3DIOVectorfield, io_plugin_type>;
extern template class EXPORT_3D TIOPlugin<C3DIOVectorfield>;
extern template class EXPORT_3D TIOPluginHandler<C3DVFIOPlugin>; 
extern template class EXPORT_3D THandlerSingleton<TIOPluginHandler<C3DVFIOPlugin> >; 
/**
   @ingroup io 
   @brief Plug-in handler for vector field IO 
*/
typedef THandlerSingleton<TIOPluginHandler<C3DVFIOPlugin> > C3DVFIOPluginHandler;

NS_MIA_END

#endif
