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


#include <mia/2d/combiner/ops.hh>

NS_BEGIN(Combiner2d)

using namespace mia; 
using namespace std; 

template <typename BO>
template <typename T, typename S>
P2DImage T2DImageCombiner<BO>::operator () ( const T2DImage<T>& a, const T2DImage<S>& b) const
{
	// there's got to be a better way ...
	BO bo; 
	typedef decltype(bo(a[0], b[0])) R; 
			
	auto  r = new T2DImage<R>(a.get_size()); 
	
	P2DImage result(r);

	transform(a.begin(), a.end(), b.begin(), r->begin(), bo); 
	return result; 
}

template <typename BO>
P2DImage T2DImageCombiner<BO>::do_combine( const C2DImage& a, const C2DImage& b) const
{
	if (a.get_size() != b.get_size()) {
		throw create_exception<invalid_argument>("C2DAddImageCombiner: input images have different size: ", 
					       a.get_size(), " vs. ", b.get_size());
	}
	return mia::filter(*this, a, b); 
}

template class T2DImageCombiner<CombineAdd>; 
template class T2DImageCombiner<CombineSub>; 
template class T2DImageCombiner<CombineTimes>; 
template class T2DImageCombiner<CombineDiv>; 
template class T2DImageCombiner<CombineAbsDiff>; 

NS_END

NS_MIA_BEGIN
using namespace Combiner2d; 

extern const char pl_add[] = "add"; 
extern const char pl_sub[] = "sub"; 
extern const char pl_mul[] = "mul"; 
extern const char pl_div[] = "div"; 
extern const char pl_absdiff[] = "absdiff"; 


template class  T2DImageCombinerPlugin<T2DImageCombiner<CombineAdd>, pl_add>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<CombineSub>, pl_sub>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<CombineTimes>, pl_mul>;
template class  T2DImageCombinerPlugin<T2DImageCombiner<CombineDiv>, pl_div>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<CombineAbsDiff>, pl_absdiff>; 

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *retval = new T2DImageCombinerPlugin<T2DImageCombiner<CombineAdd>, pl_add>();
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<CombineSub>, pl_sub>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<CombineTimes>, pl_mul>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<CombineDiv>, pl_div>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<CombineAbsDiff>, pl_absdiff>); 
	return retval;  
}

NS_MIA_END
