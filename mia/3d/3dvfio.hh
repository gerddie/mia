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


#ifndef mia_3d_3dvfio_hh
#define mia_3d_3dvfio_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/3d/3DVectorfield.hh>

NS_MIA_BEGIN

/**
   An interface class to put vector fields into something, IO plugins can understand.
*/

class EXPORT_3D C3DIOVectorfield: public C3DFVectorfield, public CIOData {
public:
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

///@cond LAZY 
struct io_3dvf_data {
	typedef  C3DIOVectorfield type;
	static const char *data_descr;
};
///@endcond 

/// Base class for vector field IO plug-ins 
typedef TIOPlugin<io_3dvf_data> C3DVFIOPlugin;

/// Plug-in handler for vector field IO 
typedef THandlerSingleton<TIOPluginHandler<C3DVFIOPlugin> > C3DVFIOPluginHandler;

NS_MIA_END

#endif
