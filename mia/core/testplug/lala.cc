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

class CLalaIOPlugin : public CTestIOPlugin {
public:
	CLalaIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
	std::string do_get_preferred_suffix() const; 
};

CLalaIOPlugin::CLalaIOPlugin():
	CTestIOPlugin("la")
{
	add_suffix(".la"); 
}

CLalaIOPlugin::PData  CLalaIOPlugin::do_load(const string& /*fname*/) const
{
	return CLalaIOPlugin::PData(new CDummyType("lala"));
}


bool CLalaIOPlugin::do_save(const string& /*fname*/, const Data& /*data*/) const
{
	return true;
}

const string CLalaIOPlugin::do_get_descr() const
{
	return "a dummy plugin to test io-plugin handling";
}

std::string CLalaIOPlugin::do_get_preferred_suffix() const
{
	return "hey"; 
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CLalaIOPlugin();
}
