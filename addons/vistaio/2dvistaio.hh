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

#include <mia/2d/imageio.hh>

NS_BEGIN(vista_2d_io)



class CVista2DImageIOPlugin : public mia::C2DImageIOPlugin {
public:
	CVista2DImageIOPlugin();
private:
        PData do_load(const std::string& fname) const;
	bool do_save(const std::string& fname, const Data& data) const;
	const std::string do_get_descr() const;
	const std::string do_get_preferred_suffix() const; 
};

NS_END
