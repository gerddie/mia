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



#include <mia/core/distance.hh>
#include <mia/core/msgstream.hh>

#include <limits>

NS_MIA_BEGIN
using  std::vector; 
using  std::numeric_limits; 

inline float d(float fp, float p, float fq, float q)
{
	return  ( fp  - fq + p * p - q * q) / (p - q) * 0.5; 
}

void EXPORT_CORE distance_transform_inplace(vector<float>& r)
{
	TRACE_FUNCTION; 
	vector<float> f(r); 
	vector<int> v(f.size()); 
	vector<float> z(f.size() + 1); 

	int k = 0;
	
	v[0] = 0;
	z[0] = -numeric_limits<float>::max();
	z[1] = +numeric_limits<float>::max();
	
	for (size_t q = 1; q < f.size(); q++) {
		float s  = d(f[q], q, f[v[k]], v[k]);
		while (s <= z[k]) {
			--k;
			s  = d(f[q], q, f[v[k]], v[k]);
		}
		++k;
		v[k] = q;
		z[k] = s;
		z[k+1] = numeric_limits<float>::max();
	}

	k = 0;
	for (size_t q = 0; q < f.size(); ++q) {
		while (z[k+1] < q) {
			cvdebug() << "k=" << k << ", q="<< q << ", z[k+1]=" << z[k+1] << "\n"; 
			++k;
		}
		float delta = float(q) - v[k]; 
		r[q] = delta * delta +  f[v[k]];
	}
}


NS_MIA_END
