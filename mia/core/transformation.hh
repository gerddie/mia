/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef mia_core_transformation_hh
#define mia_core_transformation_hh

#include <mia/core/iodata.hh>

#include <mia/core/shared_ptr.hh>
NS_MIA_BEGIN


/**
   Template of a generic tranformation function 
   \tparam D data type of the object to be transformed 
   \tparam I interpolator factory to create resampler for type D
   \todo the interpolator factory resp. th einterpolator kernel 
         should probably be part of the class 
 */

template <typename D, typename I>
class Transformation :public CIOData {
public: 

	/// interface type for plugin implementation and search 
	static const char *value; 
	
        virtual ~Transformation(); 

	typedef D Data; 
	typedef I Interpolator; 
	
	/** Apply the transformation to the input data 
	    \param input 
	    \param ipf interpolator factory 
	    \returns a shared pointer to the transformed input data
	*/
	std::shared_ptr<D > operator () (const D& input, const I& ipf) const; 
private: 
        virtual std::shared_ptr<D > apply(const D& input, const I& ipf) const = 0;

}; 

// implementation 

template <typename D, typename I>
Transformation<D, I>::~Transformation()
{
}

template <typename D, typename I>
std::shared_ptr<D > Transformation<D,I>::operator() (const D& input, const I& ipf) const
{
	return apply(input, ipf); 
}

template <typename D, typename I>
const char *Transformation<D, I>::value = "transform";

NS_MIA_END


#endif
