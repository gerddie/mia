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


/*
  The spline kernels are based on code by
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/

 */


/*
  LatexBeginPluginSection{Spline Interpolation Kernels}
  \label{sec:splinekernels}
  
  These kernels are used for spline based interpolation. 
  
  The kernels are created by calling \emph{produce\_spline\_kernel}. 
  
  LatexEnd
*/



#include <cmath>
#include <cassert>
#include <iomanip>
#include <limits>
#include <mia/core/export_handler.hh>
#include <mia/core/splinekernel.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/simpson.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <mia/core/boundary_conditions.hh>

#include <boost/filesystem.hpp>


#if defined(__SSE2__)
#include <emmintrin.h>
#endif


#define USE_FASTFLOOR
NS_MIA_BEGIN
using namespace std;

const char *CSplineKernel::type_descr = "1d"; 
const char *CSplineKernel::data_descr = "splinekernel"; 

CSplineKernel::SCache::SCache(size_t s, const CSplineBoundaryCondition& bc, bool am):
	x(numeric_limits<double>::quiet_NaN()), 
	start_idx(-1000), 
	index_limit(bc.get_width() - s), 
	weights(s), 
	index(s), 
	boundary_condition(bc), 
	is_flat(false), 
	never_flat(am)
{
}

void CSplineKernel::SCache::reset()
{
	index_limit = boundary_condition.get_width() - weights.size(); 
}

CSplineKernel::CSplineKernel(int degree, double shift, EInterpolation type):
	m_half_degree(degree >> 1),
	m_shift(shift),
	m_support_size(degree + 1), 
	m_type(type), 
	m_indices(m_support_size)
{
	for(unsigned int i = 0; i < m_support_size; ++i) 
		m_indices[i] = i; 
}

CSplineKernel::~CSplineKernel()
{
}

EInterpolation CSplineKernel::get_type() const
{
	return m_type; 
}

void CSplineKernel::operator () (double x, VWeight& weight, VIndex& index)const
{
	assert(index.size() == m_support_size);
	int ix = get_indices(x, index);
	get_weights(x - ix, weight);
}

void CSplineKernel::operator () (double x, SCache& cache) const
{
	assert(cache.index_limit == (int)cache.boundary_condition.get_width() - (int)cache.weights.size()); 
	if (x == cache.x)
		return; 
	int start_idx  = get_start_idx_and_value_weights(x, cache.weights); 
	cache.x = x; 
	if (start_idx == cache.start_idx) 
		return; 
	cache.start_idx = start_idx; 

	if (cache.never_flat || start_idx < 0 || start_idx > cache.index_limit ) {
		cache.is_flat = false; 
		fill_index(start_idx, cache.index); 
		cache.boundary_condition.apply(cache.index, cache.weights); 
	}else {
		cache.index[0] = start_idx; 
		cache.is_flat = true; 
	}
}

void CSplineKernel::get_cached(double x, SCache& cache)const
{
	int start_idx  = get_start_idx_and_value_weights(x, cache.weights); 
	cache.x = x; 
	if (start_idx == cache.start_idx) 
		return; 
	cache.start_idx = start_idx; 
	cache.is_flat = false; 
	fill_index(start_idx, cache.index); 
	cache.boundary_condition.apply(cache.index, cache.weights); 
}

void CSplineKernel::get_uncached(double x, SCache& cache)const
{
	assert(cache.index_limit == (int)cache.boundary_condition.get_width() - (int)cache.weights.size()); 
	cache.start_idx  = get_start_idx_and_value_weights(x, cache.weights); 
	if (cache.never_flat ||cache.start_idx < 0 || cache.start_idx > cache.index_limit ) {
		cache.is_flat = false; 
		fill_index(cache.start_idx, cache.index); 
		cache.boundary_condition.apply(cache.index, cache.weights); 
	}else {
		cache.index[0] = cache.start_idx; 
		cache.is_flat = true; 
	}
}

#ifdef USE_FASTFLOOR
// code taken from http://www.stereopsis.com/FPU.html
// Michael Herf
const double _double2fixmagic = 68719476736.0*1.5;     //2^36 * 1.5,  (52-_shiftamt=36) uses limited precisicion to floor
const int    _shiftamt        = 16;                    //16.16 fixed point representation,

#if BIGENDIAN_
	#define iexp_				0
	#define iman_				1
#else
	#define iexp_				1
	#define iman_				0
#endif //BigEndian_

// ================================================================================================
// Real2Int
// ================================================================================================
inline int fastfloor(double val)
{
	union {
		double dval; 
		int ival[2]; 
	} v; 
	v.dval = val + _double2fixmagic;
	return v.ival[iman_] >> _shiftamt; 
}
#else
#define fastfloor floor
#endif

void CSplineKernel::fill_index(short i, VIndex& index) const 
{
	unsigned int k= 0; 
	while (k < m_support_size){
		index[k] = i + m_indices[k]; 	
		++k; 
	}
}

int CSplineKernel::get_indices(double x, VIndex& index) const
{
	const int ix = fastfloor(x + m_shift);
	fill_index(ix - (int)m_half_degree, index); 
	return ix;
}

double CSplineKernel::get_weight_at(double /*x*/, int degree) const
{
	THROW(invalid_argument, "B-Spline: derivative degree "
	      <<  degree << " not supported" );
}

const vector<double>& CSplineKernel::get_poles() const
{
	return m_poles;
}

void CSplineKernel::add_pole(double x)
{
	m_poles.push_back(x);
}

void CSplineKernel::derivative(double x, VWeight& weight, VIndex& index)const
{
	assert(index.size() == m_support_size);
	int ix = get_indices(x, index);
	get_derivative_weights(x - ix, weight);
}

void CSplineKernel::derivative(double x, VWeight& weight, VIndex& index, int degree)const
{
	assert(index.size() == m_support_size);
	int ix = get_indices(x, index);
	get_derivative_weights(x - ix, weight, degree);
}

double CSplineKernel::get_nonzero_radius() const
{
	return m_support_size / 2.0;
}

int CSplineKernel::get_active_halfrange() const   
{
	return (m_support_size  + 1) / 2;
}

int CSplineKernel::get_start_idx_and_value_weights(double x, VWeight& weights) const
{
	const int result = fastfloor(x + m_shift);
	get_weights(x - result, weights); 
	return result - (int)m_half_degree; 
}

int CSplineKernel::get_start_idx_and_derivative_weights(double x, VWeight& weights) const
{
	const int result = fastfloor(x + m_shift);
	get_derivative_weights(x - result, weights); 
	return result - (int)m_half_degree; 
}



struct F2DKernelIntegrator {
	F2DKernelIntegrator(const CSplineKernel& spline, double s1, double s2, int deg1, int deg2):
		m_spline(spline), m_s1(s1), m_s2(s2), m_deg1(deg1), m_deg2(deg2)
		{
		}
	double operator() (double x) const {
		return m_spline.get_weight_at(x - m_s1, m_deg1) *
			m_spline.get_weight_at(x - m_s2, m_deg2);
	}
private:
	const CSplineKernel& m_spline;
	double m_s1, m_s2, m_deg1, m_deg2;
};


double  EXPORT_CORE integrate2(const CSplineKernel& spline, double s1, double s2, int deg1, int deg2, double n, double x0, double L)
{
	double sum = 0.0;
	x0 /= n;
	L  /= n;

	// evaluate interval to integrate over
	double start_int = s1 - spline.get_nonzero_radius();
	double end_int = s1 + spline.get_nonzero_radius();
	if (start_int < s2 - spline.get_nonzero_radius())
		start_int = s2 - spline.get_nonzero_radius();
	if (start_int < x0)
		start_int = x0;
	if (end_int > s2 + spline.get_nonzero_radius())
		end_int = s2 + spline.get_nonzero_radius();
	if (end_int > L)
		end_int = L;

	// Simpson formula
	if (end_int <= start_int)
		return sum;
	const size_t intervals = size_t(8 * (end_int - start_int));

	sum = simpson( start_int, end_int, intervals, F2DKernelIntegrator(spline, s1, s2, deg1, deg2));
	return sum * n;
}

EXPLICIT_INSTANCE_HANDLER(CSplineKernel); 

using boost::filesystem::path; 
CSplineKernelTestPath::CSplineKernelTestPath()
{
	CSplineBoundaryConditionTestPath bcpath;
	list< path> sksearchpath; 
	sksearchpath.push_back( path(MIA_BUILD_ROOT"/mia/core/splinekernel"));
	CSplineKernelPluginHandler::set_search_path(sksearchpath); 
	
}

NS_MIA_END
