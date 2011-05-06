/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef __mia_2d_filter_kmeans_hh
#define __mia_2d_filter_kmeans_hh

#include <limits>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dfilter.hh>

NS_BEGIN( kmeans_2dimage_filter)

class C2DKMeans : public mia::C2DFilter {
public:
	C2DKMeans(size_t classes);

	template <class T>
	typename C2DKMeans::result_type operator () (const mia::T2DImage<T>& data) const ;
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

	size_t m_classes;
};

class C2DKMeansFilterPluginFactory: public mia::C2DFilterPlugin {
public:
	C2DKMeansFilterPluginFactory();
private:
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
	int m_classes;
};

NS_END


#endif
