/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  define EXPORT_HANDLER
#endif

#include <mia/2d/2dvfio.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN

const char *io_2dvf_type::type_descr = "2dvf";

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

NS_MIA_END
