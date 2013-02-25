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

#include <mia/3d/creator.hh>

NS_BEGIN(creator_sphere_3d);

class C3DSphereCreator	: public mia::C3DImageCreator {
public:
	C3DSphereCreator(float f, float p);
	virtual mia::P3DImage operator () (const mia::C3DBounds& size, mia::EPixelType type) const;
private:
	template <typename T>
 	mia::P3DImage do_create(const mia::C3DBounds& size) const;
	float m_f;
	double m_p;
};


class C3DSphereCreatorPlugin : public  mia::C3DImageCreatorPlugin {
public:
	C3DSphereCreatorPlugin();
private:
	virtual mia::C3DImageCreator *do_create()const;
	virtual const std::string do_get_descr()const;
	float m_f;
	float m_p;
};

NS_END
