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

#include <stdexcept>
#include <istream>
#include <fstream>
#include <mia/3d/filter/labelmap.hh>

#include <mia/template/labelmap.cxx>

NS_MIA_BEGIN

template class TLabelMapFilter<C3DImage>;
template class TLabelMapFilterPlugin<C3DImage>;

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C3DLabelMapFilterPlugin();
}


NS_MIA_END
