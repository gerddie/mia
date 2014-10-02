/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_core_fastica_hh
#define mia_core_fastica_hh

#include <gsl++/vector.hh>
#include <gsl++/matrix.hh>

#include <mia/core/factory.hh>
namespace mia {


class EXPORT_CORE CFastICANonlinearityBase : public CProductBase {
public: 

	/// helper typedef for plugin handling 
	typedef CFastICANonlinearityBase plugin_data; 

	static const char *data_descr; 

	void set_sample(double sample_size, size_t num_samples); 
        void set_signal(const gsl::Matrix *signal); 
        void set_scaling(double myy); 
protected: 
        double get_sample_size() const; 
        size_t get_num_samples() const; 
        double get_scaling() const; 
        const gsl::Matrix& get_signal() const; 
private: 
        virtual void post_set_signal() = 0;
        double m_sample_size; 
        size_t m_num_samples; 
        const gsl::Matrix *m_signal; 
}; 

class EXPORT_CORE CFastICADeflNonlinearity : public CFastICANonlinearityBase {
public: 

	/// helper typedef for plugin handling 
	typedef CFastICADeflNonlinearity plugin_type; 

	static const char *type_descr; 
	
        void apply(gsl::DoubleVector& w); 
protected: 
        virtual void do_apply(gsl::DoubleVector& w) = 0; 
	virtual void post_set_signal();
	gsl::DoubleVector& get_XTw(){return m_XTw;}; 
	gsl::DoubleVector& get_workspace(){return m_workspace;}; 
private: 
        gsl::DoubleVector m_XTw;
        gsl::DoubleVector m_workspace; 
}; 


class EXPORT_CORE CFastICASymmNonlinearity : public CFastICANonlinearityBase {
public: 
	typedef CFastICASymmNonlinearity plugin_type; 
	static const char *type_descr;  
protected: 
        virtual void do_apply(gsl::Matrix& W, gsl::Matrix& wtX)  = 0; 
	virtual void post_set_signal();
private: 
        gsl::Matrix m_matrix_workspace; 
}; 

typedef TFactory<CFastICADeflNonlinearity> CFastICADeflNonlinearityPlugin; 

/**
   \ingroup interpol 
   Plugin handler for the creation of deflation FastICA nonlinearities 
*/
typedef THandlerSingleton<TFactoryPluginHandler<CFastICADeflNonlinearityPlugin> > CFastICADeflNonlinearityPluginHandler;


typedef TFactory<CFastICASymmNonlinearity> CFastICASymmNonlinearityPlugin; 

/**
   \ingroup interpol 
   Plugin handler for the creation of deflation FastICA nonlinearities 
*/
typedef THandlerSingleton<TFactoryPluginHandler<CFastICASymmNonlinearityPlugin> > CFastICASymmNonlinearityPluginHandler;


}

#endif 
