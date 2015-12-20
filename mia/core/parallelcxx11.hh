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

#ifndef mia_core_parallelcxx11_hh
#define mia_core_parallelcxx11_hh

#include <mia/core/defines.hh>

#include <thread>
#include <atomic>
#include <mutex>

NS_MIA_BEGIN

typedef std::mutex CMutex;

class CScopedLock {
public:
	CScopedLock(CMutex& m): m_mutex(m){
		m.lock();
	};
	~CScopedLock(){
		m.unlock();
	};
private:
	CMutex& m_mutex;
}
	
class C1DParallelRange {
public: 
	C1DParallelRange(int begin, int end, int stride):
		m_begin(begin),
		m_end(end),
		m_stride(stride), 
		m_current_wp(0)
		{
		}

	
	C1DParallelRange get_next_workpackage()	{
		int wp = m_current_wp++;
		int begin = m_begin + wp * m_stride;
		int end = begin + m_stride;
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
	int m_begin;
	int m_end;
	int m_stride;
	std::atomic<int> m_current_wp;
}; 

template <typename Range, typename Func>
void pfor_callback(const Range& range, Func f)
{
	while (true)  {
		Range wp = range.get_next_workpackage();
		if (!wp.empty()) 
			f(wp);
		else
			break;
	}
}

template <typename Range, typename Func>
void pfor(const Range& range, Func f) {
	
	int max_treads = CMaxThreads::get();

	std::vector<std::thread> threads;
	for (int i = 0; i < max_treads; ++i) {
		threads.push_back(std::thread(pfor_callback, range, f)); 
	}
	
	for (int i = 0; i < max_treads; ++i) {
		threads[i].join(); 
	}
}; 

template <typename Value>
class ReduceValue {
public: 
	ReduceValue(Value& v):value(v) {
	}

	Value get() const
	{
		CScopedLock (mutex);
		return value; 
	}
	void set(const Value& v) const
	{
		CScopedLock (mutex);
		value = v; 
	}
private: 
	CMutex mutex;
	Value& value; 
}

template <typename Range, typename Value, typename Func, typename Reduce>
void pfor_reduce_callback(const Range& range, ReduceValue& v, Func f, Reduce r)
{
	ReduceValue::Value value; 
	while (true)  {
		Range wp = range.get_next_workpackage();
		if (!wp.empty()) 
			value = f(wp, value);
		else
			break;
	}
	v.set(r(value, v.get())); 
}

template <typename Range, typename RValue, typename Func, typename Reduce>
Value preduce(const Range& range, Value init, Func f, Reduce r)
{
	int max_treads = CMaxThreads::get();

	ReduceValue value(init); 
		
	std::vector<std::thread> threads;
	for (int i = 0; i < max_treads; ++i) {
		threads.push_back(std::thread(pfor_callback, range, init, f, r)); 
	}
	
	for (int i = 0; i < max_treads; ++i) {
		threads[i].join(); 
	}
	return init; 
}; 

NS_MIA_END 

#endif
