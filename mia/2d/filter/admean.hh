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

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter.hh>

NS_BEGIN ( admean_2dimage_filter )

class C2DAdmean : public mia::C2DFilter
{
	int m_width;
public:
	C2DAdmean ( int hwidth ) :
		m_width ( hwidth )
	{
	}

	template <class Data2D>
	typename C2DAdmean::result_type operator () ( const Data2D& data ) const ;

private:
	virtual mia::P2DImage do_filter ( const mia::C2DImage& image ) const;
};

class CAdmean2DImageFilterFactory: public mia::C2DFilterPlugin
{
public:
	CAdmean2DImageFilterFactory();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr() const;
private:
	int m_hwidth;
};

NS_END
