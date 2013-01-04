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

#include <mia/2d/filter.hh>
#include <mia/2d/imageio.hh>

NS_BEGIN(mask_2dimage_filter)


class C2DMask: public mia::C2DFilter {
public:
	enum EFill {f_unknown, f_min, f_zero, f_max}; 

	C2DMask(const mia::C2DImageDataKey& mask_image, EFill fill, bool inverse);

	template <typename T>
	C2DMask::result_type operator () (const mia::T2DImage<T>& data) const;
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

	mia::C2DImageDataKey m_image_key;
	EFill m_fill; 
	bool m_inverse; 
};


class C2DMaskImageFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DMaskImageFilterFactory();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	std::string m_mask_filename;
	C2DMask::EFill  m_fill; 
	bool m_inverse; 
};

NS_END
