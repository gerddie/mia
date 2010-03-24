/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
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

#include <mia/3d/3dimageio.hh>

NS_BEGIN(dicom_3dimageio)

class CDicom3DImageIOPlugin: public mia::C3DImageIOPlugin {
public: 
	CDicom3DImageIOPlugin();
	typedef mia::C3DImageIOPlugin::Data Data; 
	typedef mia::C3DImageIOPlugin::PData PData; 

private: 
	
	mia::C3DImageIOPlugin::PData get_images(const std::vector<mia::P2DImage>& candidates) const; 
	void do_add_suffixes(std::multimap<std::string, std::string>& map) const;
	PData do_load(const std::string& fname) const;
	bool do_save(const std::string& fname, const Data& data) const;
	const std::string do_get_descr() const; 
}; 

NS_END
