/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * the Point in Polygon code was converted from
 * http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

   Copyright (c) Leipzig, Madrid 2004-2010

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimers.
   2. Redistributions in binary form must reproduce the above copyright notice
      in the documentation and/or other materials provided with the distribution.
   3. The name of W. Randolph Franklin may not be used to endorse or promote
      products derived from this Software without specific prior written permission.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
   PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <vector>
#include <mia/core/shared_ptr.hh>
#include <mia/2d/polygon.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/distances.hh>

NS_MIA_BEGIN
using namespace std;

bool C2DPolygon::is_inside_closed_set(const C2DFVector& point) const
{
	if (_M_points.empty())
		return false;

	const size_t nvert = _M_points.size();

	if (nvert == 1)
		return point == _M_points[0];

	if (nvert == 2)
		return is_on_line(point, _M_points[0], _M_points[1]);

	// code

	bool c = false;
	for (size_t i = 0, j = nvert-1; i < nvert; j = i++) {
		if (is_on_line(point, _M_points[i], _M_points[j]))
		    return true;

		if ( ((_M_points[i].y > point.y) != ( _M_points[j].y > point .y) ) &&
		     (point.x < (_M_points[j].x - _M_points[i].x) *
		      (point.y - _M_points[i].y) / (_M_points[j].y-_M_points[i].y) + _M_points[i].x) )
			c = !c;
	}
	return c;
}

bool C2DPolygon::is_inside_open_set(const C2DFVector& point) const
{
	const size_t nvert = _M_points.size();
	if (nvert < 3)
		return false;

	bool c = false;
	for (size_t i = 0, j = nvert-1; i < nvert; j = i++) {

		if (is_on_line(point, _M_points[i], _M_points[j]))
			return false;

		if ( ((_M_points[i].y > point.y) != ( _M_points[j].y > point .y) ) &&
		     (point.x < (_M_points[j].x - _M_points[i].x) *
		      (point.y - _M_points[i].y) / (_M_points[j].y-_M_points[i].y) + _M_points[i].x) )
			c = !c;
	}
	return c;
}

bool C2DPolygon::is_on_line(const C2DFVector& p, const C2DFVector& a, const C2DFVector& b) const
{
	const C2DFVector lp = p -a;
	const C2DFVector lt = b -a;
	const float dotlplt = dot(lt, lp);
	const float nlp = lp.norm2();
	const float nlt = lt.norm2();
	return dotlplt >= 0 && fabs(dotlplt * dotlplt - nlp * nlt) < 1e-6  && nlp < nlt;
}


void C2DPolygon::append(const C2DFVector& point)
{
	_M_points.push_back(point);
}

size_t C2DPolygon::size() const
{
	return _M_points.size();
}

float C2DPolygon::get_mimimum_distance(const C2DFVector& point)const
{
	assert(!_M_points.empty());
	const size_t nvert = _M_points.size();
	if (nvert < 2)
		return (point - _M_points[0]).norm();

	float min_distance = distance_point_line(point, _M_points[0], _M_points[nvert-1]);

	vector<C2DFVector>::const_iterator p1 = _M_points.begin();
	vector<C2DFVector>::const_iterator p2 = _M_points.begin() + 1;

	while (p2 != _M_points.end()) {
		float distance = distance_point_line(point, *p1++, *p2++);
		if (min_distance > distance) {
			min_distance = distance;
		}
	}
	return min_distance;
}

float C2DPolygon::get_hausdorff_distance(const C2DPolygon& other)const
{
	float max_distance = 0.0;
	vector<C2DFVector>::const_iterator p1 = _M_points.begin();
	vector<C2DFVector>::const_iterator pe = _M_points.end();

	while (p1 != pe) {
		float distance = other.get_mimimum_distance(*p1++);
		if (max_distance < distance)
			max_distance = distance;
	}


	p1 = other._M_points.begin();
	pe = other._M_points.end();

	while (p1 != pe) {
		float distance = get_mimimum_distance(*p1++);
		if (max_distance < distance)
			max_distance = distance;
	}
	return max_distance;
}

NS_MIA_END
