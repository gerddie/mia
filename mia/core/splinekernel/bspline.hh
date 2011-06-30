/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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

#ifndef mia_core_splinekernel_bspline_hh
#define mia_core_splinekernel_bspline_hh

#include <mia/core/splinekernel.hh>

NS_MIA_BEGIN
NS_BEGIN(bsplinekernel)

/** implements a B-Spline kernel of degree 0 */
class EXPORT_CORE CBSplineKernel0: public  CSplineKernel{
public:
	CBSplineKernel0();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

/** implements a B-Spline kernel of degree 0 */
class EXPORT_CORE CBSplineKernel1: public  CSplineKernel{
public:
	CBSplineKernel1();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};


/** implements a B-Spline kernel of degree 2 */
class EXPORT_CORE CBSplineKernel2: public  CSplineKernel{
public:
	CBSplineKernel2();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

/** implements a B-Spline kernel of degree 3 */
class EXPORT_CORE CBSplineKernel3: public  CSplineKernel{
public:
	CBSplineKernel3();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
private: 
};

/** implements a B-Spline kernel of degree 4 */
class EXPORT_CORE CBSplineKernel4: public  CSplineKernel{
public:
	CBSplineKernel4();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
private: 
};

/** implements a B-Spline kernel of degree 5 */
class EXPORT_CORE CBSplineKernel5: public  CSplineKernel{
public:
	CBSplineKernel5();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	virtual double get_weight_at(double x, int order) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

/** implements a o-Moms kernel of degree 3 */
class EXPORT_CORE CBSplineKernelOMoms3 : public  CSplineKernel{
public:
	CBSplineKernelOMoms3();
	virtual void get_weights(double x, std::vector<double>& weight)const;
	virtual void get_derivative_weights(double x, std::vector<double>& weight) const;
	void get_derivative_weights(double x, std::vector<double>& weight, int order) const;
};

NS_END
NS_MIA_END

#endif
