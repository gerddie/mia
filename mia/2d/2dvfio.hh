/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_3dvfio_hh
#define mia_2d_3dvfio_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/2d/2DVectorfield.hh>

NS_MIA_BEGIN



class EXPORT_2D C2DIOVectorfield: public C2DFVectorfield, public CIOData {
public:
	C2DIOVectorfield();

        /** Constructor to create empty Datafield if given size */
        C2DIOVectorfield(const C2DBounds& _Size);

        /** copy - Constructor */
        C2DIOVectorfield(const C2DIOVectorfield& org);

	/** copy from normal vectorfield constructor */
	C2DIOVectorfield(const C2DFVectorfield& org);

	C2DIOVectorfield *clone() const;
};

struct io_2dvf_type {
	typedef  C2DIOVectorfield type;
	static const char *type_descr;
};

typedef TIOPlugin<io_2dvf_type> C2DVFIOPlugin;
typedef THandlerSingleton<TIOPluginHandler<C2DVFIOPlugin> > C2DVFIOPluginHandler;

NS_MIA_END

#endif
