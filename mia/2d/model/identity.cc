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

#include <cassert>
#include <mia/2d/model/identity.hh>


using namespace identity_regmodel;
using namespace mia; 
using namespace std; 

void C2DIdentityRegModel::do_solve(const mia::C2DFVectorfield& b, mia::C2DFVectorfield& v) const
{
	assert(b.get_size() == v.get_size());
	copy(b.begin(), b.end(), v.begin());
}

class C2DIdentityRegModelPlugin: public C2DRegModelPlugin {
public:
	C2DIdentityRegModelPlugin();
	C2DRegModel *do_create()const;

private:
	const string do_get_descr()const;
};

C2DIdentityRegModelPlugin::C2DIdentityRegModelPlugin():
	C2DRegModelPlugin("identity")
{
}

C2DRegModel *C2DIdentityRegModelPlugin::do_create()const
{
	return new C2DIdentityRegModel();
}

const string C2DIdentityRegModelPlugin::do_get_descr()const
{
	return "identity registration model - cust copies the input force field to the output";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DIdentityRegModelPlugin();
}
