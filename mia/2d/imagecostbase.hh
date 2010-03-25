/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010, Gert Wollny
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <mia/2d/costbase.hh>
#include <mia/2d/2DImage.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN

class C2DImageCostBase: public C2DCostBase {
public: 
	C2DImageCostBase(P2DImage src, P2DImage ref, 
			 P2DInterpolatorFactory ipf,
			 float weight); 

protected: 
	const C2DImage& get_src() const;
	const C2DImage& get_ref() const;
	const C2DInterpolatorFactory& get_ipf() const;
private: 
	virtual double do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const; 
	virtual double do_evaluate_with_images(const C2DImage& floating, const C2DImage& ref, 
					       C2DFVectorfield& force) const = 0; 

	P2DImage _M_src;
	P2DImage _M_ref; 
	P2DInterpolatorFactory _M_ipf; 
}; 



NS_MIA_END
