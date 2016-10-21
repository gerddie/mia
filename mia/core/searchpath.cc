/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/core/searchpath.hh>
#include <mia/core/msgstream.hh>


#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <boost/filesystem/operations.hpp>

#include <config.h>

#ifdef MIA_USE_BOOST_REGEX
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match; 
#else 
# if __cplusplus >= 201103
# include <regex>
using std::regex;
using std::regex_match; 
# else 
# error must either use boost::regex or c++11 based regex
# endif 
#endif


NS_MIA_BEGIN

using std::swap; 
using namespace boost::filesystem;

EXPORT_CORE const std::string get_plugin_root();

struct CPluginSearchpathData {

        CPluginSearchpathData(bool no_subpath);

        void set_standard_paths(); 
        
        std::vector<boost::filesystem::path> m_paths;
	bool m_no_subpath; 
}; 

CPluginSearchpathData::CPluginSearchpathData(bool no_subpath):
	m_no_subpath(no_subpath)
{
}

void CPluginSearchpathData::set_standard_paths()
{
       
#ifdef PLUGIN_HOME_SEARCH_PATH
        char *c_home = getenv("HOME"); 
        if (c_home)
                impl->m_paths.push_back( path(c_home, bfs::native) / path(PLUGIN_HOME_SEARCH_PATH)); 
#endif

	// MIA_PLUGIN_PATH overrides the system path. 
        char *c_user = getenv("MIA_PLUGIN_PATH");
        if (c_user)
                m_paths.push_back( path(c_user) );
	else
		m_paths.push_back( path(get_plugin_root()));
}

CPluginSearchpath::CPluginSearchpath(bool no_subpath)
{
        impl = new CPluginSearchpathData(no_subpath); 
}
        

CPluginSearchpath::CPluginSearchpath(const CPluginSearchpath& other)
{
	impl = new CPluginSearchpathData(*other.impl); 
}
	
CPluginSearchpath& CPluginSearchpath::operator = (const CPluginSearchpath& other)
{
	if (&other != this) {
		auto i = new CPluginSearchpathData(*other.impl);
		swap(i, impl);
		delete i; 
	}
	return *this; 
}


CPluginSearchpath::~CPluginSearchpath()
{
        delete impl; 
}

void CPluginSearchpath::add(const char *path)
{
	impl->m_paths.push_back(boost::filesystem::path(path)); 
}

void CPluginSearchpath::add(const boost::filesystem::path& path)
{
        impl->m_paths.push_back(path); 
}

std::vector<PPluginModule> CPluginSearchpath::find_modules(const std::string& data, const std::string& type) const
{
        if (impl->m_paths.empty())
                impl->set_standard_paths(); 


        std::stringstream pattern; 
        pattern << ".*\\."<< MIA_MODULE_SUFFIX << "$";

        regex pat_expr(pattern.str());

	std::vector<PPluginModule> result;
	
	path type_path = path(data) / path(type);

	std::vector<path> candidates; 
        
        for(auto p: impl->m_paths) {
                auto dir = impl->m_no_subpath ? p  : p / type_path;
                
                cvdebug() << "Looking for " << dir.string() << "\n";
                
		if (exists(dir) && is_directory(dir)) {
                        
			// if we cant save the old directory something is terribly wrong
			directory_iterator di(dir); 
			directory_iterator dend;
			
			cvdebug() << "TPluginHandler<I>::initialise: scan '" << dir.string() << "'\n"; 
                        
			while (di != dend) {
				cvdebug() << "    candidate:'" << di->path().string() << "'"; 
				if (regex_match(di->path().string(), pat_expr)) {
					candidates.push_back(*di); 
					cverb << " add\n";
				}else
					cverb << " discard\n";
				++di;
			}
		}
        }

        for (auto i : candidates) {
		try {
			cvdebug()<< " Load '" <<i.string()<<"'\n"; 
			result.push_back(PPluginModule(new CPluginModule(i.string().c_str())));
		}
		catch (std::invalid_argument& ex) {
			cverr() << ex.what() << "\n"; 
		}
		catch (std::exception& ex) {
			cverr() << ex.what() << "\n"; 
		}
		catch (...) {
			cverr() << "Loading module " << i.string() << "failed for unknown reasons\n"; 
		}
	}
        
        return result; 
}

NS_MIA_END
