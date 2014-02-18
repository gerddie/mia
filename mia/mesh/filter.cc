/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/core/export_handler.hh>

#include <mia/mesh/filter.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using namespace boost;
using namespace std;


PTriangleMesh  EXPORT_MESH run_filter_chain(PTriangleMesh mesh, const std::vector<const char *>& filters)
{
	const auto& fh = CMeshFilterPluginHandler::instance();
	for (auto fd = filters.begin(); fd != filters.end(); ++fd) {
		auto f = fh.produce(*fd);
		if (!f)
			throw create_exception<invalid_argument>("unable to create mesh filter from '", *fd, "'");
		mesh = f->filter(*mesh);
	}
	return mesh; 
}

PTriangleMesh  EXPORT_MESH run_filter(const CTriangleMesh& mesh, const char *filter)
{
	auto f = CMeshFilterPluginHandler::instance().produce(filter);
	if (!f)
		throw create_exception<invalid_argument>("unable to create filter from '", filter, "'");
	return f->filter(mesh);
}

using boost::filesystem::path; 
CMeshFilterPluginHandlerTestPath::CMeshFilterPluginHandlerTestPath()
{
	CPathNameArray sksearchpath({path(MIA_BUILD_ROOT"/mia/mesh/filter")});
	CMeshFilterPluginHandler::set_search_path(sksearchpath); 
}



template<> const  char * const 
TPluginHandler<CMeshFilterPlugin>::m_help = 
   "These plug-ins provide mesh filters. Unless otherwise noted, "
   "they take a triangular mesh as input, "
   "process it and hand it to the next filter in the pipeline." 
; 

template class TDataFilter<CTriangleMesh>;
template class TDataFilterPlugin<CTriangleMesh>;
template class TPlugin<CTriangleMesh, filter_type>;
template class THandlerSingleton<TFactoryPluginHandler<CMeshFilterPlugin> >;
template class TFactoryPluginHandler<CMeshFilterPlugin>;
template class TPluginHandler<CMeshFilterPlugin>;
template class TFilterChain<CMeshFilterPluginHandler>; 



NS_MIA_END
