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

#ifndef __mia_3d_filter_kmeans_hh
#define __mia_3d_filter_kmeans_hh

#include <limits>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/3dfilter.hh>

NS_BEGIN( kmeans_3dimage_filter)

class C3DKMeans : public mia::C3DFilter {
public:
	C3DKMeans(size_t classes);

	template <class T>
	typename C3DKMeans::result_type operator () (const mia::T3DImage<T>& data) const ;
private:
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;

	size_t m_classes;
};

class C3DKMeansFilterPluginFactory: public mia::C3DFilterPlugin {
public:
	C3DKMeansFilterPluginFactory();
private:
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
	int m_classes;
};

NS_END


#endif
