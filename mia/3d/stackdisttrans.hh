/* -*- mia-c++ -*- 
 *
 * Copyright (c) 2004-2011 
 *
 * Max-Planck-Institute of Evolutionary Anthropology, Leipzg 
 * ETSI Telecomunicacion, UPM, Madrid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#ifndef mia_3d_stackdisttrans_hh
#define mia_3d_stackdisttrans_hh

#include <vector>
#include <mia/3d/3DVector.hh>
#include <mia/2d/2DImage.hh>
#include <mia/core/iohandler.hh>
#include <mia/core/ioplugin.hh>

NS_MIA_BEGIN

/**
   \brief A distance transform that stores distances to a 3D binary mask given as slices.  

   This class implements a 3D distance transform that makes it possible to 
   evaluate the distance of each point in a 3D volume to a binary mask 
   that is given by a set of 2D binary images that are interpreted like a 
   3D volume. 
   It implements the Techical Report 
     Pedro F. Felzenszwalb and Daniel P. Huttenlocher
     Distance Transforms of Sampled Functions
     Cornell Computing and Information Science TR2004-1963
*/

class C2DStackDistanceTransform: public CIOData {
public: 
	typedef C2DStackDistanceTransform type; 

	static const char * const value; 
	static const char * const type_descr; 


	/**
	   Return type for the result of the evaluated distances
	 */
	struct DistanceFromPoint {
		DistanceFromPoint(C3DBounds _point, float _distance):
			point(_point), 
			distance(_distance) 
			{}; 
		C3DBounds point; /*!< location of the point */
		float distance;  /*!< distance of the point to the mask */
	}; 

	
	C2DStackDistanceTransform() = default; 
	C2DStackDistanceTransform(const C2DStackDistanceTransform& other) = default; 

	/**
	   Create a copy of this object 
	 */
	C2DStackDistanceTransform *clone() const __attribute__((warn_unused_result)); 

	/**
	   Construct the distance grid and initialize it
	   \param size xy-slice of the data to be processed
	   \param voxel_size size of a voxel
	 */

	C2DStackDistanceTransform(const C2DBounds& size, const C3DFVector& voxel_size = C3DFVector::_1);
	
	/**
	   Read the given slice of the image stack and update the distance maps. 
	   \param slice the mask data 
	   \param q the z-position of the slice
	 */

	void read( const C2DImage& slice, int q); 
	

	/**
	   Evaluate the distances for a given input mask. For each mask point in image 
	   that is set to thrue the distance is evaluated. 
	   \param s z-location of the slice 
	   \param image the actual mask 
	*/
	std::vector<DistanceFromPoint> get_slice(size_t s, const C2DImage& image) const; 

	/**
	   Save the distcane transform to a file. 
	   \param filename The name of the file to save the transform to 
	   \return true if sucessful
	*/
	bool save(const std::string& filename); 

private: 	
	
	float d(float fq, float q, float fv, float v)const; 
	
	void dt1d(vector<float>& f, float scale)const; 
	void dt2d(C2DFImage& image)const; 
	
	struct SParabola {
		int k; 
		int v; 
		float z;
		float fv; 
	};
	
	C2DBounds m_size; 
	C3DFVector m_voxel_size; 
	vector<int> m_k; 
	vector< vector<SParabola> > m_zdt;
	
};

/**
   Write a point-distance pair 
   \param os output stream 
   \param v the poin-distance 
   \returns the output stream 
*/

inline std::ostream& operator << (std::ostream& os, const C2DStackDistanceTransform::DistanceFromPoint& v) 
{
	os << "[" << v.point << ":" << v.distance << "]"; 
}

inline bool operator == (const C2DStackDistanceTransform::DistanceFromPoint& lhs, 
			 const C2DStackDistanceTransform::DistanceFromPoint& rhs) 
{
	return (lhs.point == rhs.point) && (lhs.distance == rhs.distance);
}


/// Base class for the generic IO of transformations 
typedef TIOPlugin<C2DStackDistanceTransform> C2DStackDistanceTransformIO; 

/// Plug-in handler for the transformation IO plug-ins 
typedef TIOHandlerSingleton< TIOPluginHandler<C2DStackDistanceTransformIO> > C2DStackDistanceTransformIOPluginHandler;

/// data key type for temporary storage of 3D transformations \sa CDatapool 
typedef C2DStackDistanceTransformIOPluginHandler::Instance::DataKey C2DStackDistanceTransformIODataKey;

NS_MIA_END

#endif
