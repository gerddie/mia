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
#include <mia/core/productcache.hh>


NS_MIA_BEGIN

CProductCache::CProductCache(const std::string& name):m_enabled(false)
{
	CProductCacheHandler::instance().register_cache(name, this); 
}

void CProductCache::enable()
{
	tbb::spin_mutex::scoped_lock lock(m_enable_mutex);
	m_enabled = true; 
}

void CProductCache::disable()
{
	tbb::spin_mutex::scoped_lock lock(m_enable_mutex);
	m_enabled = false; 
}

bool CProductCache::is_enabled() const
{
	tbb::spin_mutex::scoped_lock lock(m_enable_mutex);
	return m_enabled; 
}

void CProductCache::clear()
{
	do_clear(); 
}

CMutex CProductCacheHandler::m_creation_mutex; 

CProductCacheHandler& CProductCacheHandler::instance() 
{
	CScopedLock lock(m_creation_mutex); 
	static CProductCacheHandler me; 
	return me; 
}

CProductCacheHandler::CProductCacheHandler()
{
}

void CProductCacheHandler::clear_all()
{
	for(auto i =  m_caches.begin();  i != m_caches.end(); ++i){
		assert(i->second); 
		i->second->clear(); 
	}
}

void CProductCacheHandler::register_cache(const std::string& name,  CProductCache* cache)
{
	// each cahc must only be registered once
	assert(m_caches.find(name) == m_caches.end()); 
	m_caches[name] = cache; 
}

NS_MIA_END