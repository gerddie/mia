/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_core_splinekernel_bspline_hh
#define mia_core_splinekernel_bspline_hh

#include <mia/core/splinekernel.hh>

NS_MIA_BEGIN
NS_BEGIN(bsplinekernel)

/** implements a B-Spline kernel of degree 0 */
class CBSplineKernel0: public  CSplineKernel{
public:
	CBSplineKernel0();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, VWeight& weight, int order) const;
};

/** implements a B-Spline kernel of degree 0 */
class CBSplineKernel1: public  CSplineKernel{
public:
	CBSplineKernel1();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, VWeight& weight, int order) const;
};


/** implements a B-Spline kernel of degree 2 */
class CBSplineKernel2: public  CSplineKernel{
public:
	CBSplineKernel2();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, VWeight& weight, int order) const;
};

/** implements a B-Spline kernel of degree 3 */
class CBSplineKernel3: public  CSplineKernel{
public:
	CBSplineKernel3();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, VWeight& weight, int order) const;
private: 
};

/** implements a B-Spline kernel of degree 4 */
class CBSplineKernel4: public  CSplineKernel{
public:
	CBSplineKernel4();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, VWeight& weight, int order) const;
private: 
};

/** implements a B-Spline kernel of degree 5 */
class CBSplineKernel5: public  CSplineKernel{
public:
	CBSplineKernel5();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, VWeight& weight, int order) const;
};

/** implements a o-Moms kernel of degree 3 */
class CBSplineKernelOMoms3 : public  CSplineKernel{
public:
	CBSplineKernelOMoms3();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	void get_derivative_weights(double x, VWeight& weight, int order) const;
};

class CBSplineKernelPlugin: public CSplineKernelPlugin {
public: 
	CBSplineKernelPlugin(); 
	virtual CSplineKernel *do_create() const;
	virtual const std::string do_get_descr()const;
private: 
	int m_degree; 
}; 


class COMomsSplineKernelPlugin: public CSplineKernelPlugin {
public: 
	COMomsSplineKernelPlugin(); 
	virtual CSplineKernel *do_create() const;
	virtual const std::string do_get_descr()const;
private: 
	int m_degree; 
}; 

NS_END
NS_MIA_END

#endif
