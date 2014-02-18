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

#ifndef mia_core_bfsv23dispatch_hh
#define mia_core_bfsv23dispatch_hh

#include <boost/filesystem/path.hpp>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

inline std::string __bfs_get_filename(const boost::filesystem::path& path) 
{
#if BOOST_FILESYSTEM_VERSION==3
	return path.filename().string(); 
#else 
	return path.filename(); 
#endif
}

inline std::string __bfs_get_extension(const boost::filesystem::path& path) 
{
#if BOOST_FILESYSTEM_VERSION==3
	return path.extension().string(); 
#else 
	return path.extension(); 
#endif
}

inline std::string __bfs_get_stem(const boost::filesystem::path& path) 
{
#if BOOST_FILESYSTEM_VERSION==3
	return path.stem().string(); 
#else 
	return path.stem(); 
#endif
}


NS_MIA_END

#endif
