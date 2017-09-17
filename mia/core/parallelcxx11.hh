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

#ifndef mia_core_parallelcxx11_hh
#define mia_core_parallelcxx11_hh

#include <mia/core/defines.hh>

#include <thread>
#include <atomic>
#include <mutex>
#include <cassert>
#include <vector>

NS_MIA_BEGIN

typedef std::mutex CMutex;
typedef std::recursive_mutex CRecursiveMutex; 


class EXPORT_CORE CMaxTasks {
public:
	static int get_max_tasks(); 
	static void set_max_tasks(int mt); 
private:
	static int max_tasks; 
}; 

#define ATOMIC std::atomic

template <typename Mutex>
class TScopedLock {
public:
	TScopedLock(Mutex& m): m_mutex(m){
		m_mutex.lock();
		own_lock = true; 
	};

	TScopedLock(const TScopedLock<Mutex>& other) = delete;
	TScopedLock& operator = (const TScopedLock<Mutex>& other) = delete;

	~TScopedLock(){
		if (own_lock) 
			m_mutex.unlock();
	};

	void release() {
		if (own_lock) {
			own_lock = false; 
			m_mutex.unlock();
		}
	}
private:
	Mutex& m_mutex;
	bool own_lock;
};

typedef TScopedLock<CMutex> CScopedLock;
typedef TScopedLock<CRecursiveMutex> CRecursiveScopedLock;

class EXPORT_CORE C1DParallelRange {
public: 
	C1DParallelRange(int begin, int end, int block = 1):
		m_begin(begin),
		m_end(end),
		m_block(block), 
		m_current_wp(0)
		{
			assert(begin <= end); 
		}

	C1DParallelRange(const C1DParallelRange& orig):
		m_begin(orig.m_begin),
		m_end(orig.m_end),
		m_block(orig.m_block)
		{
			m_current_wp = orig.m_current_wp.load();
		}
	
	C1DParallelRange get_next_workpackage()	{
		int wp = m_current_wp++;
		int begin = m_begin + wp * m_block;
		int end = begin + m_block;
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

	int begin() const {
		return m_begin; 
	}

	int end() const {
		return m_end; 
	}

private:
	int m_begin;
	int m_end;
	int m_block;
	std::atomic<int> m_current_wp;
}; 

// The functor f must actually be passed by value because a copy must 
// be used. 
//coverity[PASS_BY_VALUE]
template <typename Range, typename Func>
void pfor_callback(Range& range, Func f)
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
void pfor(Range range, const Func& f) {
	
	int max_treads = CMaxTasks::get_max_tasks(); 
	
	std::thread::hardware_concurrency();

	std::vector<std::thread> threads;
	for (int i = 0; i < max_treads; ++i) {
		threads.push_back(std::thread(pfor_callback<Range, Func>, std::ref(range), f)); 
	}
	
	for (int i = 0; i < max_treads; ++i) {
		threads[i].join(); 
	}
}; 

template <typename V>
class ReduceValue {
public:
	typedef V Value; 
	ReduceValue(const Value& i):identity(i), value(i) {
	}
	
	template <typename Reduce> 
	void reduce(const Value& v, Reduce r)
	{
		CScopedLock sl(mutex);
		value = r(v, value); 
	}
	const Value& get_identity() const {
		return identity;
	}
	const Value& get_reduced() const {
		return value; 
	}
private: 
	mutable CMutex mutex;
	Value identity; 
	Value value; 
}; 

// The functor f must actually be passed by value because a copy must 
// be used. 
//coverity[PASS_BY_VALUE]
template <typename Range, typename Value, typename Func, typename Reduce>
void preduce_callback(Range& range, ReduceValue<Value>& v, Func f, Reduce r)
{
	Value value = v.get_identity(); 
	while (true)  {
		Range wp = range.get_next_workpackage();
		if (!wp.empty()) 
			value = f(wp, value);
		else
			break;
	}
	v.reduce(value, r);
}

template <typename Range, typename Value, typename Func, typename Reduce>
Value preduce(Range range, Value identity, const Func&  f, Reduce r)
{
	int max_treads = CMaxTasks::get_max_tasks();

	ReduceValue<Value> value(identity); 
		
	std::vector<std::thread> threads;
	for (int i = 0; i < max_treads; ++i) {
		threads.push_back(std::thread(preduce_callback<Range, Value, Func, Reduce>,
					      std::ref(range), std::ref(value), f, r)); 
	}
	
	for (int i = 0; i < max_treads; ++i) {
		threads[i].join(); 
	}
	return value.get_reduced(); 
}; 

NS_MIA_END 

#endif
