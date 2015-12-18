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


#include <mia/core/defines.hh>

#ifdef HAVE_TBB
#include <tbb/task_scheduler_init.h>
#include <tbb/recursive_mutex.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#else  // use C++ 11 thread 
#include <thread>
#include <atomic>
NS_MIA_BEGIN

template <typename C> 
class C1DParallelRange {
public: 
	C1DParallelRange(C begin, C end, C stride):
		m_begin(begin),
		m_end(end),
		m_stride(stride), 
		m_current_wp(0)
		{
		}

	
	C1DParallelRange get_next_workpackage()	{
		int wp = m_current_wp++;
		C begin = m_begin + wp * m_stride;
		C end = begin + m_stride;
		if (begin > m_end) {
			return C1DParallelRange(m_end,m_end,0);
		}
		if (end > m_end) {
			return C1DParallelRange(begin, m_end, 1);
		}
		return C1DParallelRange(begin, end, 1);
	}

	bool empty() const {
		return m_begin >= m_end; 
	}
	
private:
	C m_begin;
	C m_end;
	C m_stride;
	std::atomic<int> m_current_wp;
}; 



template <typename Range, typename func>
void pfor_callback(const Range& range, func f)
{
	Range wp = range.get_next_workpackage();
	if (!wp.empty()) 
		f(wp);
}

template <typename Range, typename func>
void pfor(const Range& range, func f) {
	
	int max_treads = CMaxThreads::get();

	std::vector<std::thread> threads;
	for (int i = 0; i < max_treads; ++i) {
		threads.push_back(std::thread(pfor_callback, range, f)); 
	}
	
	for (int i = 0; i < max_treads; ++i) {
		threads[i].join(); 
	}
}; 

#endif








NS_MIA_END

#endif 
