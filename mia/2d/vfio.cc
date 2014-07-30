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

#include <mia/core/export_handler.hh>

#include <mia/2d/defines2d.hh>
#include <mia/2d/vfio.hh>
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


template <> const char *  const 
TPluginHandler<C2DVFIOPlugin>::m_help =  
   "These plug-ins handle loading and storing of the supported 2D vector field file types.";


template class TPlugin<io_2dvf_type, io_plugin_type>; 
template class TIOPlugin<io_2dvf_type>;
template class THandlerSingleton<TIOPluginHandler<C2DVFIOPlugin> >;
template class TIOPluginHandler<C2DVFIOPlugin>;
template class TPluginHandler<C2DVFIOPlugin>;


NS_MIA_END
