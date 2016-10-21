/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef addons_vtkvf_hh
#define addons_vtkvf_hh

#include <mia/3d/vfio.hh>

namespace vtkvf {

class CVtk3DVFIOPlugin : public mia::C3DVFIOPlugin {
public:
	CVtk3DVFIOPlugin();
private:

        virtual PData do_load(const std::string&  filename) const;
	virtual bool do_save(const std::string& fname, const Data& data) const;
virtual const std::string do_get_descr() const;
};

}

#endif 

