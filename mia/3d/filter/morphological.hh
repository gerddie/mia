/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#ifndef mia_3d_filter_morphological_hh
#define mia_3d_filter_morphological_hh


#include <mia/3d/3dfilter.hh>
#include <mia/3d/shape.hh>

NS_BEGIN(morph_3dimage_filter)

class C3DDilate: public mia::C3DFilter {
public:
	C3DDilate(mia::P3DShape shape, bool hint);

	template <typename T>
	typename mia::C3DFilter::result_type operator () (const mia::T3DImage<T>& result)const;

private:

	virtual mia::P3DImage do_filter(const mia::C3DImage& src) const;

	mia::P3DShape m_shape;
	bool m_more_dark;
};


class C3DErode: public mia::C3DFilter {
public:
	C3DErode(mia::P3DShape shape, bool hint);

	template <typename T>
	typename mia::C3DFilter::result_type operator () (const mia::T3DImage<T>& result)const;

private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& src) const;

	mia::P3DShape m_shape;
	bool m_more_dark;
};

class C3DOpenClose: public mia::C3DFilter {
public:
	C3DOpenClose(mia::P3DShape shape, bool hint, bool open);
private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& src) const;

	C3DErode m_erode;
	C3DDilate m_dilate;
	bool m_open;
};

class C3DMorphFilterFactory: public mia::C3DFilterPlugin {
public:
	C3DMorphFilterFactory(const char *name);

private:
	virtual mia::C3DFilter *do_create()const;
	virtual mia::C3DFilter *dodo_create(mia::P3DShape shape, bool hint) const = 0;

	mia::P3DShape m_shape;
	std::string m_hint;
};

class C3DDilateFilterFactory: public C3DMorphFilterFactory {
public:
	C3DDilateFilterFactory();
private:
	virtual mia::C3DFilter *dodo_create(mia::P3DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;
};


class C3DErodeFilterFactory: public C3DMorphFilterFactory {
public:
	C3DErodeFilterFactory();
private:
	virtual mia::C3DFilter *dodo_create(mia::P3DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;
};


class C3DOpenFilterFactory: public C3DMorphFilterFactory {
public:
	C3DOpenFilterFactory();
private:
	virtual mia::C3DFilter *dodo_create(mia::P3DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;
};

class C3DCloseFilterFactory: public C3DMorphFilterFactory {
public:
	C3DCloseFilterFactory();
private:
	virtual mia::C3DFilter *dodo_create(mia::P3DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;
};




NS_END

#endif
