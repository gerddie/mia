/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


#include <mia/2d/cost/gncc.hh>

namespace mia_2d_gncc {
          
          
CGNCC2DImageCost::CGNCC2DImageCost():
{
	m_convert_to_float = produce_2dimage_filter("convert:repn=float,map=copy");
	m_sobel_x = produce_2dimage_filter("sobel:dir=x");
	m_sobel_y = produce_2dimage_filter("sobel:dir=y");
	
}


const FRemoveMean : public TFilter<C2DFImage> {
	template <typename T>
	C2DFImage operator ()( const T2DImage<T>& image) const{
		C2DFImage result(image.get_size());
		double mean = 0.0;
		for_each(auto i: image){
			mean += i; 
		}
		mean /= image.size();
		transform(image.begin(), image.end(), result.begin(),
			  [&mean](T x) { return x - mean;});
		return result; 
	}
}; 

double CGNCC2DImageCost::do_value(const mia::C2DImage& a,
				  const mia::C2DImage& MIA_PARAM_UNUSED(b)) const
{
	auto float_image = m_convert_to_float->filter(a);
	auto agx =  m_sobel_x->filter(*float_image);
	auto agy =  m_sobel_y->filter(*float_image);

	auto& nagx = dynamic_cast<const C2DFImage&>(*agx);
	auto& nagy = dynamic_cast<const C2DFImage&>(*agy);
	
	auto inagx = nagx.begin();
	auto enagx = nagx.end();
	auto inagy = nagy.begin();
	auto inbgx = m_grad_x.begin();
	auto inbgy = m_grad_y.begin();

	NCCSums nccsum; 
	
	while (inagx != enagx) {
		nccsum.add(*inagx++, inbgx++);
		nccsum.add(*inagy++, inbgy++);
		++inagx;
		++inbgx; 
		++inagy;
		++inbgy; 
	}
	return nccsum.value(); 
}       

double CGNCC2DImageCost::do_evaluate_force(const mia::C2DImage& a,
					   const mia::C2DImage& MIA_PARAM_UNUSED(b),
					   mia::C2DFVectorfield& force) const
{
	
}       
        
void CGNCC2DImageCost::post_set_reference(const mia::C2DImage& ref)
{
	auto float_image = m_convert_to_float->filter(ref);
	auto agx =  m_sobel_x->filter(*float_image);
	auto agy =  m_sobel_y->filter(*float_image);
	
	m_grad_x = dynamic_cast<const C2DFImage&>(*agx);
	m_grad_y = dynamic_cast<const C2DFImage&>(*agy);
}       

CGNCC2DImageCostPlugin::CGNCC2DImageCostPlugin():
	mia::C2DImageCostPlugin("gncc")
{
}

mia::C2DImageCost *CGNCC2DImageCostPlugin::do_create() const
{
	return new CGNCC2DImageCost(); 
}


const std::string CGNCC2DImageCostPlugin::do_get_descr() const
{
	return "Implementation of the 2D normalized gradient correlation image similarity measures"; 
}


NS_END(NS)
