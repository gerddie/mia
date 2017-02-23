/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_core_fastica_nonlinearity_hh
#define mia_core_fastica_nonlinearity_hh

#include <mia/core/gsl_vector.hh>
#include <mia/core/gsl_matrix.hh>

#include <mia/core/factory.hh>
namespace mia {


class EXPORT_CORE CFastICANonlinearityBase : public CProductBase {
public: 

	/// helper typedef for plugin handling 
	typedef CFastICANonlinearityBase plugin_data; 

	static const char *data_descr; 

	CFastICANonlinearityBase(); 

        void set_signal(const gsl::Matrix *signal); 
        void set_mu(double m); 

protected: 
        double get_sample_scale() const {return m_sample_scale;}
	double get_mu() const { return m_mu;}; 
        const gsl::Matrix& get_signal() const; 

private: 
        virtual void post_set_signal() = 0;
	double m_mu;
	double m_sample_scale;
        const gsl::Matrix *m_signal; 
}; 

/**
   \brief This is the base clase for non-linearities used in deflation based ICA
   
   This class defines the interface of the nonlinearity g for deflation based FastICA. In order to 
   implement the a real non-linearity the method get_correction_and_scale must be overwritten. 
   
   If the factor \f$\mu\f$ defined in the parent class is \f$\ge 1.0\f$, than the normal implementation 
   will be used, if the value is positive but \f$\le 1.0\f$ then the stabelized variant of the algorithm 
   is used. 
   
*/

class EXPORT_CORE CFastICADeflNonlinearity : public CFastICANonlinearityBase {
public: 

	/// helper typedef for plugin handling 
	typedef CFastICADeflNonlinearity plugin_type; 

	static const char *type_descr; 


        void apply(gsl::Vector& w); 
	void apply(gsl::Matrix& W);

	std::vector<double> get_saddle_test_table(const gsl::Matrix& ics) const;  
	double get_saddle_test_value(const gsl::Vector& ic) const;

protected: 
	virtual void post_set_signal();
private: 
	/**
	   Key worker function of the class that needs to be overwritten. Given the signal X and the 
	   input vector \a w passed to \a apply the parameters are to interpreted as follows: 
	   \param XTw the vector resulting from the multiplication of $X^T w$. This vector will be overwritten. 
	   \param correction [in,out] a vector of the same size like \a w. On output it must contain the 
	   correction $X g(X^T w)$
	 */

	virtual double get_correction_and_scale(gsl::Vector& XTw, gsl::Vector& correction) = 0; 
	
	/**
	   This function evaluates the SIR quantity needed for the saddle test
	   \param ic independet component to evaluate the SIR from 
	 */
	virtual double do_get_saddle_test_value(const gsl::Vector& ic) const = 0;
	void sum_final(gsl::Vector& w, double scale); 
	void sum_final_stabelized(gsl::Vector& w, double scale); 

        gsl::Vector m_XTw;
        gsl::Vector m_workspace; 
}; 


typedef std::shared_ptr<CFastICADeflNonlinearity> PFastICADeflNonlinearity; 


EXPORT_CORE PFastICADeflNonlinearity produce_fastica_nonlinearity(const std::string& descr); 


typedef TFactory<CFastICADeflNonlinearity> CFastICADeflNonlinearityPlugin; 

/**
   \ingroup interpol 
   Plugin handler for the creation of deflation FastICA nonlinearities 
*/
typedef THandlerSingleton<TFactoryPluginHandler<CFastICADeflNonlinearityPlugin> > CFastICADeflNonlinearityPluginHandler;

}

#endif 
