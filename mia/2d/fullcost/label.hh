/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_2d_imagefullcost_hh
#define mia_2d_imagefullcost_hh


#include <mia/2d/fullcost.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/cost.hh>

NS_MIA_BEGIN

class EXPORT C2DLabelFullCost : public C2DFullCost {
public: 
	C2DLabelFullCost(const std::string& src, 
                         const std::string& ref, 
                         double weight, 
                         int maxlabels, 	
			 int debug); 
private: 
	double do_evaluate(const C2DTransformation& t, CDoubleVector& gradient) const;
	void do_set_size(); 

	static P2DImage get_from_pool(const C2DImageDataKey& key); 

	double do_value(const C2DTransformation& t) const; 
	bool do_get_full_size(C2DBounds& size) const; 

        double value(int idx, int label) const; 
        double value_and_gradient(int idx, int label, C2DFVector& gradient, const C2DBounds& pos, int boundaries) const; 
        void prepare_distance_fields(const C2DUBImage& image); 
        
	double do_value() const; 
	void do_reinit(); 

	C2DImageDataKey m_src_key;
	C2DImageDataKey m_ref_key;
	
	P2DImage m_src; 
	P2DImage m_ref; 

	C2DUBImage m_src_scaled; 
	C2DUBImage m_ref_scaled; 

        std::vector<bool> m_ref_label_exists; 
        std::vector<C2DFImage> m_ref_distances;

	enum EBoundaries {
		eb_none = 0, 
		eb_xlow  = 1, /**< at low x-boundary */ 
		eb_xhigh = 2, /**< at high x-boundary */  
		eb_x = 3, /**< at high x-boundary */  
		eb_ylow = 4,  /**< at low y-boundary */ 
		eb_yhigh = 8, /**< at high y-boundary */
		eb_y = 12, /**< any y-boundary */
	}; 
 
	int m_debug; 
}; 

// plugin implementation 
class C2DLabelFullCostPlugin: public C2DFullCostPlugin {
public: 
	C2DLabelFullCostPlugin(); 
private: 
	C2DFullCost *do_create(float weight) const;
	const std::string do_get_descr() const;


	std::string m_src_name;
	std::string m_ref_name;
        int m_maxlabel; 
	int m_debug; 
}; 


NS_MIA_END

#endif
