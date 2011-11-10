/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <mia/2d/2dfilter.hh>
#include <mia/2d/2dimageio.hh>

NS_BEGIN(mask_2dimage_filter)


class C2DMask: public mia::C2DFilter {
public:
	C2DMask(const mia::C2DImageDataKey& mask_image);

	template <typename T>
	C2DMask::result_type operator () (const mia::T2DImage<T>& data) const;
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

	mia::C2DImageDataKey m_image_key;
};


class C2DMaskImageFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DMaskImageFilterFactory();
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	std::string m_mask_filename;
	bool m_invert;
};

NS_END
