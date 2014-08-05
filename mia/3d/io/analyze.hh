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

#ifndef mia_3d_io_analyze_hh
#define mia_3d_io_analyze_hh

#include <mia/3d/imageio.hh>

NS_BEGIN(IMAGEIO_3D_ANALYZE)

struct analyze_image_dimension;
struct analyze_dsr;

class CAnalyze3DImageIOPlugin : public mia::C3DImageIOPlugin {
public:
	CAnalyze3DImageIOPlugin();
private:
	typedef mia::C3DImageIOPlugin::PData PData;
	typedef mia::C3DImageIOPlugin::Data Data;

	virtual PData do_load(const std::string&  filename) const;
	virtual bool do_save(const std::string& fname, const Data& data) const;
	virtual const std::string do_get_descr() const;
	const std::string do_get_preferred_suffix() const; 

	void swap_hdr(analyze_dsr& hdr) const;

	mia::C3DImage *read_image(const mia::C3DBounds& size, short datatype, mia::CInputFile& data_file)const;

	bool save_data(const std::string& fname, const Data& data, analyze_image_dimension& dime) const;
	const mia::TDictMap<mia::EPixelType> m_type_table;
	mutable bool m_swap_endian;
};

NS_END

#endif
