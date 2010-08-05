/*
** Copyrigh (C) 2007 Gert Wollny <gert at die.upm.es>
**   E.S.T.I. Telecomunication, Universidad Politecnica, Madrid
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/


#include <mia/2d/2dfilter.hh>
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

	mia::P2DShape _M_shape;
	bool _M_more_dark;
};


class C2DErode: public mia::C2DFilter {
public:
	C2DErode(mia::P2DShape shape, bool hint);

	template <typename T>
	typename mia::C2DFilter::result_type operator () (const mia::T2DImage<T>& result)const;

private:

	virtual mia::P2DImage do_filter(const mia::C2DImage& src) const;

	mia::P2DShape _M_shape;
	bool _M_more_dark;
};

class C2DOpenClose: public mia::C2DFilter {
public:
	C2DOpenClose(mia::P2DShape shape, bool hint, bool open);
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& src) const;

	C2DErode _M_erode;
	C2DDilate _M_dilate;
	bool _M_open;
};

class C2DMorphFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DMorphFilterFactory(const char *name);

private:
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual mia::C2DFilterPlugin::ProductPtr dodo_create(mia::P2DShape shape, bool hint) const = 0;

	virtual void prepare_path() const;

	std::string _M_shape_descr;
	std::string _M_hint;
};

class C2DDilateFilterFactory: public C2DMorphFilterFactory {
public:
	C2DDilateFilterFactory();
private:
	virtual mia::C2DFilterPlugin::ProductPtr dodo_create(mia::P2DShape shape, bool hint) const;
	virtual const std::string do_get_descr()const;
};


class C2DErodeFilterFactory: public C2DMorphFilterFactory {
public:
	C2DErodeFilterFactory();
private:
	virtual mia::C2DFilterPlugin::ProductPtr dodo_create(mia::P2DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;

};


class C2DOpenFilterFactory: public C2DMorphFilterFactory {
public:
	C2DOpenFilterFactory();
private:
	virtual mia::C2DFilterPlugin::ProductPtr dodo_create(mia::P2DShape shape, bool hint) const;
	virtual const std::string do_get_descr()const;
};

class C2DCloseFilterFactory: public C2DMorphFilterFactory {
public:
	C2DCloseFilterFactory();
private:
	virtual mia::C2DFilterPlugin::ProductPtr dodo_create(mia::P2DShape shape, bool hint) const;

	virtual const std::string do_get_descr()const;
};

NS_END
