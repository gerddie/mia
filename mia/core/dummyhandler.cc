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

template<> const char * const 
TPluginHandler<CTestIOPlugin>::m_help = "These are some dummy plug-ins for IO handler testing."; 


template class TPlugin<test_io_data,io_plugin_type>; 
template class TPluginHandler<CTestIOPlugin>;
template class TIOPlugin<test_io_data>; 
template class TIOPluginHandler<CTestIOPlugin>;
template class THandlerSingleton<TIOPluginHandler<CTestIOPlugin> >;


NS_MIA_END
