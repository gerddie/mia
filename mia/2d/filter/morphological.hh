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

#include <mia/2d/filter.hh>
#include <mia/2d/shape.hh>
#include <iomanip>
#include <limits>

NS_BEGIN(morphological_2dimage_filter)

class C2DDilate: public mia::C2DFilter {
public:
	C2DDilate(mia::P2DShape shape, bool hint);

	template <typename T>
	typename mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& result)const;

private:

	virtual mia::P2DImage do_filter(const mia::C2DImage& src) const;

	mia::P2DShape m_shape;
	bool m_more_dark;
};


class C2DErode: public mia::C2DFilter {
public:
	C2DErode(mia::P2DShape shape, bool hint);

	template <typename T>
	typename mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& result)const;

private:

	virtual mia::P2DImage do_filter(const mia::C2DImage& src) const;

	mia::P2DShape m_shape;
	bool m_more_dark;
};

class C2DOpenClose: public mia::C2DFilter {
public:
	C2DOpenClose(mia::P2DShape shape, bool hint, bool open);
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& src) const;

	C2DErode m_erode;
	C2DDilate m_dilate;
	bool m_open;
};

class C2DMorphFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DMorphFilterFactory(const char *name);

private:
	virtual mia::C2DFilter *do_create()const;
	virtual mia::C2DFilter *dodo_create(mia::P2DShape shape, bool hint) const = 0;

	mia::P2DShape m_shape;
	std::string m_hint;
};

class C2DDilateFilterFactory: public C2DMorphFilterFactory {
public:
	C2DDilateFilterFactory();
private:
	virtual mia::C2DFilter *dodo_create(mia::P2DShape shape, bool hint) const;
	virtual const std::string do_get_descr()const;
};


class C2DErodeFilterFactory: public C2DMorphFilterFactory {
public:
	C2DErodeFilterFactory();
private:
	virtual mia::C2DFilter *dodo_create(mia::P2DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;

};


class C2DOpenFilterFactory: public C2DMorphFilterFactory {
public:
	C2DOpenFilterFactory();
private:
	virtual mia::C2DFilter *dodo_create(mia::P2DShape shape, bool hint) const;
	virtual const std::string do_get_descr()const;
};

class C2DCloseFilterFactory: public C2DMorphFilterFactory {
public:
	C2DCloseFilterFactory();
private:
	virtual mia::C2DFilter *dodo_create(mia::P2DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;
};

NS_END
