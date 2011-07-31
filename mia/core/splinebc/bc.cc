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


/* 
   LatexBeginPluginDescription{Spline Interpolation Boundary Conditions}
   
   \subsection{Mirror on Boundary}
   \label{splinebc:mirror}
   
   \begin{description}
   
   \item [Plugin:] mirror
   \item [Description:] This plug-in mirror-on-boundary spline interpolation kernels. 
       A index $i$ into a coeficient vector of length $w$ is translated according to 
       \begin{equation*}
         i^* := \left\{ 
	               \begin{array}{lll} 
		       |i| & \text{for} &   |i| mod (2w-2) < w\\
		       2 w - 2 - |i| & \multicolumn{2}{l}{otherwise}
		       \end{array}
		       \right.
       \end{equation*}

   \plugtabstart
   w & int &  length of the coefficient vector (in the according dimension) &  0 \\
   \plugtabend
   \end{description}

   LatexEnd  
*/

#include <mia/core/splinebc/bc.hh>

NS_MIA_BEGIN

CMirrorOnBoundary::CMirrorOnBoundary():
	m_width2(0)
{
}

CMirrorOnBoundary::CMirrorOnBoundary(int width):
	CSplineBoundaryCondition(width), 
	m_width2(2*width - 2)
{
}

CSplineBoundaryCondition *CMirrorOnBoundary::clone () const
{
	return new CMirrorOnBoundary(*this); 
}

void CMirrorOnBoundary::do_set_width(int width)
{
	m_width2 = 2*width - 2; 
}

void CMirrorOnBoundary::test_supported(int npoles) const
{
}

void CMirrorOnBoundary::do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const
{
	for (size_t k = 0; k < index.size(); k++) {
		int idx = (index[k] < 0) ? -index[k] : index[k]; 
		
		idx = (get_width() == 1) ? (0) : ((idx < m_width2) ? idx : idx % m_width2);
		if (get_width() <= idx) {
			idx = m_width2 - idx;
		}
		index[k] = idx; 
	}
}


double CMirrorOnBoundary::initial_coeff(const std::vector<double>& coeff, double pole)const
{ 
	double zn = pole;
	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(coeff.size() - 1));
	double sum = coeff[0] + z2n * coeff[coeff.size() - 1];
	
	z2n *= z2n * iz;
	
	for (size_t n = 1; n < coeff.size()  - 1; n++) {
		sum += (zn + z2n) * coeff[n];
		zn *= pole;
		z2n *= iz;
	}
	
	return(sum / (1.0 - zn * zn));
}

double CMirrorOnBoundary::initial_anti_coeff(const std::vector<double>& coeff, double pole)const
{
	return ((pole / (pole * pole - 1.0)) * 
		(pole * coeff[coeff.size() - 2] + coeff[coeff.size() - 1]));
	
}

/*
   LatexBeginPluginDescription{Spline Interpolation Boundary Conditions}

   \subsection{Zero boundary condition}
   \label{splinebc:zero}
   
   \begin{description}
   
   \item [Plugin:] zero
   \item [Description:] This plug-in sets those spline weights to zero that correspond to the indices that fall outside 
                        the coefficient vector index domain. The according indices are set to zero to avoid memory access errors. 
			Given spline weights $s_i$: 
       \begin{equation*}
         s_i^* := \left\{ 
	               \begin{array}{lll} 
		       0 & \text{for} &  i < 0\\
		       0 & \text{for} & i \ge w \\
		       s_i  & \multicolumn{2}{l}{otherwise}
		       \end{array}
		       \right.
       \end{equation*}

   \plugtabstart
   w & int &  length of the coefficient vector (in the according dimension) &  0 \\
   \plugtabend
   \item[Remark:] This boudary condition is not supported by spline kernels of a degree equal or larger then 4.    
   
   \end{description}


   LatexEnd  
 */


CZeroBoundary::CZeroBoundary(int width):
	CSplineBoundaryCondition(width)
{
}

CSplineBoundaryCondition *CZeroBoundary::clone ()const
{
	return new CZeroBoundary(*this); 
}

void CZeroBoundary::test_supported(int npoles) const
{
	/**
	   Unfortunately, the current pre-filtering does not work properly 
	   for splines with more then one pole. 
	 */

	if (npoles > 1) {
		THROW(invalid_argument, "Currently, zero-boundary not supported for splines with more then one pole");  
	}
}


void CZeroBoundary::do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const
{
#ifdef __SSE2__ // is this faster ? 
	const int size = index.size(); 
	if (size == 4) {
		__m128i buffer; 
		__m128i dbuf[2]; 
		memcpy(dbuf, &weights[0], size * sizeof(double)); 
		__m128i zero = _mm_setzero_si128();
		__m128i maxv = _mm_set1_epi16 (get_width() - 1);

		memcpy(&buffer, &index[0], size * sizeof(short));
		__m128i idx = _mm_load_si128(&buffer);
		__m128i old_idx = idx; 
		idx = _mm_max_epi16(idx, zero);
		idx = _mm_min_epi16(idx, maxv);
		_mm_store_si128(&buffer, idx); 
		memcpy(&index[0], &buffer, size * sizeof(short));

		// now the weights 
		maxv = _mm_set1_epi16 (get_width());
		auto lower_bound_mask =  _mm_cmpgt_epi16(zero, old_idx); 
		auto higher_bound_mask =  _mm_cmplt_epi16(old_idx, maxv);
		auto mask = _mm_andnot_si128(lower_bound_mask, higher_bound_mask); 
		// convert mask 
		auto mask_low =  _mm_unpacklo_epi16(mask, mask); 
		auto mask_high = mask_low; 
		mask_low = _mm_unpacklo_epi32(mask_low, mask_low); 
		mask_high = _mm_unpackhi_epi32(mask_high, mask_high);
		
		dbuf[0] = _mm_and_si128(mask_low, dbuf[0]); 
		dbuf[1] = _mm_and_si128(mask_high, dbuf[1]);
		
		memcpy(&weights[0], dbuf, size * sizeof(double)); 
	} else 
#endif	

	for (size_t k = 0; k < index.size(); k++) {
		if (index[k] < 0 || index[k] >= get_width()) {
			index[k] = 0; 
			weights[k] = 0; 
		}	
	}
}

double CZeroBoundary::initial_coeff(const std::vector<double>& coeff, double pole) const
{
	double zn = pole  * pole;
	double ip = 1.0/ pole; 
	double z2n = pow(pole, 2.0 * coeff.size()+2) ;

	double sum = 0.0; 
	for (size_t n = 0; n < coeff.size() - 1; n++) {
		sum -= (zn + z2n) * coeff [n]; 
		zn *= pole;
		z2n *= ip; 
	}
	
	sum -= zn * coeff [coeff.size() - 1]; 
	return coeff [0] + sum / ( 1- zn * zn); 

}

double CZeroBoundary::initial_anti_coeff(const std::vector<double>& coeff, double pole)const
{
	return - coeff[coeff.size() - 1] * pole; 
}


/*
   LatexBeginPluginDescription{Spline Interpolation Boundary Conditions}
   \subsection{Repeat boundary value}
   \label{splinebc:repeat}
   
   \begin{description}
   
   \item [Plugin:] repeat
   \item [Description:] This plug-in resets the indices outside the coefficient vector index range 
                      the the corresponding extrem. 
       \begin{equation*}
         i^* := \left\{ 
	               \begin{array}{lll} 
		       0 & \text{for} &   i < 0\\
		       w-1 & \text{for} &  i \ge w \\
		       i  & \multicolumn{2}{l}{otherwise}
		       \end{array}
		       \right.
       \end{equation*}

   \plugtabstart
   w & int &  length of the coefficient vector (in the according dimension) &  0 \\
   \plugtabend
   \item[Remark:] This boudary condition is not supported by spline kernels of a degree equal or larger then 4. 
   
   \end{description}

   LatexEnd  
*/

CRepeatBoundary::CRepeatBoundary():
	m_widthm1(0)
{
	zero = _mm_setzero_si128();
	maxv = _mm_set1_epi16 (m_widthm1);
}

CSplineBoundaryCondition *CRepeatBoundary::clone ()const
{
	return new CRepeatBoundary(*this); 
}

CRepeatBoundary::CRepeatBoundary(int width):
	CSplineBoundaryCondition(width), 
	m_widthm1(width-1)
{
	zero = _mm_setzero_si128();
	maxv = _mm_set1_epi16 (m_widthm1);
}

void CRepeatBoundary::test_supported(int npoles) const
{
	if (npoles > 1) {
		THROW(invalid_argument, "Currently, repeat-boundary not supported for splines woth more then one pole");  
	}
}

void CRepeatBoundary::do_set_width(int width)
{
	m_widthm1 = width-1; 
	maxv = _mm_set1_epi16 (m_widthm1);
}

void CRepeatBoundary::do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const
{
#ifdef __SSE2__
	const int size = index.size(); 
	if (size < 9) {
		__m128i buffer; 
		memcpy(&buffer, &index[0], size * sizeof(short));
		__m128i idx = _mm_load_si128(&buffer);
		idx = _mm_max_epi16(idx, zero);
		idx = _mm_min_epi16(idx, maxv);
		_mm_store_si128(&buffer, idx); 
		memcpy(&index[0], &buffer, size * sizeof(short));
	} else 
#endif	
		for (size_t k = 0; k < index.size(); k++) {
			if (index[k] < 0) 
				index[k] = 0; 
			else if (index[k] > m_widthm1) {
				index[k] = m_widthm1; 
			}	
		}
}


double CRepeatBoundary::initial_coeff(const std::vector<double>& coeff, double pole)const
{ 
	double zn = pole;
	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(2 * coeff.size()));

	double sum = 0.0; 
	for (size_t n = 0; n < coeff.size(); n++) {
		sum += (zn  + z2n) * coeff [n]; 
		zn *= pole;
		z2n *= iz;
	}
	return coeff[0] + sum / ( 1 - zn * zn); 
}

double CRepeatBoundary::initial_anti_coeff(const std::vector<double>& coeff, double pole)const
{
	return - pole / (1 - pole) * coeff[coeff.size() - 1];
}


CMirrorOnBoundaryPlugin::CMirrorOnBoundaryPlugin():
	CSplineBoundaryConditionPlugin("mirror")
{
}

CSplineBoundaryCondition *CMirrorOnBoundaryPlugin::do_create(int width) const
{
	return new CMirrorOnBoundary(width); 
}

const std::string CMirrorOnBoundaryPlugin::do_get_descr() const
{
	return "Spline interpolation boundary conditions that mirror on the boundary"; 
}


CRepeatBoundaryPlugin::CRepeatBoundaryPlugin():
	CSplineBoundaryConditionPlugin("repeat")
{
}

CSplineBoundaryCondition *CRepeatBoundaryPlugin::do_create(int width) const
{
	return new CRepeatBoundary(width); 
}

const std::string CRepeatBoundaryPlugin::do_get_descr() const
{
	return "Spline interpolation boundary conditions that repeats the value at the boundary"; 
}


CZeroBoundaryPlugin::CZeroBoundaryPlugin():
	CSplineBoundaryConditionPlugin("zero")
{
}

CSplineBoundaryCondition *CZeroBoundaryPlugin::do_create(int width) const
{
	return new CZeroBoundary(width);
}
	
const std::string CZeroBoundaryPlugin::do_get_descr() const
{
	return "Spline interpolation boundary conditions that assumes zero for values outside"; 
}


extern "C" EXPORT CPluginBase  *get_plugin_interface()
{
	CPluginBase  *result = new CZeroBoundaryPlugin(); 
	result->append_interface(new CRepeatBoundaryPlugin()); 
	result->append_interface(new CMirrorOnBoundaryPlugin()); 
	return result;
}


NS_MIA_END
