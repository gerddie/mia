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

#ifndef ph_iodummy_hh
#define ph_iodummy_hh

#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/core/factory.hh>

NS_MIA_BEGIN

class EXPORT_CORE CDummyType : public std::string, public CIOData {
public:
	CDummyType(const char *s);

	CDummyType* clone() const;
};

struct test_io_data {
	typedef CDummyType type;
	static const char *data_descr;
};

extern template class EXPORT_CORE TPlugin<test_io_data,io_plugin_type>; 
extern template class EXPORT_CORE TIOPlugin<test_io_data>; 


class EXPORT_CORE CTestIOPlugin : public TIOPlugin<test_io_data> {
public:
	typedef  test_io_data::type Data;
//	typedef  TIOPlugin<test_io_type>::PData PData;
	CTestIOPlugin(const char *name);
};


/// Test IO plugin handler, don't use this in real code  
typedef THandlerSingleton<TIOPluginHandler<CTestIOPlugin> > CTestIOPluginHandler;


NS_MIA_END

#endif
