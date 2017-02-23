/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/core/testplugin.hh>

NS_MIA_USE

class CDummy2 :public CTestPlugin {
public:
	CDummy2();
private:
  	virtual const std::string do_get_descr() const;
};

CDummy2::CDummy2():
  CTestPlugin("dummy2")
{
}

const std::string CDummy2::do_get_descr() const
{
  return "test module with no data (2)";
}

class CDummy3 :public CTestPlugin {
public:
	CDummy3();
private:
  	virtual const std::string do_get_descr() const;
};

CDummy3::CDummy3():
  CTestPlugin("dummy3")
{
	add_property(test_property);
}

const std::string test_dummy_symbol()
{
	return std::string("test_dummy_symbol from dummy3"); 
}

const std::string CDummy3::do_get_descr() const
{
	return test_dummy_symbol(); 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CTestPlugin *d2 = new CDummy2();
	d2->append_interface(new CDummy3());
	return d2;
}
