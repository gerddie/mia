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

#include <mia/3d/filter.hh>

NS_BEGIN( transform_3dimage_filter)


class C3DTransform : public mia::C3DFilter {
public:
	C3DTransform(const std::string& name, const std::string& kernel, const std::string& bc);

private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	std::string m_name; 
	std::string m_kernel;
	std::string m_bc; 

};

class C3DTransformFilterPluginFactory: public mia::C3DFilterPlugin {
public:
	C3DTransformFilterPluginFactory();
private:
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
	std::string m_filename; 
	std::string m_interpolator_kernel;
	std::string m_interpolator_bc; 
};

NS_END


