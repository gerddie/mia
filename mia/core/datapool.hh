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

#ifndef mia_core_datapool_hh
#define mia_core_datapool_hh

#include <map>
#include <boost/any.hpp>

#include <mia/core/parallel.hh>

NS_MIA_BEGIN

/**
   \ingroup io

   \brief temporary data storage to avoid storing on disk

   This class implements a data storage pool as a singelton
   that can be used to store some
   data temporarly like if it would be stored on disk.
   Each IO plugin automatically defines an interface to store in and load
   from this pool. The extension, by which this behaviour is triggered is ".@".

   \todo add delete key function,
   \todo add automatic key generation
   \todo (maybe) all plugins that load data should load to the pool first to enable
   loading from temporarly generated data
*/

class EXPORT_CORE CDatapool
{
public:

       /**
          add some data to the pool, if the key already exists, the value  is replaced
          \param key unique identifier
          \param value value to be stored
        */
       void add(const std::string& key, boost::any value);
       /**
          \param key: key of data to be retrieved
          \returns value indexed by key, and throws \a invalid argument if value not exists
        */
       boost::any get(const std::string& key) const;

       /**
          gets some data and remove this data from the pool
          \param key key of data to be retrieved
          \returns value indexed by key, and throws \a invalid argument if value not exists
        */
       boost::any get_and_remove(const std::string& key);

       /**
          remove this data from the pool
          \param key key of data to be retrieved
       */
       void remove(const std::string& key);


       /**
          \param key
          \returns \a true if key exists in pool and \a false if not
       */
       bool has_key(const std::string& key) const;
       /**
          \returns the unique instance of the data pool
        */
       static CDatapool& instance();

       /**
          function to be used for debugging purpouses
          \returns true if the pool has some data that was not uses
       */
       bool has_unused_data() const;

       /**
          Remove all stored data from the pool.
        */
       void clear();
private:

       CDatapool();
       CDatapool(const CDatapool& other);
       typedef std::map<std::string, boost::any> Anymap;
       typedef Anymap::const_iterator const_iterator;
       const_iterator get_iterator(const std::string& key) const;
       Anymap m_map;
       typedef std::map<std::string, bool> Usagemap;
       mutable Usagemap m_usage;
       static CRecursiveMutex m_mutex;
};

NS_MIA_END

#endif
