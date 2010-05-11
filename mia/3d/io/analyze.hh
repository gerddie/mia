/* -*- mia-c++  -*-
 *
 * Copyright (c) 2004-2009 Gert Wollny <gert at die.upm.es>
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Biomedical Image Technologies, Universidad Politecnica de Madrid
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


#ifndef mia_3d_io_analyze_hh
#define mia_3d_io_analyze_hh

#include <mia/3d/3dimageio.hh>

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
	virtual void do_add_suffixes(std::multimap<std::string, std::string>& map) const;
	virtual const std::string do_get_descr() const; 

	void swap_hdr(analyze_dsr& hdr) const; 

	template <bool flipped> 
	mia::C3DImage *read_image(const mia::C3DBounds& size, short datatype, mia::CInputFile& data_file)const; 

	bool save_data(const std::string& fname, const Data& data, analyze_image_dimension& dime) const; 
	const mia::TDictMap<mia::EPixelType> _M_type_table; 
	mutable bool _M_swap_endian; 
};

NS_END

#endif
