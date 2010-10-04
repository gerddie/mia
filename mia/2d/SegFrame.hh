/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
   Ths class implements the frame of a myocardial segmentation consisting of 
   six sections CSegsection, the segmentation helper CSegStar, and the name of the 
   corresponding image file.  
*/
class  EXPORT_2D CSegFrame {
public:
	typedef std::vector<CSegSection> Sections;
	typedef std::pair<float, float> Statistics; 
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

	void set_image(P2DImage image); 

	
	const CSegStar& get_star() const;

	const C2DBoundingBox get_boundingbox() const;

	void write(xmlpp::Node& node) const;

	void shift(const C2DFVector& delta, const std::string& cropped_file);

	void transform(const C2DTransformation& t);
	void inv_transform(const C2DTransformation& t);

	float get_hausdorff_distance(const CSegFrame& other) const;

	C2DUBImage get_section_masks(const C2DBounds& size) const; 
	C2DUBImage get_section_masks() const; 
	C2DUBImage get_section_masks(size_t n_sections) const; 

	SectionsStats get_stats(const C2DUBImage& mask) const; 
	SectionsStats get_stats() const; 

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

