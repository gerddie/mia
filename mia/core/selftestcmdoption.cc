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

#include <cassert>
#include <mia/core/selftestcmdoption.hh>
#include <mia/core/plugin_base.hh>

NS_MIA_BEGIN

CSelftestCallback::CSelftestCallback(int argc, char **argv):
       m_argc(argc), m_argv(argv)
{
}

int CSelftestCallback::run () const
{
       PrepareTestPluginPath prepare_plugin_path;
       return do_run(m_argc, m_argv);
}



NS_MIA_END
