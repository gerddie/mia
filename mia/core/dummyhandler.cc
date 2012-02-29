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
#include <mia/core/dummyhandler.hh>

#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN
using namespace std;

CDummyType::CDummyType(const char *s):
	string(s)
{
}

CDummyType* CDummyType::clone() const
{
	return new CDummyType(c_str());
}

const char *test_io_data::data_descr = "test";

CTestIOPlugin::CTestIOPlugin(const char *name):
	TIOPlugin<test_io_data>(name)
{
}

template class TPluginHandler<CTestIOPlugin>;
template class TIOPluginHandler<CTestIOPlugin>;
template class THandlerSingleton<TIOPluginHandler<CTestIOPlugin> >;
template class TIOHandlerSingleton<TIOPluginHandler<CTestIOPlugin> >;

template<> const char * const 
TPluginHandler<CTestIOPlugin>::m_help = "These are some dummy plug-ins for IO handler testing."; 

NS_MIA_END
