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

#ifndef SegSet_h
#define SegSet_h

#include <mia/2d/SegFrame.hh>
#include <mia/2d/BoundingBox.hh>

namespace xmlpp {
	class Document;
};

NS_MIA_BEGIN

/**
   A set of slices containing segmentation information
   
 */
class EXPORT_2D CSegSet {
public:
	typedef std::vector<CSegFrame> Frames;

	/// Standard constructor 
	CSegSet();
	/**
	   Construct the segmentation set by reading from a file 
	   \param src_filename file name to read set from 
	 */
	CSegSet(const std::string& src_filename);

	/**
	   Construct a segmentation set by reading from a XML document
	   \param node the root node of the XML document 
	 */
	CSegSet(const xmlpp::Document& node);

	/**
	   Append a segmentation frame 
	   \param frame 
	 */
	void add_frame(const CSegFrame& frame);

	/**
	   Write the segmentation information to an XML tree 
	   \returns root node of xml tree. 
	 */
	xmlpp::Document *write() const;


	/// \returns read-only vector of the segmentation frames 
	const Frames& get_frames()const;

	/** 
	    \returns a reference to the read-write vector of the segmentation frames 
	    Changing this vector changes the segmentation set 
	*/ 
	Frames& get_frames();

	/**
	   \returns the box of minimal size that includes the segmentation 
	 */
	const C2DBoundingBox get_boundingbox() const;

	/**
	   Rename the base of the image file names for all frames on a frame by frame basis. 
	   \param new_base new base name 
	*/
	void rename_base(const std::string& new_base); 


	/**
	   This function renames the images files, shifts the origin of the segmentation and 
	   removes frames from the beginning of the set 
	   \param skip number of frames to skipü at the beginning 
	   \param shift new origin of segmentation 
	   \param  new_filename_base new file name base
	   \rename This function does too many things at once. 
	 */
	CSegSet  shift_and_rename(size_t skip, const C2DFVector&  shift, const std::string& new_filename_base)const;

	/**
	   Transform the segmentations slice wise by using the given transformation 
	   Wroks in-place. 
	   \param t tranformation 
	 */
	void transform(const C2DTransformation& t);

	void set_RV_peak(int peak); 
	int get_RV_peak() const; 

	void set_LV_peak(int peak); 
	int get_LV_peak() const; 

private:
	void read(const xmlpp::Document& node);
	Frames m_frames;
	int m_RV_peak; 
	int m_LV_peak; 
};


NS_MIA_END

#endif
