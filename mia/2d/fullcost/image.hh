/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
 *
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

#ifndef mia_2d_imagefullcost_hh
#define mia_2d_imagefullcost_hh


#include <mia/2d/fullcost.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/cost.hh>

NS_MIA_BEGIN

class EXPORT C2DImageFullCost : public C2DFullCost {
public: 
	C2DImageFullCost(const std::string& src, 
			 const std::string& ref, 
			 const std::string& cost, 
			 EInterpolation ip_type, 
			 double weight, 
			 bool debug); 
private: 
	double do_evaluate(const C2DTransformation& t, CDoubleVector& gradient) const;
	void do_set_size(); 

	static P2DImage get_from_pool(const C2DImageDataKey& key); 

	bool do_has(const char *property) const; 
	double do_value(const C2DTransformation& t) const; 

	double do_value() const; 
	void do_reinit(); 

	C2DImageDataKey _M_src_key;
	C2DImageDataKey _M_ref_key;
	
	P2DImage _M_src; 
	P2DImage _M_ref; 

	P2DImage _M_src_scaled; 
	P2DImage _M_ref_scaled; 

	P2DImageCost _M_cost_kernel; 
	P2DInterpolatorFactory _M_ipf; 
	bool _M_debug;
}; 

NS_MIA_END

#endif
