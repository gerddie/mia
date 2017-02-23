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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <limits>
#include <cmath>
#include <mia/2d/creator.hh>

NS_BEGIN(creator_circle_2d);
using namespace mia;
using namespace std;

class C2DCircleCreator	: public C2DImageCreator {
public:
	C2DCircleCreator(float f, float p);
	virtual P2DImage operator () (const C2DBounds& size, EPixelType type) const;
private:
	template <typename T>
 	P2DImage do_create(const C2DBounds& size) const;
	float m_f;
	double m_p;
};

class C2DCircleCreatorPlugin : public  C2DImageCreatorPlugin {
public:
	C2DCircleCreatorPlugin();
private:
	virtual C2DImageCreator *do_create()const;
	virtual const string do_get_descr()const;
	float m_f;
	float m_p;
};

NS_END
