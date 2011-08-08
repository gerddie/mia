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


#ifndef mia_core_splinebc_bc_hh
#define mia_core_splinebc_bc_hh

#include <mia/core/boundary_conditions.hh>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

NS_MIA_BEGIN

/*
   \ingroup interpol 
   \brief Class for mirroring on the boundary
   
   This is based directly on the implementation provided by Philip Thevenaz 
*/
class CMirrorOnBoundary : public CSplineBoundaryCondition {
public: 

	CMirrorOnBoundary(); 
	CMirrorOnBoundary(const CMirrorOnBoundary& other) = default; 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */

	CMirrorOnBoundary(int width); 
	CSplineBoundaryCondition *clone() const; 
private: 
	void do_set_width(int width); 
	int get_index(int idx) const; 
	void do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const;
	virtual void test_supported(int npoles) const;
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const ;

	int m_width2; 
}; 

/*
   \ingroup interpol 
   \brief Boundary condition that sets all the values outside the coefficient domain to zero 
   
   \todo the pre-filtering is not yet implemented 
*/
class CZeroBoundary : public CSplineBoundaryCondition {
public: 

	CZeroBoundary() = default; 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CZeroBoundary(int width); 
	CZeroBoundary(const CZeroBoundary& other) = default; 

	CSplineBoundaryCondition *clone() const; 
private: 
	void do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const;
	virtual void test_supported(int npoles) const;
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const ;

}; 


/*
   \ingroup interpol 
   \brief Boundary condition that repeats the value at the boundary 
   \todo the pre-filtering is not yet implemented 
*/
class CRepeatBoundary : public CSplineBoundaryCondition {
public: 

	CRepeatBoundary(); 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CRepeatBoundary(int width); 
	CRepeatBoundary(const CRepeatBoundary& other) = default; 
	
	CSplineBoundaryCondition *clone() const; 
private: 
	void do_set_width(int width); 
	void do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const;

	virtual void test_supported(int npoles) const;
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const;
	int m_widthm1; 
#ifdef __SSE2__
	__m128i zero; 
	__m128i maxv;
#endif
}; 


class CMirrorOnBoundaryPlugin: public CSplineBoundaryConditionPlugin {
public: 
	CMirrorOnBoundaryPlugin(); 
private: 
	virtual CSplineBoundaryCondition *do_create(int width) const; 
	virtual const std::string do_get_descr() const; 
}; 

class CRepeatBoundaryPlugin: public CSplineBoundaryConditionPlugin {
public: 
	CRepeatBoundaryPlugin(); 
private: 
	virtual CSplineBoundaryCondition *do_create(int width) const; 
	virtual const std::string do_get_descr() const; 
}; 

class CZeroBoundaryPlugin: public CSplineBoundaryConditionPlugin {
public: 
	CZeroBoundaryPlugin(); 
private: 
	virtual CSplineBoundaryCondition *do_create(int width) const; 
	virtual const std::string do_get_descr() const; 
}; 


NS_MIA_END

#endif
