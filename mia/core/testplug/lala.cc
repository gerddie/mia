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

#include <mia/core/dummyhandler.hh>

NS_MIA_USE
using namespace std;

class CLalaIOPlugin : public CTestIOPlugin
{
public:
       CLalaIOPlugin();
private:
       PData do_load(const string& fname) const;
       bool do_save(const string& fname, const Data& data) const;
       const string do_get_descr() const;
       const std::string do_get_preferred_suffix() const;
};

CLalaIOPlugin::CLalaIOPlugin():
       CTestIOPlugin("la")
{
       add_suffix(".la");
       add_suffix(".hey");
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

const std::string CLalaIOPlugin::do_get_preferred_suffix() const
{
       return "hey";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
       return new CLalaIOPlugin();
}
