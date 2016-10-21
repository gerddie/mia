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

#ifndef addons_vistaio_3dtrans_hh
#define addons_vistaio_3dtrans_hh

#include <mia/3d/transformio.hh>

NS_BEGIN(vista_3dtrans_io)

class C3DVistaTransformationIO: public mia::C3DTransformationIO {
public: 	
	C3DVistaTransformationIO(); 
private: 
	virtual PData do_load(const std::string& fname) const;
	virtual bool do_save(const std::string& fname, const mia::C3DTransformation& data) const;
	const std::string do_get_descr() const;
	const std::string do_get_preferred_suffix() const; 
}; 


NS_END

#endif 
