/* -*- mia-c++  -*-
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

#include <mia/3d/cost/ngf.hh>
#include <mia/3d/nfg.hh>


NS_BEGIN(ngf_3dimage_cost)

using namespace std;
using namespace boost;
using namespace mia;


double FScalar::cost(const C3DFVector& src, const C3DFVector& ref) const
{
	double d = dot(src, ref);
	return - d * d * 0.5;
}
C3DFVector  FScalar::grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
			   const C3DFVector& ref, double& cost) const	
{
	double d = dot(*isrc,ref);
	cost -= d * d * 0.5;
	C3DFVector result ( dot(isrc[1] - isrc[-1], ref),
			    dot(isrc[nx] - isrc[-nx], ref),
			    dot(isrc[nxy] - isrc[-nxy], ref));
	return - d * result;
}

double FCross::cost(const C3DFVector& src, const C3DFVector& ref) const
{
	C3DFVector d = cross(src, ref);
	return d.norm2(); 
}

C3DFVector  FCross::grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
	      const C3DFVector& ref, double& cost) const 
{
	C3DFVector d = cross(*isrc, ref);
	cost += d.norm2();
	
	return C3DFVector  ( dot(d, cross(isrc[1] - isrc[-1], ref)),
			     dot(d, cross(isrc[nx] - isrc[-nx], ref)),
			     dot(d, cross(isrc[nxy] - isrc[-nxy], ref)));
}

double FDeltaScalar::cost (const C3DFVector& src, const C3DFVector& ref) const
{
	double d = get_dot(src, ref);
	return d * d;
}

C3DFVector FDeltaScalar::grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
			     const C3DFVector& ref, double& cost) const
{
	double d = get_dot(*isrc,ref);
	cost += d * d;
	C3DFVector result ( dot(isrc[1] - isrc[-1], ref),
			    dot(isrc[nx] - isrc[-nx], ref),
			    dot(isrc[nxy] - isrc[-nxy], ref));
	return d * result;
}

double FDeltaScalar::get_dot(const C3DFVector& src, const C3DFVector& ref)const 
{
	return dot (src, ref) > 0 ? dot(src - ref, ref): dot(src + ref, ref);
}


NS_END
