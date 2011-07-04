/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

/*
  LatexBeginPluginSection{2D "fat" cost functions}
  \label{sec:fatcost2d}
  
  This cost function implementation is obsolete and will be replaced by 
  \hyperref[sec:2dfullcost]{2D fullcost}. 
  
  LatexEnd
*/


#include <mia/core/export_handler.hh>

#include <stdexcept>
#include <sstream>

#include <mia/2d/fatcost.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/core/fatcost.cxx>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using namespace std;
using namespace boost;

C2DImageFatCost::C2DImageFatCost(P2DImage src, P2DImage ref,
				 P2DInterpolatorFactory ipf, float weight):
	TFatCost<C2DTransformation, C2DFVectorfield>(src, ref, ipf, weight)
{
	if (src->get_size() != ref->get_size()) {
		throw invalid_argument("C2DImageFatCost: input images must be of same size");
	}
}

P2DImageFatCost C2DImageFatCost::get_downscaled(const C2DBounds& scale) const
{
	stringstream downscalerstr;
	downscalerstr << "downscale:bx=" << scale.x << ",by=" << scale.y;
	std::shared_ptr<C2DFilter > downscaler = C2DFilterPluginHandler::instance().produce(downscalerstr.str().c_str());

	P2DImage src_scaled = downscaler->filter(get_src());
	P2DImage ref_scaled = downscaler->filter(get_ref());
	return cloned(src_scaled, ref_scaled);
}

C2DBounds C2DImageFatCost::get_size() const
{
	return get_src().get_size();
}

C2DFatImageCostPlugin::C2DFatImageCostPlugin(const char *name):
	TFactory<C2DImageFatCost>(name),
	m_interpolator(CSplineKernelPluginHandler::instance().produce("bspline:d=3")),
	m_weight(1.0f)

{
	TRACE("C2DFatImageCostPlugin::C2DFatImageCostPlugin");
	add_parameter("src", new CStringParameter(m_src_name, true, "study image"));
	add_parameter("ref", new CStringParameter(m_ref_name, true, "reference image"));
	add_parameter("interp", new CFactoryParameter<CSplineKernelPluginHandler>(m_interpolator, false, "image interpolator kernel"));
	add_parameter("weight", new CFloatParameter(m_weight, 1e-10f, 1e+10f,
						    false, "weight of cost function"));
}

P2DImageFatCost C2DFatImageCostPlugin::create_directly( P2DImage src, P2DImage ref,
							P2DInterpolatorFactory ipf,
							float weight)
{
	return do_create(src, ref, ipf, weight);
}

C2DFatImageCostPlugin::ProductPtr C2DFatImageCostPlugin::do_create()const
{
	TRACE("C2DFatImageCostPlugin::do_create");

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	typedef C2DImageIOPluginHandler::Instance::PData PImageVector;


        PImageVector reference = imageio.load(m_ref_name);
        if (reference->empty()) {
                stringstream msg;
                msg << "Unable to load images form " << m_ref_name;
                throw invalid_argument(msg.str());
        }

        PImageVector source    = imageio.load(m_src_name);
        if (source->empty()) {
                stringstream msg;
                msg << "Unable to load images form " << m_src_name;
                throw invalid_argument(msg.str());
		}

		if (source->size() > 1)
			cvwarn() << "'" << m_src_name << "' contains more then one image, using only first\n";

		if (reference->size() > 1)
			cvwarn() << "'" << m_ref_name << "' contains more then one image, using only first\n";

		P2DInterpolatorFactory ipf(new C2DInterpolatorFactory(ipf_spline, m_interpolator));

		return do_create((*source)[0], (*reference)[0], ipf, m_weight);
	}

double C2DImageFatCostList::value() const
{
	TRACE("C2DImageFatCostList::value");
	double value = 0.0;
	for (const_iterator i = begin(); i != end(); ++i)
		value += (*i)->value();
	return value;
}

double C2DImageFatCostList::evaluate_force(C2DFVectorfield& force) const
{
	TRACE("C2DImageFatCostList::evaluate_force");
	double value = 0.0;
	for (const_iterator i = begin(); i != end(); ++i)
		value += (*i)->evaluate_force(force);
	return value;
}

C2DImageFatCostList C2DImageFatCostList::get_downscaled(const C2DBounds& scale) const
{
	C2DImageFatCostList result;

	for (const_iterator i = begin(); i != end(); ++i)
		result.push_back((*i)->get_downscaled(scale));

	return result;
}

C2DBounds C2DImageFatCostList::get_size() const
{
	if (!empty())
		return (*this)[0]->get_size();
	return C2DBounds(0,0);
}

void C2DImageFatCostList::transform(const C2DTransformation& transform)
{
	TRACE("C2DImageFatCostList::transform");
	for (iterator i = begin(); i != end(); ++i)
		(*i)->transform(transform);
}


C2DFatImageCostPluginHandlerImpl::C2DFatImageCostPluginHandlerImpl(const std::list<boost::filesystem::path>& searchpath)
	:TFactoryPluginHandler<C2DFatImageCostPlugin>(searchpath)
{
}

P2DImageFatCost C2DFatImageCostPluginHandlerImpl::create_directly(const std::string& plugin, P2DImage src,
								  P2DImage ref,
								  P2DInterpolatorFactory ipf,
								  float weight) const
{
	C2DFatImageCostPlugin *factory = this->plugin(plugin.c_str());
	if (!factory)
		throw invalid_argument(string("C2DFatImageCostPluginHandlerImpl::create_directly: '") +
				       plugin + "' unknown");
	return factory->create_directly(src, ref, ipf, weight);
}

template class TFatCost<C2DTransformation, C2DFVectorfield>;

template class TPlugin<C2DImage, fatcost_type>;
template class TFactory<C2DImageFatCost>;

template class TPluginHandler<C2DFatImageCostPlugin>;
template class TFactoryPluginHandler<C2DFatImageCostPlugin>;
template class THandlerSingleton<C2DFatImageCostPluginHandlerImpl>;

NS_MIA_END
