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

#ifndef mia_2d_combiner_ops_hh
#define mia_2d_combiner_ops_hh

#include <mia/2d/2dfilter.hh>
#include <mia/2d/combiner/plugin.hh>


NS_BEGIN(Combiner2d)


template <typename CombineOP>
class T2DImageCombiner: public mia::C2DImageCombiner {
	
	template <typename F, typename A, typename B>
	friend typename F::result_type mia::_filter(const F& f, const A& a, const B& b); 
	
	template <typename T, typename S>
	mia::P2DImage operator () ( const mia::T2DImage<T>& a, const mia::T2DImage<S>& b) const;

	mia::P2DImage do_combine( const mia::C2DImage& a, const mia::C2DImage& b) const;
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
	typedef T2DImageCombiner<Combine##NAME> C2D##NAME##ImageCombiner;

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

typedef T2DImageCombiner<CombineAbsDiff> C2DAbsDiffImageCombiner;

NS_END

#endif
