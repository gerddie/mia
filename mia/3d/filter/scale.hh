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

#include <memory>
#include <mia/core/filter.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/interpolator.hh>

NS_BEGIN(scale_3dimage_filter)


class C3DScale: public mia::C3DFilter {
public:
	C3DScale(const mia::C3DBounds& size, const std::string& interpolator);

	template <typename  T>
	mia::C3DFilter::result_type operator () (const mia::T3DImage<T>& data) const;

private:
	C3DScale::result_type do_filter(const mia::C3DImage& image) const;

	const mia::C3DBounds m_size;
	std::unique_ptr<mia::C3DInterpolatorFactory> m_ipf;
};

class C3DScaleFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DScaleFilterPlugin();
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::C3DBounds m_s; 
	std::string m_interp;
};


class CIsoVoxel: public mia::C3DFilter {
public:
	CIsoVoxel(float voxelsize, const std::string& interpolator);

private:
	CIsoVoxel::result_type do_filter(const mia::C3DImage& image) const;
	CIsoVoxel::result_type do_filter(mia::P3DImage image) const;

	float m_voxelsize;
	std::string m_interp;
};


class CIsoVoxelFilterPlugin: public mia::C3DFilterPlugin {
public:
	CIsoVoxelFilterPlugin();
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	float m_voxelsize;
	std::string m_interp;
};

NS_END
