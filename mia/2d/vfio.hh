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

#ifndef mia_2d_3dvfio_hh
#define mia_2d_3dvfio_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/2d/vectorfield.hh>

NS_MIA_BEGIN

/**
   @ingroup io 
   \brief a class for the IO of 2D vector fields 

   \remark this class is obsolete, new code should use the 2D transformation IO 
 */


class EXPORT_2D C2DIOVectorfield: public C2DFVectorfield, public CIOData {
public:
	C2DIOVectorfield();

        /** Constructor to create empty Datafield if given size */
        C2DIOVectorfield(const C2DBounds& _Size);

        /** copy - Constructor */
        C2DIOVectorfield(const C2DIOVectorfield& org);

	/** copy from normal vectorfield constructor */
	C2DIOVectorfield(const C2DFVectorfield& org);

	/**
	   Create a copy of this object (needed for CDatapool) 
	   \returns a dynamically allocated copy 
	 */
	C2DIOVectorfield *clone() const __attribute__((warn_unused_result));
};

struct io_2dvf_type {
	typedef  C2DIOVectorfield type;
	static const char *data_descr;
};



/// Base class for 2D vector field IO plugins 
typedef TIOPlugin<io_2dvf_type> C2DVFIOPlugin;

extern template class EXPORT_2D TPlugin<io_2dvf_type, io_plugin_type>; 
extern template class EXPORT_2D TIOPlugin<io_2dvf_type>; 
extern template class EXPORT_2D TPluginHandler<C2DVFIOPlugin>;
extern template class EXPORT_2D TIOPluginHandler<C2DVFIOPlugin>;

/// Handler for 2D vector field IO 
typedef  THandlerSingleton<TIOPluginHandler<C2DVFIOPlugin> > C2DVFIOPluginHandler;

extern template class EXPORT_2D THandlerSingleton<TIOPluginHandler<C2DVFIOPlugin> >;


NS_MIA_END

#endif
