/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/module.hh>
#include <vector>

namespace boost {
namespace filesystem {
class path;
}
}

NS_MIA_BEGIN

class EXPORT_CORE CPluginSearchpath {
public:
        CPluginSearchpath(bool no_subpath=false);

	CPluginSearchpath(const CPluginSearchpath& other); 
	CPluginSearchpath& operator = (const CPluginSearchpath& other); 

        ~CPluginSearchpath();

        void add(const char *path);
	
        void add(const boost::filesystem::path& path)__attribute__((deprecated));
        
        std::vector<PPluginModule> find_modules(const std::string& data, const std::string& type) const; 

private:
        struct CPluginSearchpathData *impl; 
        
}; 

NS_MIA_END
