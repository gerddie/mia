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
	void filter_line(std::vector<T>& coeff, const std::vector<double>& poles)const;
private:
	template <typename T> 
	T initial_coeff(const std::vector<T>& coeff, double pole)const;
	
	template <typename T>
	T initial_anti_coeff(const std::vector<T>& coeff, double pole)const;

	virtual int get_index(int idx) const = 0; 

	virtual void do_apply(std::vector<int>& index, std::vector<double>& weights) const = 0;
	
	virtual void do_set_width(int width); 

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
	int get_index(int idx) const; 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;
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
	int get_index(int idx) const; 
	void do_set_width(int width); 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;
	int m_widthm1; 
}; 


////
template <typename T>
void CBoundaryCondition::filter_line(std::vector<T>& coeff, const std::vector<double>& poles)const
{
	/* special case required by mirror boundaries */
	if (coeff.size() < 2) {
		return;
	}

	/// interpolating splines? 
	if (poles.empty()) 
		return; 
	
	/* compute the overall gain */
	double	lambda = 1.0;
	for (size_t k = 0; k < poles.size() ; ++k) {
		lambda  *=  2 - poles[k] - 1.0 / poles[k];
	}
	
	/* apply the gain */
	for_each(coeff.begin(), coeff.end(), [lambda](T& x) { x *= lambda;});
	
	/* loop over all poles */
	for (size_t k = 0; k < poles.size(); ++k) {
		/* causal initialization */
		coeff[0] = initial_coeff(coeff, poles[k]);
		cvdebug() << "initial coeff  ["<< k <<"]= " << coeff[0] << "\n"; 

		/* causal recursion */
		for (size_t n = 1; n < coeff.size(); ++n) {
			coeff[n] += poles[k] * coeff[n - 1];
			cvdebug() << "causal[" << n << "] = "<<coeff[n] << "\n"; 
		}
		
		/* anticausal initialization */
		coeff[coeff.size() - 1] = initial_anti_coeff(coeff, poles[k]);
		cvdebug() << "initial anti- coeff  ["<< coeff.size() - 1 <<"]= " << coeff[coeff.size() - 1] << "\n"; 
		/* anticausal recursion */
		for (int n = coeff.size() - 2; 0 <= n; n--) {
			cvdebug() << "anticoeff["<< n <<"]= " 
				  << poles[k] << " * (" << coeff[n + 1]  << " - " << coeff[n] << ") = " ; 
			coeff[n] = poles[k] * (coeff[n + 1] - coeff[n]);
			cverb << coeff[n] << "\n"; 
		}
	}
}

template <typename T>
T CBoundaryCondition::initial_coeff(const std::vector<T>& coeff, double pole)const
{ 
	/* full loop */
	int idx_butlast = this->get_index(coeff.size() - 1); 

	if (idx_butlast < 0) 
		return T();

	double zn = pole;

	if (idx_butlast == 0) 
		return 1.0 / (1 - pole) * coeff[0];


	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(coeff.size() - 1));
	T sum = coeff[0] + z2n * coeff[coeff.size() - 1];
	
	z2n *= z2n * iz;
	
	for (size_t n = 1; n < coeff.size()  - 1; n++) {
		sum += (zn + z2n) * coeff[n];
		zn *= pole;
		z2n *= iz;
	}
	
	return(sum / (1.0 - zn * zn));
}

template <typename T>
T CBoundaryCondition::initial_anti_coeff(const std::vector<T>& coeff, double pole)const
{
	int idx_butlast = this->get_index(coeff.size() - 1); 
	if (idx_butlast == coeff.size() - 1) 
		return ((pole / (pole * pole - 1.0)) * 
			(pole * coeff[coeff.size() - 2] + coeff[coeff.size() - 1]));
	
	if (idx_butlast == 0)
		return - ( 1.0 / (1 - pole) -1) * coeff[coeff.size() - 1];

	return T(); 
}


NS_MIA_END
#endif
