/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

#ifndef SegFrame_h
#define SegFrame_h

#include <vector>
#include <mia/2d/SegStar.hh>
#include <mia/2d/SegSection.hh>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

/**
   \brief A class to represent one segmented frame in a heart perfusion series 
   
   This class implements the frame of a myocardial segmentation consisting of 
   six sections CSegsection, the segmentation helper CSegStar, and the name of the 
   corresponding image file.  
*/
class  EXPORT_2D CSegFrame {
public:
	/// convenience typedef for the sections 
	typedef std::vector<CSegSection> Sections;
	
	/** convenience typedef for the statistics values  
	    @remark maybe a type with more meaningful elements would be better 
	 */ 
	typedef std::pair<float, float> Statistics; 
	
	/// convenience typedef for the section statistics vector 
	typedef std::vector<Statistics> SectionsStats; 
	
	
	CSegFrame();

	/**
	   Construct the segmentation frame from 
	   \param image image file name 
	   \param star CSegStar 
	   \param sections the segmentation sections 
	 */
	CSegFrame(const std::string& image, const CSegStar& star, const Sections& sections);

	/**
	   Construct the segmentation frame from a XML root node
	   \param node
	 */
	CSegFrame(const xmlpp::Node& node);

	/// \returns the file name of the corresponding image 
	const std::string& get_imagename() const;

	/** set the file name of the corresponding image 
	    \param name 
	 */
	void set_imagename(const std::string& name);

	/** rename the file name base of the image according to 
	    sed -e "s/.*[^0-9]\([0-9]*\..*\)/$new_base\1/"
	    \param new_base
	 */
	void rename_base(const std::string& new_base); 

	/// \returns a read-only reference to the segmentation sections 
	const Sections& get_sections() const;

	/**
	   Set the image corresponding to the segmentation frame 
	   @param image 
	 */
	void set_image(P2DImage image); 

	/**
	   @returns the star of the LV contained in this frame 
	 */
	const CSegStar& get_star() const;

	/// @returns the bounding box enclosing all segmentation sections belonging to thie frame 
	const C2DBoundingBox get_boundingbox() const;


	/**
	   Append the segmentation frame to a XML node 
	   @param node parent node to append the frame description to 
	 */
	void write(xmlpp::Node& node) const;

	/**
	   Shift the segmentation frame and change the file name to the new name 
	   corresponding to the shifted image 
	   @param delta translation 
	   @param cropped_file new image file name 
	*/
	void shift(const C2DFVector& delta, const std::string& cropped_file);


	/**
	   transform the frame segmentation by a given transformation 
	   @param t 
	 */
	void transform(const C2DTransformation& t);

	/**
	   transform the frame segmentation by the inverse of the given transformation 
	   @param t 
	 */
	void inv_transform(const C2DTransformation& t);

	/**
	   Evaluate the Hausdorff distance of this segmentation frame to another 
	   @param other 
	   @returns Hausdorff distance
	 */
	float get_hausdorff_distance(const CSegFrame& other) const;


	/**
	   Evaluate a mask image based on the segmented sections
	   @param size size of the output image 
	   @returns an image containing the masks for each section  numbered in the storage 
	   order of the sections
	   @remark If overlap exists between the sections the masks with a higher index overwrite 
	   the masks with a lower index. 
	 */
	C2DUBImage get_section_masks(const C2DBounds& size) const; 

	/**
	   Create the section masks by using the size of the image corresponding to the frame 
	   @returns the mask image, for details see get_section_masks(const C2DBounds& size). 
	 */
	C2DUBImage get_section_masks() const; 

	/**
	   Create the section masks by using the size of the image corresponding to the frame. 
	   If the number of requested sections is equal to the number of sections stored, this 
	   call is equal to get_section_masks(), 
           Otherwiese, instead of using the sections as defined, evaluate the union of all the sections 
	   and then split this union evenly in \a n_sections starting by the first directional ray 
	   and moving clockwiese with the star center as the angular point. 
	   @param n_sections number of target sections 
	   @returns the mask image, for details see get_section_masks(const C2DBounds& size). 
	 */
	C2DUBImage get_section_masks(size_t n_sections) const; 


	/**
	   Evaluate inetnsity mean and variation of the image data for the registions  
	   defined by the given mask image. 
	   @param mask 
	   @returns the statustics 
	 */
	SectionsStats get_stats(const C2DUBImage& mask) const; 
	
	/**
	   Evaluate inetnsity mean and variation of the image data for the registions  
	   defined the get_section_masks(size_t n_sections) method. 
	   @param n_sections 
	   @returns the statustics 
	 */
	SectionsStats get_stats(size_t n_sections) const; 
private:
	void load_image() const; 

	bool m_has_star;
	CSegStar m_star;
	Sections m_sections;
	std::string m_filename;
	mutable P2DImage m_image; 
};

NS_MIA_END

#endif

