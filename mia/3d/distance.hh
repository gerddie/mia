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

#ifndef mia_3d_distance_hh
#define mia_3d_distance_hh

#include <mia/core/iodata.hh>
#include <mia/3d/3DImage.hh>


NS_MIA_BEGIN

/**
   This function evaluates the 3D distance transform of an input function. 
   If the input function is given as raw data that is not the result of another 
   call to this distance transform function, then one should call 
   distance_transform_prepare to properly prepare the data for the distance transform. 

   \param f squared values of the input function input 
   \returns the squared distance values for each grid point. 
*/

C3DFImage EXPORT_3D distance_transform(const C3DImage& f); 

/**
   \brief 3D distance transform for high resolution data 
   
   This class represents a 3D distance transform for high resolution data. Other than 
   distance_transform(const C3DImage& f), this class does not store the distances for the 
   whole grid. Instead the distances can ether be obtained slice-wise, or by evaluating 
   the distance at certain points. 

*/
class C3DDistance : public CIOData {
public: 

	/**
	   Initializes the distance transform 
	*/
	C3DDistance(); 

	~C3DDistance(); 

	C3DDistance(const C3DDistance& other) = delete;
	C3DDistance& operator =(const C3DDistance& other) = delete;
		
	/**
	   Add another z-slice to the distance transform. Note, the slices have to be 
	   added in the order of increasing z. 
	   Thr input data should be prepared using the distance_transform_prepare function. 
	   \param z current z-position 
	   \param slice of the function to measure the distance to 
	*/
	
	void push_slice(int z, const C2DImage& slice); 

	/**
	   Evaluate the distance of a point to the input function 
	   \param p point 
	   \returns the squared Euclidian distance of the point to the function 
	 */
	float get_distance_at(const C3DFVector& p) const;

	/**
	   Evaluate the distance of a slice to the represented input function 
	   \param z the slice to evaluate the distance to 
	   \returns a 2D data field of the squared distances
	*/
	C2DFImage get_distance_slice(int z) const; 
private: 
	struct C3DDistanceImpl *impl; 
}; 

NS_MIA_END

#endif 
