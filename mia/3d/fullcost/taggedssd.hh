/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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

#ifndef mia_3d_imagefullcost_hh
#define mia_3d_imagefullcost_hh


#include <mia/3d/fullcost.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/cost.hh>

NS_BEGIN(taggedssd_3d)

class EXPORT C3DTaggedSSDCost : public mia::C3DFullCost {
 public: 
	C3DTaggedSSDCost(const std::string& src_x, 
			 const std::string& ref_x, 
			 const std::string& src_y, 
			 const std::string& ref_y, 
			 const std::string& src_z, 
			 const std::string& ref_z,
			 double weight); 
 private: 
	double do_evaluate(const mia::C3DTransformation& t, mia::CDoubleVector& gradient) const;
	void do_set_size(); 

	static mia::P3DImage get_from_pool(const mia::C3DImageDataKey& key); 

	double do_value(const mia::C3DTransformation& t) const; 

	double do_value() const; 
	void do_reinit(); 
	bool do_get_full_size(mia::C3DBounds& size) const; 

	mia::C3DImageDataKey m_src_key[3];
	mia::C3DImageDataKey m_ref_key[3];
	
	mia::P3DImage m_src[3]; 
	mia::P3DImage m_ref[3]; 
	
	mia::P3DImage m_src_scaled[3]; 
	mia::P3DImage m_ref_scaled[3];

	bool m_debug;
}; 

NS_END

#endif
