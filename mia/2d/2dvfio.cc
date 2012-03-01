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

#include <mia/core/export_handler.hh>

#include <mia/2d/2dvfio.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN

const char *io_2dvf_type::data_descr = "2dvf";

C2DIOVectorfield::C2DIOVectorfield()
{
}

/** Constructor to create empty Datafield if given size */
C2DIOVectorfield::C2DIOVectorfield(const C2DBounds& _Size):
	C2DFVectorfield(_Size)
{
}


C2DIOVectorfield::C2DIOVectorfield(const C2DIOVectorfield& org):
	C2DFVectorfield(org),
	CIOData(org)
{
}


C2DIOVectorfield::C2DIOVectorfield(const C2DFVectorfield& org):
	C2DFVectorfield(org)
{
}

C2DIOVectorfield *C2DIOVectorfield::clone() const
{
	return new C2DIOVectorfield(*this);
}

template class TIOPlugin<io_2dvf_type>;
template class THandlerSingleton<TIOPluginHandler<C2DVFIOPlugin> >;
template class TIOPluginHandler<C2DVFIOPlugin>;
template class TPluginHandler<C2DVFIOPlugin>;

template <> const char *  const 
TPluginHandler<C2DVFIOPlugin>::m_help =  
   "These plug-ins handle loading and storing of the supported 2D vector field file types.";


NS_MIA_END
