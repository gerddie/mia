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


#include <mia/3d/vfio.hh>

#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN

const char *io_3dvf_data::data_descr = "3dvf";

C3DIOVectorfield::C3DIOVectorfield()
{
}

/** Constructor to create empty Datafield if given size */
C3DIOVectorfield::C3DIOVectorfield(const C3DBounds& _Size):
	C3DFVectorfield(_Size)
{
}

C3DIOVectorfield::C3DIOVectorfield(const C3DIOVectorfield& org):
	C3DFVectorfield(org),
	CIOData(org)
{
}

C3DIOVectorfield::C3DIOVectorfield(const C3DFVectorfield& org):
	C3DFVectorfield(org)
{
}

C3DIOVectorfield *C3DIOVectorfield::clone() const
{
	return new C3DIOVectorfield(*this);
}

template <> const char *  const 
TPluginHandler<C3DVFIOPlugin>::m_help =  
   "These plug-ins implement loading and saving of vector fields to certain file formats.";

template class TIOPlugin<io_3dvf_data>;
template class THandlerSingleton<TIOPluginHandler<C3DVFIOPlugin> >;
template class TIOPluginHandler<C3DVFIOPlugin>;
template class TPluginHandler<C3DVFIOPlugin>;

NS_MIA_END
