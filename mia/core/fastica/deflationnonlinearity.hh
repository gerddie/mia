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

#ifndef mia_core_fastica_deflationnonlinearity_hh
#define mia_core_fastica_deflationnonlinearity_hh

#include <mia/core/fastica_nonlinearity.hh>

NS_BEGIN(fastica_deflnonlin)

class CFastICADeflPow3 : public mia::CFastICADeflNonlinearity {
	virtual void do_apply(gsl::DoubleVector& w); 
	virtual void do_apply_stabelized(gsl::DoubleVector& w);
	void common_evaluations(); 
}; 

class CFastICADeflTanh : public mia::CFastICADeflNonlinearity {
public: 
        CFastICADeflTanh(double a); 
private: 
	virtual void do_apply(gsl::DoubleVector& w);
	virtual void do_apply_stabelized(gsl::DoubleVector& w);
	double common_evaluations_and_scale(); 
        double m_a; 
}; 

class CFastICADeflGauss : public mia::CFastICADeflNonlinearity {
public: 
        CFastICADeflGauss(double a); 
private: 

	virtual void do_apply(gsl::DoubleVector& w);
	virtual void do_apply_stabelized(gsl::DoubleVector& w);
	virtual void post_set_signal();
	double common_evaluations_and_scale(); 
        gsl::DoubleVector m_usquared;
	gsl::DoubleVector m_ex; 
        double m_a; 
}; 

class CFastICADeflPow3Plugin: public mia::CFastICADeflNonlinearityPlugin {
public: 
        CFastICADeflPow3Plugin(); 
        
        virtual mia::CFastICADeflNonlinearity *do_create() const;
        
	virtual const std::string do_get_descr()const;

}; 


class CFastICADeflTanhPlugin: public mia::CFastICADeflNonlinearityPlugin {
public: 
        CFastICADeflTanhPlugin(); 
private: 
        virtual mia::CFastICADeflNonlinearity *do_create() const;
        
	virtual const std::string do_get_descr()const;
        double m_a; 

}; 

class CFastICADeflGaussPlugin: public mia::CFastICADeflNonlinearityPlugin {
public: 
        CFastICADeflGaussPlugin(); 
private:         
        virtual mia::CFastICADeflNonlinearity *do_create() const;
        
	virtual const std::string do_get_descr()const;
        double m_a; 

}; 

NS_END

#endif 


