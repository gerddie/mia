/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


/* 
   LatexBeginPluginDescription{2D image combiner}
   
   \subsection{Arithmetic image combiners}
   \label{combiner2d:aritmetic}
   
   \begin{description}
   
   \item [Plugin:] add, sub, mul, div, absdiff
   \item [Description:] combine the two images by applying the corresponding arithmetic operaton. 
   The input images must be of the same size. 
   \item [Input1:] Abitrary gray scale or binary image 
   \item [Input2:] Abitrary gray scale or binary image 
   \item [Output:] The combined image of the pixel type that is deducted from the 
                   C++ return type for the corresponding operation between the input pixel types.
   \item [Remark:] No special error handling is implemented for the division by zero.  
   \end{description}
   These plug-ins don't take additional parameters

   LatexEnd  
 */


#include <boost/lambda/lambda.hpp>
#include <mia/2d/combiner/ops.hh>

NS_BEGIN(Combiner2d)

using namespace mia; 

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
		THROW(invalid_argument, "C2DAddImageCombiner: input images have different size: " 
		      << a.get_size() << " vs. "<< b.get_size() << "\n"); 
	}
	return mia::filter(*this, a, b); 
}

template class T2DImageCombiner<BinaryAdd>; 
template class T2DImageCombiner<BinarySub>; 
template class T2DImageCombiner<BinaryTimes>; 
template class T2DImageCombiner<BinaryDiv>; 
template class T2DImageCombiner<BinaryAbsDiff>; 

NS_END

NS_MIA_BEGIN
using namespace Combiner2d; 

extern const char pl_add[] = "add"; 
extern const char pl_sub[] = "sub"; 
extern const char pl_mul[] = "mul"; 
extern const char pl_div[] = "div"; 
extern const char pl_absdiff[] = "absdiff"; 


template class  T2DImageCombinerPlugin<T2DImageCombiner<BinaryAdd>, pl_add>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<BinarySub>, pl_sub>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<BinaryTimes>, pl_mul>;
template class  T2DImageCombinerPlugin<T2DImageCombiner<BinaryDiv>, pl_div>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<BinaryAbsDiff>, pl_absdiff>; 

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *retval = new T2DImageCombinerPlugin<T2DImageCombiner<BinaryAdd>, pl_add>();
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<BinarySub>, pl_sub>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<BinaryTimes>, pl_mul>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<BinaryDiv>, pl_div>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<BinaryAbsDiff>, pl_absdiff>); 
	return retval;  
}

NS_MIA_END
