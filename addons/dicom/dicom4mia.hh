/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef dicom4mia_hh
#define dicom4mia_hh

#include <mia/core/attributes.hh>
#include <mia/core/attribute_names.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/image.hh>
#include <mia/3d/image.hh>

#ifdef __GNUC__
#  define EXPORT_DICOM __attribute__((visibility("default")))
#else 
#  define EXPORT_DICOM
#endif

NS_MIA_BEGIN

class EXPORT_DICOM CDicomReader {
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
	C3DFVector get_voxel_size(bool warn_no_z) const;

	P2DImage get_image() const;

	bool has_3dimage() const; 

	P3DImage get_3dimage() const; 

	int get_number_of_frames() const; 
private:
	template <typename T> P3DImage load_image3d()const;
	template <typename T> P2DImage load_image()const;


	struct CDicomReaderData *impl;
	std::string m_filename;

};

class EXPORT_DICOM CDicomWriter {
public:
	CDicomWriter(const C2DImage& image);
	~CDicomWriter();

	bool write(const char *filename) const;

	friend CDicomReader ugly_trick_writer_dcm_to_reader_dcm(CDicomWriter& writer);
private:
	struct CDicomWriterData *impl;
};

#ifdef ENABLE_TEST_HACKS
CDicomReader EXPORT_DICOM ugly_trick_writer_dcm_to_reader_dcm(CDicomWriter& writer);
#endif

NS_MIA_END
#endif
