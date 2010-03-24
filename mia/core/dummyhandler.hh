/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
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

#ifndef ph_iodummy_hh
#define ph_iodummy_hh

#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>

NS_MIA_BEGIN

class EXPORT_CORE CDummyType : public std::string, public CIOData {
public:
	CDummyType(const char *s);

	CDummyType* clone() const;
};

struct test_io_data {
	typedef CDummyType type;
	static const char *type_descr;
};


class EXPORT_CORE CTestIOPlugin : public TIOPlugin<test_io_data> {
public:
	typedef  test_io_data::type Data;
//	typedef  TIOPlugin<test_io_type>::PData PData;
	CTestIOPlugin(const char *name);
};


typedef TIOHandlerSingleton<TIOPluginHandler<CTestIOPlugin> > CTestIOPluginHandler;

NS_MIA_END

#endif
