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

#include <mia/mesh/filter.hh>


NS_BEGIN(mia_meshfilter_deltribynormal)

using mia::C3DFVector;
using mia::PTriangleMesh;
using mia::CTriangleMesh;


class CDeleteTriangleByNormalMeshFilter: public mia::CMeshFilter
{
public:
       CDeleteTriangleByNormalMeshFilter(const C3DFVector& point_direction, float angle);
private:
       PTriangleMesh do_filter(const CTriangleMesh& image) const;

       C3DFVector m_point_direction;
       float m_test_angle_cos;
};


class CDeleteTriangleByNormalMeshFilterPlugin: public mia::CMeshFilterPlugin
{
public:
       CDeleteTriangleByNormalMeshFilterPlugin();

       virtual mia::CMeshFilter *do_create()const;
       virtual const std::string do_get_descr()const;

private:
       C3DFVector m_point_direction;
       float m_tolerance_angle;
};

NS_END
