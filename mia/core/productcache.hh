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


#ifndef mia_core_productcache_hh
#define mia_core_productcache_hh

#include <map>
#include <string>
#include <mia/core/defines.hh>
#include <tbb/recursive_mutex.h>
#include <tbb/spin_mutex.h>

NS_MIA_BEGIN
typedef tbb::recursive_mutex::scoped_lock CRecursiveScopedLock; 

/** \brief base class for the product cache 
    
    This is the base class for all product caches. Product caches are used to 
    store results of the factory plugin handler creation method in order to avoid 
    re-creating the same objects over and over again.  
    
    Normally a product cache is disabled, because most programs don't make use of the 
    same filters over and over again.  
*/
class CProductCache {
public: 
	/**
	   This constructor registers the cache in the global product cache map
	   \param name name of the handled factory  
	*/
	CProductCache(const std::string& name); 

	/// enable this product cache 
	void enable(); 

	/// disable this product cache 
	void disable(); 

	/// \returns whether this cache is enabled 
	bool is_enabled() const; 
	
	/// clear teh cache 
	void clear(); 
	
private: 
	virtual void do_clear() = 0; 
	bool m_enabled; 
	mutable tbb::spin_mutex m_enable_mutex; 
}; 


/**
   \brief The type specific product cache 
   

 */
template <typename Handler> 
class TProductCache: public CProductCache {
public: 
	typedef typename Handler::ProductPtr ProductPtr; 
	

	/**
	   Constructor 
	 */
	TProductCache(); 


	/**
	   Get a profuct if it is cached, 
	   \param name the initializer string 
	   \returns the shared product pointer, if available, or an empty shared pointer 
	 */
	ProductPtr get(const std::string& name) const;


	/**
	   Add a product pointer to the cache if the cache is enabled
	   \param name initializer string of the product 
	   \param the shared pointer of the product 
	 */
	void add(const std::string& name, ProductPtr product); 
private: 
	
	virtual void do_clear(); 

	typedef std::map<std::string, ProductPtr> CMap; 
	CMap m_cache; 
	mutable tbb::recursive_mutex m_cache_mutex; 
}; 


/**
   \brief The singleton that handles all product caches 

*/

class CProductCacheHandler {
public: 
	/// \returns an instance of the cache handler 
	static CProductCacheHandler& instance(); 

	/// clears all registered product caches
	void clear_all(); 

	/**
	   Register a product cache 
	   \param name of the factory plugin handler 
	   \param cache the actual cache 
	 */
	void register_cache(const std::string& name,  CProductCache* cache);
private: 
	CProductCacheHandler(); 
	
	CProductCacheHandler(const CProductCacheHandler& other) = delete; 
	CProductCacheHandler& operator = (const CProductCacheHandler& other) = delete; 

	std::map<std::string, CProductCache*> m_caches; 
	static CMutex m_creation_mutex;
}; 


/////////////////////////

template <typename Handler> 
TProductCache<Handler>::TProductCache():
	CProductCache(Handler::get_search_descriptor())
{
}

template <typename Handler> 
typename Handler::ProductPtr TProductCache<Handler>::get(const std::string& name) const
{
	if (is_enabled()) {
		CRecursiveScopedLock lock(m_cache_mutex);
		auto i = m_cache.find(name); 
		if (i != m_cache.end())
			return i->second; 
	}
	return ProductPtr(); 
}
	
template <typename Handler> 
void TProductCache<Handler>::add(const std::string& name, ProductPtr product)
{
	if (is_enabled()) {
		CRecursiveScopedLock lock(m_cache_mutex);
		//re-check whether another thead already added the product 
		if (!get(name))
			m_cache[name] = product; 
	}
}

template <typename Handler> 
void TProductCache<Handler>::do_clear()
{
	CRecursiveScopedLock lock(m_cache_mutex);
	m_cache.clear(); 
}

NS_MIA_END

#endif 