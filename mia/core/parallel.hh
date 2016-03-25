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

#ifndef mia_core_parallel_hh
#define mia_core_parallel_hh


#include <miaconfig.h>

#include <mia/core/defines.hh>

#ifdef HAVE_TBB

#include <tbb/task_scheduler_init.h>
#include <tbb/recursive_mutex.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <tbb/recursive_mutex.h>
#include <tbb/spin_mutex.h>

NS_MIA_BEGIN

typedef tbb::blocked_range<int> C1DParallelRange;
typedef tbb::mutex CMutex; 
typedef tbb::mutex::scoped_lock CScopedLock;

typedef tbb::recursive_mutex CRecursiveMutex; 
typedef tbb::recursive_mutex::scoped_lock CRecursiveScopedLock; 

#define ATOMIC tbb::atomic

template <typename Range, typename Func>
void pfor(const Range& range, Func body) {
	tbb::parallel_for(range, body); 
}

template<typename Range, typename Value, 
         typename Func, typename Reduction>
Value preduce( const Range& range, const Value& identity,
	       const Func& func, const Reduction& reduction) {
	return tbb::parallel_reduce(range, identity, func, reduction); 
}; 

NS_MIA_END

#else  // no TBB: use C++ 11 thread

#include <mia/core/parallelcxx11.hh>

#endif


#endif 
