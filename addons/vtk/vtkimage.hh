/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef addons_vtkimage_hh
#define addons_vtkimage_hh

#include <mia/3d/imageio.hh>

namespace vtkimage {

class CVtk3DImageIOPlugin : public mia::C3DImageIOPlugin {
public:
	CVtk3DImageIOPlugin();
private:

        virtual PData do_load(const std::string&  filename) const;
	virtual bool do_save(const std::string& fname, const Data& data) const;
	virtual const std::string do_get_descr() const;
};


class CVtkXML3DImageIOPlugin : public mia::C3DImageIOPlugin {
public:
	CVtkXML3DImageIOPlugin();
private:

        virtual PData do_load(const std::string&  filename) const;
	virtual bool do_save(const std::string& fname, const Data& data) const;
	virtual const std::string do_get_descr() const;
};

}

#endif 
