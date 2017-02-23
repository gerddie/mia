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

#include <mia/3d/filter.hh>

NS_BEGIN( labelscale_3dimage_filter)

class C3DLabelscale : public mia::C3DFilter {
public:
	C3DLabelscale(const mia::C3DBounds& out_size);

	template <class T>
	typename C3DLabelscale::result_type operator () (const mia::T3DImage<T>& data) const ;
private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;

	mia::C3DBounds m_out_size;

};

class C3DLabelscaleFilterPluginFactory: public mia::C3DFilterPlugin {
public:
	C3DLabelscaleFilterPluginFactory();
private:
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
        
	mia::C3DBounds m_out_size;
};



NS_END
