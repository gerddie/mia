/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


NS_BEGIN(mia_meshfilter_scale) 

using mia::C3DFVector; 
using mia::PTriangleMesh; 
using mia::CTriangleMesh; 
using std::pair; 

enum EMeshScaleStrategy {mss_iso_linear, mss_aniso_linear, mss_iso_fitbox, mss_stretch_fitbox, mss_undefined}; 


enum EMeshScaleStrategyInternal {mss_direct, mss_fix_iso, mss_fix_aniso}; 

class CScaleMeshFilter: public mia::CMeshFilter {
public: 
	CScaleMeshFilter(EMeshScaleStrategyInternal strategy, const C3DFVector& param1, const C3DFVector& param2); 
private: 
	PTriangleMesh do_filter(const CTriangleMesh& image) const; 

	PTriangleMesh scale(const CTriangleMesh& mesh, const C3DFVector& scale, const C3DFVector& shift) const; 
	pair<C3DFVector,  C3DFVector> get_aniso_scale_and_shift(const CTriangleMesh& mesh, const C3DFVector& bstart, const mia::C3DFVector& bend)const; 
	pair<C3DFVector,  C3DFVector> get_iso_scale_and_shift(const CTriangleMesh& mesh, const C3DFVector& bstart, const mia::C3DFVector& bend)const; 
	
	EMeshScaleStrategyInternal m_strategy; 
	C3DFVector m_param1; 
	C3DFVector m_param2; 
}; 


class CScaleMeshFilterPlugin: public mia::CMeshFilterPlugin {
public: 
	CScaleMeshFilterPlugin(); 

	virtual mia::CMeshFilter *do_create()const;
	virtual const std::string do_get_descr()const;

private: 
	EMeshScaleStrategy m_scale_strategy;
	float m_scale; 
	C3DFVector m_scale_aniso; 
	C3DFVector m_shift; 
	C3DFVector m_box_start; 
	C3DFVector m_box_end; 
}; 

NS_END
