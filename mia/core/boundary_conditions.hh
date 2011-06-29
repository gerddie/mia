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

#include <mia/core/defines.hh>
#include <vector>
#include <memory>

NS_MIA_BEGIN

/**
   \brief Base class for B-spline interpolation boundary conditions 
   
   This class is the base class for B-spline interpolation boundary conditions. 
   In order to obtain a usable derivative, the do_apply method has to be implemented. 
 */

class CBoundaryCondition {
public: 

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

	/// \returns the width of the coefficient domain 
	int get_width() const {
		return m_width; 
	}
private: 
	virtual void do_apply(std::vector<int>& index, std::vector<double>& weights) const = 0;
	

	int m_width; 
}; 

typedef std::shared_ptr<CBoundaryCondition> PBoundaryCondition; 
/**
   \brief Class for mirroring on the the boundary
 */


class CMirrorOnBoundary : public CBoundaryCondition {
public: 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CMirrorOnBoundary(int width); 
private: 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;
	int m_width2; 
}; 

/**
   \brief Boundary condition that sets all the values outside the coefficient domain to zero 

*/

class CZeroBoundary : public CBoundaryCondition {
public: 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CZeroBoundary(int width); 
private: 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;
}; 


/**
   \brief Boundary condition that repeats the value at the boundary 

*/
class CRepeatBoundary : public CBoundaryCondition {
public: 
	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	CRepeatBoundary(int width); 
private: 
	void do_apply(std::vector<int>& index, std::vector<double>& weights) const;
	int m_widthm1; 
}; 

NS_MIA_END
#endif
