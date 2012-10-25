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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef __GCC__
#define PRIVATE
#else
#define PRIVATE __attribute__((visibility("hidden")))
#endif

#include <cassert>
#include <iomanip>
#include <limits>
#include <mia/3d/shape.hh>
#include <mia/3d/2dimagefifofilter.hh>

NS_BEGIN(morphological_fifof)

template <typename T, bool is_float>
struct DilateCompare {

	static bool apply (T a, T b) {
		return a < b;
	}

	static bool apply (bool v) {
		return v;
	}

	static T start_value() {
		return std::numeric_limits<T>::min();
	}
};

template <typename T>
struct DilateCompare<T,true> {

	static bool apply (T a, T b) {
		return a < b;
	}
	static bool apply (bool v) {
		return v;
	}

	static T start_value() {
		return 0.0;
	}
};


template <typename T, bool is_float>
struct ErodeCompare {
	static bool apply (T a, T b) {
		return a > b;
	}
	static bool apply (bool v) {
		return !v;
	}
	static T start_value() {
		return std::numeric_limits<T>::max();
	}
};



template <template <typename, bool> class Compare>
class PRIVATE C2DMorphFifoFilter : public mia::C2DImageFifoFilter {
 public:
	C2DMorphFifoFilter(mia::P3DShape shape);
	typedef mia::C2DImage *result_type;

	template <typename T>
		mia::C2DImage *operator()(const mia::T2DImage<T>& input);

	template <typename T>
		mia::C2DImage *operator()(const mia::T3DImage<T>& input) const;
 private:

	void do_push(::boost::call_traits<mia::P2DImage>::param_type x);
	void do_initialize(::boost::call_traits<mia::P2DImage>::param_type x);
	mia::P2DImage do_filter();
	void shift_buffer();

	mia::P3DShape m_shape;
	std::auto_ptr<mia::C3DImage> m_buffer;

	mia::C2DBounds m_slice_size;
	CShiftSlices m_shifter;
};

class PRIVATE C2DMorphFifoFilterPluginBase : public mia::C2DFifoFilterPlugin {
 protected:
	C2DMorphFifoFilterPluginBase(const char *name);
	virtual const std::string do_get_descr() const;
	mia::P3DShape get_shape() const;
 private:
	mia::P3DShape m_shape;
};

template <template <typename, bool> class Compare>
class PRIVATE C2DMorphFifoFilterPlugin : public C2DMorphFifoFilterPluginBase {
 protected:
	C2DMorphFifoFilterPlugin(const char *name);
	virtual mia::C2DImageFifoFilter *do_create()const;
 private:

};

class PRIVATE C2DDilateFifoFilterPlugin : public C2DMorphFifoFilterPlugin<DilateCompare> {
public:
	C2DDilateFifoFilterPlugin();
};

class PRIVATE C2DErodeFifoFilterPlugin : public C2DMorphFifoFilterPlugin<ErodeCompare> {
public:
	C2DErodeFifoFilterPlugin();

};

class PRIVATE C2DOpenFifoFilterPlugin : public C2DMorphFifoFilterPluginBase {
public:
	C2DOpenFifoFilterPlugin();
private:
	virtual mia::C2DImageFifoFilter *do_create()const;
};

class PRIVATE C2DCloseFifoFilterPlugin : public C2DMorphFifoFilterPluginBase {
public:
	C2DCloseFifoFilterPlugin();
private:
	virtual mia::C2DImageFifoFilter *do_create()const;
};

NS_END

