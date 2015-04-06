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

#include <mia/mesh/filter/scale.hh>

NS_BEGIN(mia_meshfilter_scale) 
using namespace mia; 
using namespace std; 


static const TDictMap<EMeshScaleStrategy>::Table table[] = {
	{ "iso-linear", mss_iso_linear, "Scale and shirt the mesh by scaling it "
	  "isotropically as given by parameter 'isoscale' and "
	  "apply a shift as given by parameter 'shift'"},
	{ "aniso-linear", mss_aniso_linear, "Scale and shirt the mesh by scaling "
	  "it anisotropically as given by parameter 'anisoscale' and "
	  "apply a shift as given by parameter 'shift'"},
	{ "iso-fitbox", mss_iso_fitbox, "Fit the mesh into the given box by shifting"
	  " it and apply isotropic scaling. "
	  "The box is given by the parameters 'box-start' for upper, left, frontal "
	  "corner, and 'box-end' for the lower, right back corner"}, 
	{ "aniso-fitbox", mss_stretch_fitbox, "Fit the mesh into the given box by "
	  "shifting it and apply anisotropic scaling. "
	  "The box is given by the parameters 'box-start' for upper, left, frontal "
	  "corner, and 'box-end' for the lower, right back corner"},
	{NULL, mss_undefined, NULL}
}; 

static const TDictMap<EMeshScaleStrategy> s_meshscale_dict(table); 


CScaleMeshFilter::CScaleMeshFilter(EMeshScaleStrategyInternal strategy, const C3DFVector& param1, const C3DFVector& param2):
	m_strategy(strategy), m_param1(param1), m_param2(param2) 
{
}

PTriangleMesh CScaleMeshFilter::scale(const CTriangleMesh& mesh, const C3DFVector& scale, const C3DFVector& shift) const
{
	cvdebug() << "scale by " << scale << " and shift by " << shift << "\n"; 
	PTriangleMesh result(mesh.clone()); 
	for_each(result->vertices_begin(), result->vertices_end(), [scale, shift](C3DFVector& v)->void {
			v *= scale; 
			v += shift; 
		}); 
	return result; 
}

pair<C3DFVector,  C3DFVector> CScaleMeshFilter::get_iso_scale_and_shift(const CTriangleMesh& mesh, const C3DFVector& bstart, const C3DFVector& bend)const
{
	C3DFVector bsize = bend - bstart; 
	auto iv = mesh.vertices_begin(); 

	C3DFVector max_v = *iv; 
	C3DFVector min_v = *iv; 

	for_each(iv + 1, mesh.vertices_end(), [&min_v, &max_v](const C3DFVector& v)->void {
			if (min_v.x > v.x) min_v.x = v.x; 
			if (min_v.y > v.y) min_v.y = v.y; 
			if (min_v.z > v.z) min_v.z = v.z; 

			if (max_v.x < v.x) max_v.x = v.x; 
			if (max_v.y < v.y) max_v.y = v.y; 
			if (max_v.z < v.z) max_v.z = v.z; 
		}); 

	pair<C3DFVector,  C3DFVector> result; 

	result.first = bsize/(max_v - min_v); 

	if (result.first.x < result.first.y) 
		result.first.y = result.first.x; 
	else 
		result.first.x = result.first.y;
	if (result.first.x < result.first.z) 
		result.first.z = result.first.x; 
	else {
		result.first.x = result.first.z; 
		result.first.y = result.first.z; 
	}
	
	result.second = (bsize - (max_v - min_v)* result.first) / 2.0;

	cvdebug() << __func__ << " scale=" << result.first << ", shift=" << result.second << "\n"; 
	return result; 
}

pair<C3DFVector,  C3DFVector> CScaleMeshFilter::get_aniso_scale_and_shift(const CTriangleMesh& mesh, const C3DFVector& bstart, const C3DFVector& bend) const
{
	C3DFVector bsize = bend - bstart; 
	auto iv = mesh.vertices_begin(); 

	C3DFVector max_v = *iv; 
	C3DFVector min_v = *iv; 

	for_each(iv + 1, mesh.vertices_end(), [&min_v, &max_v](const C3DFVector& v)->void {
			if (min_v.x > v.x) min_v.x = v.x; 
			if (min_v.y > v.y) min_v.y = v.y; 
			if (min_v.z > v.z) min_v.z = v.z; 

			if (max_v.x < v.x) max_v.x = v.x; 
			if (max_v.y < v.y) max_v.y = v.y; 
			if (max_v.z < v.z) max_v.z = v.z; 
		}); 

	pair<C3DFVector,  C3DFVector> result; 

	result.first = bsize/(max_v - min_v); 
	result.second = (bsize - (max_v - min_v)* result.first) / 2.0;
	return result; 
}


PTriangleMesh CScaleMeshFilter::do_filter(const CTriangleMesh& mesh) const
{
	if (mesh.vertices_size() == 0) 
		return PTriangleMesh(mesh.clone()); 
	
	switch (m_strategy)  {
	case mss_direct: return scale(mesh, m_param1, m_param2); 
	case mss_fix_iso: {
		auto help = get_iso_scale_and_shift(mesh, m_param1, m_param2); 
		return scale(mesh, help.first, help.second);
	}
	case mss_fix_aniso: {
			auto help = get_aniso_scale_and_shift(mesh, m_param1, m_param2); 
			return scale(mesh, help.first, help.second);
			
		}
	default: 
		assert(0 && "Internal programming error"); 
	}
}

CScaleMeshFilterPlugin::CScaleMeshFilterPlugin():
	CMeshFilterPlugin("scale"), 
	m_scale_strategy(mss_iso_linear), 
	m_scale(1.0f), 
	m_scale_aniso(1.0, 1.0, 1.0), 
	m_shift(0,0,0), 
	m_box_start(0,0,0), 
	m_box_end(256, 256, 256)
{
	typedef CDictParameter<EMeshScaleStrategy> CStrategyParameter; 
	add_parameter("strategy", new CStrategyParameter(m_scale_strategy, s_meshscale_dict, "Isotropic scaling parameter.", false));
	add_parameter("isoscale", make_param(m_scale, false, "Isotropic scaling parameter."));
	add_parameter("anisoscale", make_param(m_scale_aniso, false, "Ansiotropic scaling parameter."));
	add_parameter("shift", make_param(m_shift, false, "Shift mesh after scaling."));
	add_parameter("box-start", make_param(m_box_start, false, "Box start for fitbox scaling."));
	add_parameter("box-end", make_param(m_box_end, false, "Box end for fitbox scaling."));
}

					 

CMeshFilter *CScaleMeshFilterPlugin::do_create()const
{
	switch (m_scale_strategy) {
	case mss_iso_linear:
		return new CScaleMeshFilter(mss_direct, C3DFVector(m_scale, m_scale, m_scale), m_shift); 
	case mss_aniso_linear:
		return new CScaleMeshFilter(mss_direct, m_scale_aniso, m_shift); 
	case mss_iso_fitbox:
		return new CScaleMeshFilter(mss_fix_iso, m_box_start, m_box_end); 
	case mss_stretch_fitbox: 
		return new CScaleMeshFilter(mss_fix_aniso, m_box_start, m_box_end); 
	default:
		throw invalid_argument("CScaleMeshFilterPlugin:Unknown scaling strategy requested"); 
	}
}

const std::string CScaleMeshFilterPlugin::do_get_descr()const
{
	return "This plug-in provides a filter to scale triangulat meshes by a given strategy"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CScaleMeshFilterPlugin();
}


NS_END

