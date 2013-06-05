/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_3d_combiner_ops_hh
#define mia_3d_combiner_ops_hh

#include <mia/3d/filter.hh>
#include <mia/3d/combiner/plugin.hh>


NS_BEGIN(Combiner3d)

class C3DImageOpCombinerResult: public mia::CCombinerResult {
public:
	C3DImageOpCombinerResult(mia::P3DImage result);
private:
	virtual void do_save(const std::string& fname) const;
	virtual boost::any do_get() const; 
	mia::P3DImage m_result;
};


template <typename CombineOP>
class T3DImageCombiner: public mia::C3DImageCombiner {
	
	template <typename F, typename A, typename B>
	friend typename F::result_type mia::_filter(const F& f, const A& a, const B& b); 
	
	template <typename T, typename S>
	mia::PCombinerResult operator () ( const mia::T3DImage<T>& a, const mia::T3DImage<S>& b) const;

	mia::PCombinerResult do_combine( const mia::C3DImage& a, const mia::C3DImage& b) const;
}; 


#define COMBINE_OP(NAME, op) \
	template <typename A, typename B>	\
	struct __Combine##NAME {					\
		typedef decltype(*(A*)0 op *(B*)0) return_type; \
		static return_type apply(A a, B b) {		\
			return a op b;				\
		}						\
	};							\
								\
	class Combine##NAME {					\
        public:									\
	template <typename A, typename B>				\
	typename __Combine##NAME<A,B>::return_type operator ()(A a, B b)const { \
		return __Combine##NAME<A,B>::apply(a,b);			\
	   }								\
	};								\
	typedef T3DImageCombiner<Combine##NAME> C3D##NAME##ImageCombiner;

COMBINE_OP(Add,   +)
COMBINE_OP(Sub,   -)
COMBINE_OP(Times, *)
COMBINE_OP(Div,   /)


template <typename A, typename B>
struct __CombineAbsDiff {
	typedef decltype(*(A*)0 - *(B*)0) return_type; 
		static return_type apply(A a, B b) {	       
			return static_cast<double>(a) > static_cast<double>(b) ? (a - b) : (b - a); 
		}
};						

class CombineAbsDiff {							
public:									
	template <typename A, typename B>
	typename __CombineAbsDiff<A,B>::return_type operator ()(A a, B b)const {
		return __CombineAbsDiff<A,B>::apply(a,b);
	}
};									\

typedef T3DImageCombiner<CombineAbsDiff> C3DAbsDiffImageCombiner;

NS_END

#endif
