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

#ifndef mia_core_boundary_conditions_hh
#define mia_core_boundary_conditions_hh

#include <mia/core/msgstream.hh>
#include <mia/core/type_traits.hh>
#include <vector>
#include <memory>

NS_MIA_BEGIN

enum EBoundaryConditions  {
	bc_mirror_on_bounds, 
	bc_repeat, 
	bc_zero, 
	bc_unknown
}; 

/**
   \ingroup interpol 

   \brief Base class for B-spline interpolation boundary conditions 
   
   This class is the base class for B-spline interpolation boundary conditions. 
   In order to obtain a usable derivative, the do_apply method has to be implemented. 

   \todo for spline degrees large then 1, the pre-filtering has to tale the boundary conditionsinto account. 
 */

class CBoundaryCondition {
public: 

	CBoundaryCondition(); 

	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */

	CBoundaryCondition(int width); 

	/**
	   Apply the boundary conditions 
	   \param index - indices into the coeffisicnt domain will be changed to fit domain 
	   \param weights - according weights
	   \returns true if the index set was in the coefficient domain 
	 */
	bool apply(std::vector<int>& index, std::vector<double>& weights) const;
	
	/**
	   (re-)set the width of the supported index range 
	   \param width new width 
	 */

	void set_width(int width); 

	/// \returns the width of the coefficient domain 
	int get_width() const {
		return m_width; 
	}
	
	template <typename T> 
	void filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const;

	void filter_line(std::vector<double>& coeff, const std::vector<double>& poles) const;

	template <typename T> 
	void template_filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const;
private:

	virtual void do_apply(std::vector<int>& index, std::vector<double>& weights) const = 0;
	virtual void test_supported(int npoles) const = 0;
	
	virtual void do_set_width(int width); 

	
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const = 0;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const = 0;


	int m_width; 
}; 

typedef std::shared_ptr<CBoundaryCondition> PBoundaryCondition; 




/**
   \ingroup interpol 
   \brief Class for mirroring on the boundary
   
   This is based directly on the implementation provided by Philip Thevenaz 
*/
class CMirrorOnBoundary : public CBoundaryCondition {
public: 

	CMirrorOnBoundary(); 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CMirrorOnBoundary(int width); 
private: 
	void do_set_width(int width); 
	int get_index(int idx) const; 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;
	virtual void test_supported(int npoles) const;
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const ;

	int m_width2; 
}; 

/**
   \ingroup interpol 
   \brief Boundary condition that sets all the values outside the coefficient domain to zero 
   
   \todo the pre-filtering is not yet implemented 
*/
class CZeroBoundary : public CBoundaryCondition {
public: 

	CZeroBoundary() = default; 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CZeroBoundary(int width); 
private: 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;
	virtual void test_supported(int npoles) const;
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const ;

}; 


/**
   \ingroup interpol 
   \brief Boundary condition that repeats the value at the boundary 
   \todo the pre-filtering is not yet implemented 
*/
class CRepeatBoundary : public CBoundaryCondition {
public: 

	CRepeatBoundary(); 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CRepeatBoundary(int width); 
private: 
	void do_set_width(int width); 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;

	virtual void test_supported(int npoles) const;
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const;
	int m_widthm1; 
}; 


template <typename T, int size>
struct __dispatch_filter_line {
	static void apply(const CBoundaryCondition& bc, std::vector<T>& coeff, const std::vector<double>& poles); 
}; 

template <typename T, int size>
void __dispatch_filter_line<T, size>::apply(const CBoundaryCondition& bc, std::vector<T>& coeff, 
					 const std::vector<double>& poles) 
{
	std::vector<double> temp(coeff.size());
	for (int i = 0; i < size; ++i) {
		std::transform(coeff.begin(), coeff.end(), temp.begin(), 
			       [i](const T& x) { return x[i]; }); 
		bc.filter_line(temp, poles); 
		for (size_t j = 0; j < coeff.size(); ++j)
			coeff[j][i] = temp[j]; 
	}
}

template <typename T>
struct __dispatch_filter_line<T,1> {
	static void apply(const CBoundaryCondition& bc, std::vector<T>& coeff, const std::vector<double>& poles); 
}; 

template <typename T>
void __dispatch_filter_line<T, 1>::apply(const CBoundaryCondition& bc, std::vector<T>& coeff, 
					 const std::vector<double>& poles) 
{
	bc.template_filter_line(coeff, poles); 
}

////
template <typename T> 
void CBoundaryCondition::filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const
{
	typedef atomic_data<T> atom; 
	__dispatch_filter_line<T, atom::size>::apply(*this, coeff, poles); 
}


template <typename T> 
void CBoundaryCondition::template_filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const
{
	std::vector<double> temp(coeff.size()); 
	std::copy(coeff.begin(), coeff.end(), temp.begin()); 
	filter_line(temp, poles); 
	std::transform(temp.begin(), temp.end(), coeff.begin(), [](double x) {return static_cast<T>(x);});
}



NS_MIA_END
#endif
