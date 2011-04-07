/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#ifndef mia_2d_splinetransform_hh
#define mia_2d_splinetransform_hh

NS_MIA_BEGIN



class C2DSplineTransformation: C2DTransformation {
public: 

	typedef T2DInterpolator<C2DFVector> Interpolator; 

	C2DSplineTransformation();

	bool save(const std::string& filename, const std::string& type) const; 

	P2DTransformation upscale(const C2DBounds& size) const; 

	void add(const C2DTransformation& a); 

	size_t degrees_of_freedom() const; 

	virtual const C2DBounds& get_size() const; 
private: 
	
	virtual bool do_save(const std::string& filename, const std::string& type) const; 

	virtual P2DTransformation do_upscale(const C2DBounds& size) const;

	virtual void do_add(const C2DTransformation& a); 

	virtual size_t do_degrees_of_freedom() const; 

	C2DFVectorfield do_translate(const C2DFVectorfield& gradient) const; 

	Interpolator m_interpolator; 
	
};



NS_MIA_END

#endif

