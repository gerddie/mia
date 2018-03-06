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

#ifndef mia_core_productcache_hh
#define mia_core_productcache_hh

#include <map>
#include <string>
#include <mia/core/parallel.hh>

NS_MIA_BEGIN
/**
    \ingroup plugin

    \brief base class for the product cache

    This is the base class for all product caches. Product caches are used to
    store results of the factory plugin handler creation method in order to avoid
    re-creating the same objects over and over again.

    Normally a product cache is disabled, because most programs don't make use of the
    same filters over and over again.
*/
class EXPORT_CORE CProductCache
{
public:
       /**
          This constructor registers the cache in the global product cache map
          \param name name of the handled factory
       */
       CProductCache(const std::string& name);

       /** enable or disable the cache accululation according to the given flag
           If the flag is set to true, then all newly created objects will be added
           to  the cache, if it is set to false, all objects that are in the cache may be
           reused, but no new ones are added to the cache.
           If you want to truely not use uncached values, you must clear the cache by calling clear()
           and disable it.
           \param enable
        */
       void enable_write(bool enable);

       /// clear the cache
       void clear();
protected:
       /// \returns whether this cache is enabled
       bool is_enabled() const;
private:
       virtual void do_clear() = 0;
       bool m_enabled;
       mutable CMutex m_enable_mutex;
};


/**
   \brief The type specific product cache

   \tparam ProductPtr the shared pointer of the product that is actually stored in this cache.

 */
template <typename ProductPtr>
class EXPORT_CORE TProductCache: public CProductCache
{
public:


       /**
          Constructor
        */
       TProductCache(const std::string& descriptor);


       /**
          Get a profuct if it is cached,
          \param name the initializer string
          \returns the shared product pointer, if available, or an empty shared pointer
        */
       ProductPtr get(const std::string& name) const;


       /**
          Add a product pointer to the cache if the cache is enabled
          \param name initializer string of the product
          \param product the shared pointer of the product to be added
        */
       void add(const std::string& name, ProductPtr product);
private:

       virtual void do_clear();

       typedef std::map<std::string, ProductPtr> CMap;
       CMap m_cache;
       mutable CRecursiveMutex m_cache_mutex;
};


/**
   \brief The singleton that handles all product caches

*/

class EXPORT_CORE CProductCacheHandler
{
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
       void register_cache(const std::string& name,  CProductCache *cache);
private:
       CProductCacheHandler();

       CProductCacheHandler(const CProductCacheHandler& other) = delete;
       CProductCacheHandler& operator = (const CProductCacheHandler& other) = delete;

       std::map<std::string, CProductCache *> m_caches;
       static CMutex m_creation_mutex;
};


/////////////////////////

template <typename ProductPtr>
TProductCache<ProductPtr>::TProductCache(const std::string& descriptor):
       CProductCache(descriptor)
{
}

template <typename ProductPtr>
ProductPtr TProductCache<ProductPtr>::get(const std::string& name) const
{
       CRecursiveScopedLock lock(m_cache_mutex);
       auto i = m_cache.find(name);

       if (i != m_cache.end())
              return i->second;

       return ProductPtr();
}

template <typename ProductPtr>
void TProductCache<ProductPtr>::add(const std::string& name, ProductPtr product)
{
       if (is_enabled()) {
              CRecursiveScopedLock lock(m_cache_mutex);

              //re-check whether another thead already added the product
              if (!get(name))
                     m_cache[name] = product;
       }
}

template <typename ProductPtr>
void TProductCache<ProductPtr>::do_clear()
{
       CRecursiveScopedLock lock(m_cache_mutex);
       m_cache.clear();
}

NS_MIA_END

#endif
