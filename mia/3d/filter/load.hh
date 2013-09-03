/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

NS_BEGIN( load_3dimage_filter)


class C3DLoad : public mia::C3DFilter {
public:
	C3DLoad(const std::string& name);

private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	std::string m_name; 
};

class C3DLoadFilterPluginFactory: public mia::C3DFilterPlugin {
public:
	C3DLoadFilterPluginFactory();
private:
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
	std::string m_filename; 
};

NS_END
