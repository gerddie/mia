/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef SegSet_h
#define SegSet_h

#include <mia/core/ioplugin.hh>
#include <mia/core/xmlinterface.hh>
#include <mia/2d/segframe.hh>
#include <mia/2d/boundingbox.hh>

NS_MIA_BEGIN



/**
   @ingroup perf 
   \brief A set of segmentation of a 2D series of perfusion images 
   
   A set of slices containing segmentation information specifically designed for 
   myocardial perfusion image series. 
*/
class EXPORT_2D CSegSet: public CIOData {
public:
	/// convenience typedef for the frames comprising a segmentation set 
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
	CSegSet(const CXMLDocument& node);


	CSegSet(int version);
	/**
	   Append a segmentation frame 
	   \param frame 
	 */
	void add_frame(const CSegFrame& frame);

	/**
	   Write the segmentation information to an XML tree 
	   \returns root node of xml tree. 
	 */
	CXMLDocument write() const;


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
	   \remark This function does too many things at once. 
	 */
	CSegSet  shift_and_rename(size_t skip, const C2DFVector&  shift, const std::string& new_filename_base)const;

	/**
	   Transform the segmentations slice wise by using the given transformation 
	   Wroks in-place. 
	   \param t tranformation 
	 */
	void transform(const C2DTransformation& t);


	/**
	   Set the frame number of the RV peak enhancement
	   \param peak 
	*/
	void set_RV_peak(int peak); 
	
	/**
	   \\returns the frame number of the RV peak enhancement (-1 if not set)
	*/
	int get_RV_peak() const; 

	/**
	   Set the frame number of the LV peak enhancement
	   \param peak 
	*/
	void set_LV_peak(int peak); 

	/**
	   \\returns the frame number of the LV peak enhancement (-1 if not set)
	*/
	int get_LV_peak() const; 

	/**
	   \\returns the frame number of the image that should be used as reference frame 
	   for time-intensity analysis after motion compensation - if the used motion compensation 
	   algorithm provides some (like quasiperiodic, one2many, or serial do). Returns -1 if no values is given. 
	*/
	int get_preferred_reference() const; 


	/**
	   Set the preferred reference frame for this segmentation set. 
	   \param value 
	 */
	void  set_preferred_reference(int value); 

private:
	void read(const CXMLDocument& node);
	Frames m_frames;
	int m_RV_peak; 
	int m_LV_peak; 
	int m_preferred_reference; 
	int m_version; 
};


NS_MIA_END

#endif
