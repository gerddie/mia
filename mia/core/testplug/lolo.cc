/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <mia/core/dummyhandler.hh>

NS_MIA_USE

using namespace std;

class CLoloIOPlugin : public CTestIOPlugin {
public:
	CLoloIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};

CLoloIOPlugin::CLoloIOPlugin():
	CTestIOPlugin("lo")
{
	this->add_suffix(".lo"); 
}

CLoloIOPlugin::PData  CLoloIOPlugin::do_load(const string& /*fname*/) const
{
	return CLoloIOPlugin::PData(new CDummyType("lolo"));
}


bool CLoloIOPlugin::do_save(const string& /*fname*/, const Data& /*data*/) const
{
	return true;
}

const string CLoloIOPlugin::do_get_descr() const
{
	return "a dummy plugin to test io-plugin handling";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CLoloIOPlugin();
}
