/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007-2009 Gert Wollny <gert dot wollny at acm dot org>
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

#ifndef dicom4mia_hh
#define dicom4mia_hh

#include <mia/core/attributes.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2DImage.hh>


NS_MIA_BEGIN

extern const char * IDAcquisitionDate; 
extern const char * IDAcquisitionNumber; 
extern const char * IDImageType; 
extern const char * IDInstanceNumber; 
extern const char * IDMediaStorageSOPClassUID; 
extern const char * IDModality; 
extern const char * IDPatientOrientation; 
extern const char * IDPatientPosition; 
extern const char * IDStudyDescription; 
extern const char * IDSamplesPerPixel; 
extern const char * IDSeriesDescription; 
extern const char * IDSeriesNumber; 
extern const char * IDSliceLocation; 
extern const char * IDStudyID; 
extern const char * IDTestValue; 
extern const char * IDSOPClassUID; 
extern const char * IDSmallestImagePixelValue; 
extern const char * IDLargestImagePixelValue; 

class CDicomReader {
public: 
	CDicomReader(const char *filename); 

	// this is here only for testing
	CDicomReader(struct CDicomReaderData *yeah); 
	
	~CDicomReader(); 

	bool good() const; 
	int rows() const; 
	int cols() const; 
	
	C2DBounds image_size()const; 
	
	int samples_per_pixel() const; 
	int bits_allocated() const; 
	int bits_used() const; 

	std::string get_attribute(const std::string& name, bool required)const; 
	C2DFVector get_pixel_size() const; 
	
	P2DImage get_image() const; 
private: 
	template <typename T> P2DImage load_image()const; 
	void add_attribute(C2DImage& image, const char *key, bool required) const; 

	struct CDicomReaderData *impl; 
	string m_filename; 

}; 

class CDicomWriter {
public: 
	CDicomWriter(const C2DImage& image); 
	bool write(const char *filename) const;

	friend CDicomReader ugly_trick_writer_dcm_to_reader_dcm(CDicomWriter& writer);
private: 
	struct CDicomWriterData *impl;
}; 

#ifdef ENABLE_TEST_HACKS
CDicomReader ugly_trick_writer_dcm_to_reader_dcm(CDicomWriter& writer);
#endif

NS_MIA_END
#endif
