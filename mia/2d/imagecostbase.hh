/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#include <mia/2d/costbase.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN

class EXPORT_2D C2DImageCostBase: public C2DCostBase {
public:
	C2DImageCostBase(const C2DImageDataKey& src_key, const C2DImageDataKey& ref_key,
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

	const C2DImageDataKey m_src_key;
	const C2DImageDataKey m_ref_key;

	mutable P2DImage m_src;
	mutable P2DImage m_ref;
	P2DInterpolatorFactory m_ipf;
};

class EXPORT_2D  C2DImageCostBasePlugin: public C2DCostBasePlugin {
public:
	C2DImageCostBasePlugin(const char * const name);
protected:
	EInterpolation get_ipf() const;
	const C2DImageDataKey get_src_key() const;
	const C2DImageDataKey get_ref_key() const;
private:
	std::string m_src_name;
	std::string m_ref_name;
	EInterpolation m_interpolator;
};


NS_MIA_END
