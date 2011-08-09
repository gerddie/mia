/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
using namespace std;

class EXPORT CDummy1 :public CTestPlugin {
public:
	CDummy1();
private:
	virtual const string do_get_descr() const;
	virtual bool do_test() const;
};

CDummy1::CDummy1():
  CTestPlugin("dummy1")
{
}

const string CDummy1::do_get_descr() const
{
	return "test module with no data";
}


bool CDummy1::do_test() const
{
	return true;
}


extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CDummy1();
}





