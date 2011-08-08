/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#include <mia/core/testplugin.hh>

NS_MIA_USE

class CDummy2 :public CTestPlugin {
public:
	CDummy2();
private:
  	virtual const std::string do_get_descr() const;
	virtual bool do_test() const;
};

CDummy2::CDummy2():
  CTestPlugin("dummy2")
{
}

const std::string CDummy2::do_get_descr() const
{
  return "test module with no data";
}

bool CDummy2::do_test() const
{
	return false;
}

class CDummy3 :public CTestPlugin {
public:
	CDummy3();
private:
  	virtual const std::string do_get_descr() const;
	virtual bool do_test() const;
};

CDummy3::CDummy3():
  CTestPlugin("dummy3")
{
	add_property(test_property);
}

const std::string CDummy3::do_get_descr() const
{
  return "test module with no data";
}

bool CDummy3::do_test() const
{
	return false;
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CTestPlugin *d2 = new CDummy2();
	d2->append_interface(new CDummy3());
	return d2;
}
