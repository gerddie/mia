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

#ifndef mia_core_boundary_conditions_hh
#define mia_core_boundary_conditions_hh

#include <mia/core/msgstream.hh>
#include <mia/core/type_traits.hh>
#include <mia/core/factory.hh>
#include <mia/core/product_base.hh>
#include <mia/core/splinekernel.hh>

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

   \brief Abstract base class for B-spline interpolation boundary conditions 
   
   This class is the abstract base class for B-spline interpolation boundary conditions. 

   The actual boundary conditions are implemented as plug-ins and instances are 
   created by calling produce_spline_boundary_condition. 
*/
class EXPORT_CORE CSplineBoundaryCondition : public CProductBase{
public: 

	/// helper typedef for plug-in handling 
	typedef CSplineBoundaryCondition plugin_data; 
	
	/// helper typedef for plug-in handling 
	typedef CSplineBoundaryCondition plugin_type; 

	/// type portion of the plugin search path 
	static const char * const type_descr; 
	
	/// data portion of the plugin search path 
	static const char * const data_descr; 


	/// pointer type to this boundary condition
	typedef std::unique_ptr<CSplineBoundaryCondition> Pointer; 

	CSplineBoundaryCondition(); 


	/**
	   Default copy constructor 
	 */
	CSplineBoundaryCondition(const CSplineBoundaryCondition& other) = default; 

	/**
	   Constructor for the boundary conditions. 
	   \param width size of the coefficent domain 
	 */
	
	CSplineBoundaryCondition(int width); 

	/**
	   Apply the boundary conditions 
	   \param index - indices into the coeffisicnt domain will be changed to fit domain 
	   \param weights - according weights
	   \returns true if the index set was in the coefficient domain 
	 */
	bool apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const;
	
	/**
	   (re-)set the width of the supported index range 
	   \param width new width 
	 */

	void set_width(int width); 

	/// \returns the width of the coefficient domain 
	int get_width() const {
		return m_width; 
	}
	
	/**
	   Prefiltering function to convert a vector of input data to spline coefficients 
	   \tparam T must either be a scalar type or an array of scalar data types whose elements 
	   can be accessed by using the operator[]. This restriction is currently necessary to 
	   allow a dynamic polymorphic implementation  of the pre-filtering step needed for 
           different boundary condition models. 
	   \param[in,out] coeff vector of function values that will be converted to spline coefficients 
	   \param poles the poles of the B-spline the coefficients are created for 
	   
	 */
	template <typename T> 
	void filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const;

        /**
	   Prefiltering function to convert a vector of double valued input data to spline coefficients 
	   This is the actual work routine that will be called by the other filter_line functions 
	   after type conversion and decomposition has been executed 
	   \param[in,out] coeff vector of function values that will be converted to spline coefficients 
	   \param poles the poles of the B-spline the coefficients are created for 
	*/
	void filter_line(std::vector<double>& coeff, const std::vector<double>& poles) const;

	/**
	   Prefiltering function to convert a vector of scalar valued input data to spline coefficients. 
	   \tparam T a scalar type. 
	   \param[in,out] coeff vector of function values that will be converted to spline coefficients 
	   \param poles the poles of the B-spline the coefficients are created for 
	*/
	
	template <typename T> 
	void template_filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const;

	/**
	   \returns a copy of the (derived) instance of this boundary condition 
	 */
	virtual 
		CSplineBoundaryCondition *clone() const __attribute__((warn_unused_result)) = 0 ; 
private:

	virtual void do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const = 0;
	virtual void test_supported(int npoles) const = 0;
	
	virtual void do_set_width(int width); 

	
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const = 0;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const = 0;


	int m_width; 
}; 
/**  \ingroup interpol 
     Pointer type of the boundary conditions. 
*/
typedef CSplineBoundaryCondition::Pointer PSplineBoundaryCondition; 

/**  \ingroup interpol 
     \brief Base plugin for spline boundary conditions
*/
class EXPORT_CORE CSplineBoundaryConditionPlugin: public TFactory<CSplineBoundaryCondition> {
public: 
	/**
	   Constructor for the spline boundary conditions plug-ins. 
	 */

	CSplineBoundaryConditionPlugin(const char * name); 
private: 
	virtual CSplineBoundaryCondition *do_create() const;
	
	virtual CSplineBoundaryCondition *do_create(int width) const = 0; 

	int m_width; 
}; 

/**
   \ingroup interpol 
   Plugin handler for the creation of spline boundary conditions
*/
typedef THandlerSingleton<TFactoryPluginHandler<CSplineBoundaryConditionPlugin> > CSplineBoundaryConditionPluginHandler;

struct EXPORT_CORE CSplineBoundaryConditionTestPath {
	CSplineBoundaryConditionTestPath(); 
}; 


/**
   Create a specific instance of a spline interpolation boundary condition. 
   \param descr Description of the requested boundary conditions
   \returns the actual boundary condition 
*/
inline 
PSplineBoundaryCondition produce_spline_boundary_condition(const std::string& descr)
{
	return CSplineBoundaryConditionPluginHandler::instance().produce_unique(descr); 
}

/**
   Create a specific instance of a spline interpolation boundary condition. 
   \param descr Description of the requested boundary conditions
   \param width width of the input domain 
   \returns the actual boundary condition 
*/
EXPORT_CORE PSplineBoundaryCondition produce_spline_boundary_condition(const std::string& descr, int width); 


template <typename T, int size>
struct __dispatch_filter_line {
	static void apply(const CSplineBoundaryCondition& bc, std::vector<T>& coeff, const std::vector<double>& poles); 
}; 

template <typename T, int size>
void __dispatch_filter_line<T, size>::apply(const CSplineBoundaryCondition& bc, std::vector<T>& coeff, 
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
	static void apply(const CSplineBoundaryCondition& bc, std::vector<T>& coeff, const std::vector<double>& poles); 
}; 

template <typename T>
void __dispatch_filter_line<T, 1>::apply(const CSplineBoundaryCondition& bc, std::vector<T>& coeff, 
					 const std::vector<double>& poles) 
{
	bc.template_filter_line(coeff, poles); 
}

////
template <typename T> 
void CSplineBoundaryCondition::filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const
{
	typedef atomic_data<T> atom; 
	__dispatch_filter_line<T, atom::size>::apply(*this, coeff, poles); 
}


template <typename T> 
void CSplineBoundaryCondition::template_filter_line(std::vector<T>& coeff, const std::vector<double>& poles) const
{
	std::vector<double> temp(coeff.size()); 
	std::copy(coeff.begin(), coeff.end(), temp.begin()); 
	filter_line(temp, poles); 
	std::transform(temp.begin(), temp.end(), coeff.begin(), [](double x) {return static_cast<T>(x);});
}



NS_MIA_END
#endif
