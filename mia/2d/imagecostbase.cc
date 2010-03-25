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


#include <mia/2d/imagecostbase.hh>

NS_MIA_BEGIN


C2DImageCostBase::C2DImageCostBase(P2DImage src, P2DImage ref, 
				   P2DInterpolatorFactory ipf,
				   float weight):
	C2DCostBase(weight), 
	_M_src(src), 
	_M_ref(ref),
	_M_ipf(ipf)
{
	assert(src->get_size() == ref->get_size());
}

const C2DImage& C2DImageCostBase::get_src() const
{
	return *_M_src; 
}

const C2DImage& C2DImageCostBase::get_ref() const
{
	return *_M_ref; 
}

const C2DInterpolatorFactory& C2DImageCostBase::get_ipf() const
{
	return *_M_ipf; 
}

double C2DImageCostBase::do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const
{
	P2DImage floating = t(get_src(), get_ipf()); 
	return do_evaluate_with_images(*floating, get_ref(), force); 
}

NS_MIA_END
