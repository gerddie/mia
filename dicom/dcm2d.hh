/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/2d/2dimageio.hh>

NS_BEGIN(IMAGEIO_2D_DICOM)

class CDicom2DImageIOPlugin: public mia::C2DImageIOPlugin {
public:
	CDicom2DImageIOPlugin();
	typedef mia::C2DImageIOPlugin::Data Data;
	typedef mia::C2DImageIOPlugin::PData PData;

private:
	PData do_load(const std::string& fname) const;
	bool do_save(const std::string& fname, const Data& data) const;
	const std::string do_get_descr() const;
};

NS_END
