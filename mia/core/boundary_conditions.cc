/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2011 Gert Wollny 
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

#include <cassert>
#include <mia/core/msgstream.hh>
#include <mia/core/boundary_conditions.hh>

NS_MIA_BEGIN
using std::vector; 

CBoundaryCondition::CBoundaryCondition():
	m_width(0)
{
	
}

CBoundaryCondition::CBoundaryCondition(int width):
	m_width(width)
{
}

void CBoundaryCondition::set_width(int width)
{
	m_width = width; 
	do_set_width(width); 
}

void CBoundaryCondition::do_set_width(int width)
{
}

bool CBoundaryCondition::apply(std::vector<int>& index, std::vector<double>& weights) const
{
	assert(m_width > 0); 
	if ( (index[0] >= 0) && index[index.size()-1] < m_width) 
		return true; 
	do_apply(index, weights); 
	return false; 
}

CMirrorOnBoundary::CMirrorOnBoundary():
	m_width2(0)
{
}

CMirrorOnBoundary::CMirrorOnBoundary(int width):
	CBoundaryCondition(width), 
	m_width2(2*width - 2)
{
}

int CMirrorOnBoundary::get_index(int idx) const
{
	return idx; 
}

void CMirrorOnBoundary::do_set_width(int width)
{
	m_width2 = 2*width - 2; 
}

void CMirrorOnBoundary::do_apply(std::vector<int>& index, std::vector<double>& weights) const
{
	for (size_t k = 0; k < index.size(); k++) {
		int idx = (index[k] < 0) ? -index[k] : index[k]; 
		
		idx = (get_width() == 1) ? (0) : ((idx < m_width2) ? idx : idx % m_width2);
		if (get_width() <= idx) {
			cvdebug() << "yes:" << idx << "\n"; 
			idx = m_width2 - idx;
		}
		index[k] = idx; 
		cvdebug() << k << ": " << index[k] << "\n"; 
	}
}


CZeroBoundary::CZeroBoundary(int width):
	CBoundaryCondition(width)
{
}

int CZeroBoundary::get_index(int idx) const
{
	return -1; 
}

void CZeroBoundary::do_apply(std::vector<int>& index, std::vector<double>& weights) const
{
	for (size_t k = 0; k < index.size(); k++) {
		if (index[k] < 0 || index[k] >= get_width()) {
			index[k] = 0; 
			weights[k] = 0; 
		}	
	}
}

CRepeatBoundary::CRepeatBoundary():
	m_widthm1(0)
{
}

CRepeatBoundary::CRepeatBoundary(int width):
	CBoundaryCondition(width), 
	m_widthm1(width-1)
{
}

void CRepeatBoundary::do_set_width(int width)
{
	m_widthm1 = width-1; 
}

int CRepeatBoundary::get_index(int idx) const
{
	return 0; 
}

void CRepeatBoundary::do_apply(std::vector<int>& index, std::vector<double>& weights) const
{
	for (size_t k = 0; k < index.size(); k++) {
		if (index[k] < 0) 
			index[k] = 0; 
		else if (index[k] > m_widthm1) {
			index[k] = m_widthm1; 
		}	
	}
}

NS_MIA_END
