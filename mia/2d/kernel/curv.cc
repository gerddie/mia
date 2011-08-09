/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <mia/2d/cstkernel.hh>
#include <mia/core/dictmap.hh>

NS_BEGIN(cstkernel_2dvector)
NS_MIA_USE;
using namespace std; 


enum EBoundary { bc_unknown, bc_neumann, bc_dirichlet}; 
enum EPointLoc { pl_unknwon, pl_mid, pl_grid}; 

typedef TDictMap<EBoundary> CBoundaryCondMap; 
typedef TDictMap<EPoint>    CPointLocMap; 

const CBoundaryCondMap::Table[] = {
	{ "neumann", bc_neumann}, 
	{ "dirichlet", bc_dirichlet}, 
	{ 0, bc_unknown}
}; 

const CPointLocMap::Table[] ={
	{ "mid", pl_mid}, 
	{ "grid", pl_grid}, 
	{ 0, pl_unknown}
}; 

class CCST2DCurvKernel: public CCST2DImageKernel {
public: 
	CCST2DCurvKernel(ffwf_r2r_kind forward, EBoundary boundary, EPoint point);
private: 
	class PrivatePlan: public PCST2DVectorKernel::CPlan {
	public: 
		PrivatePlan(fftwf_r2r_kind forward, std::vector<int> size); 
	private: 
		virtual void do_execute(C2DFVector *buffer) const; 
		
		vector<float> m_l1; 
		vector<float> m_l2;
		float m_step_n_weight; 

	}; 
	virtual PCST2DVectorKernel::CPlan *do_prepare(fftwf_r2r_kind fw_kind, const std::vector<int>& size);

	EBoundary m_boundary; 
	EPoint m_point; 
}; 


CCST2DCurvKernel::PrivatePlan::PrivatePlan(fftwf_r2r_kind forward, std::vector<int> size):
	PCST2DVectorKernel::CPlan( forward, size),
	m_l1(size[0]), 
	m_l2(size[1]), 
{
	double i_start = (boundary == bc_dirichlet)  ? 1 : 0; 

	double n_l1 = m_l1.size(); 
	double n_l2 = m_l2.size(); 

	if (point == pl_grid) 
		n_l1 += (boundary == bc_dirichlet) ? 1.0 : -1.0; 
		
	double i = i_start; 
	for (vector<float>::iterator l = m_l1.begin(); l != m_l1.end(); ++l, ++i) 
		*l = 2.0 * cos( (M_PI * i ) / n_l1 ) - 2.0 ;

	i = i_start; 
	for (vector<float>::iterator l = m_l2.begin(); l != m_l2.end(); ++l, ++i)
		*l = 2.0 * cos( (M_PI * i ) / n_l2 ) - 2.0 ;
}

void CCST2DCurvKernel::PrivatePlan::do_execute(C2DFVector *buffer) const
{
	assert(2 == get_size().size()); 
	
	size_t sx = get_size()[0]; 
	size_t sy = get_size()[1]; 

	vector<float>::const_iterator l2 = m_l2.begin(); 
	for (size_t y = 0; y < sy; ++y, ++l2) {
		vector<float>::const_iterator l1 = m_l1.begin(); 
		for (size_t x = 0; x < sx; ++x, ++buffer, ++l1) {
			float l = 1.0 +  m_step_n_weight  * ( *l1 + *l2 ); 
			*buffer = (l != 0.0f) ? 0.0f : *buffer / l; 
		}
}


NS_END




