/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#include <mia/2d/imagecostbase.hh>

NS_MIA_BEGIN


C2DImageCostBase::C2DImageCostBase(const C2DImageDataKey& src_key, const C2DImageDataKey& ref_key,
				   P2DInterpolatorFactory ipf,
				   float weight):
	C2DCostBase(weight),
	m_src_key(src_key),
	m_ref_key(ref_key),
	m_ipf(ipf)
{
}

P2DImage get_from_key(const C2DImageDataKey& key)
{
	C2DImageIOPlugin::PData in_image_list = key.get();

	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("ImageCostBase: no image available in data pool");

	return (*in_image_list)[0];
}


const C2DImage& C2DImageCostBase::get_src() const
{
	if (!m_src)
		m_src = get_from_key(m_src_key);
	return *m_src;
}

const C2DImage& C2DImageCostBase::get_ref() const
{
	if (!m_ref)
		m_ref = get_from_key(m_ref_key);
	return *m_ref;
}

const C2DInterpolatorFactory& C2DImageCostBase::get_ipf() const
{
	return *m_ipf;
}

double C2DImageCostBase::do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const
{
	P2DImage floating = t(get_src(), get_ipf());
	return do_evaluate_with_images(*floating, get_ref(), force);
}


C2DImageCostBasePlugin::C2DImageCostBasePlugin(const char * const name):
	C2DCostBasePlugin(name),
	m_interpolator(ip_bspline3)
{
	add_parameter("src", new CStringParameter(m_src_name, true, "study image"));
	add_parameter("ref", new CStringParameter(m_ref_name, true, "reference image"));
	add_parameter("interp", new CDictParameter<EInterpolation>(m_interpolator,
								   GInterpolatorTable, "image interpolator"));
}


EInterpolation C2DImageCostBasePlugin::get_ipf() const
{
	return m_interpolator;
}

const C2DImageDataKey C2DImageCostBasePlugin::get_src_key() const
{
	return C2DImageIOPluginHandler::instance().load_to_pool(m_src_name);
}

const C2DImageDataKey C2DImageCostBasePlugin::get_ref_key() const
{
	return C2DImageIOPluginHandler::instance().load_to_pool(m_ref_name);
}

NS_MIA_END
