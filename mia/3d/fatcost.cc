/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/core/export_handler.hh>

#include <stdexcept>
#include <sstream>

#include <mia/3d/fatcost.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/core/fatcost.cxx>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using namespace std;
using namespace boost;

C3DImageFatCost::C3DImageFatCost(P3DImage src, P3DImage ref, float weight):
	TFatCost<C3DTransformation, C3DFVectorfield>(src, ref, weight)
{
	if (src->get_size() != ref->get_size()) {
		throw invalid_argument("C3DImageFatCost: input images must be of same size");
	}
}

P3DImageFatCost C3DImageFatCost::get_downscaled(const C3DBounds& scale) const
{
	stringstream downscalerstr;
	downscalerstr << "downscale:bx=" << scale.x << ",by=" << scale.y << ",bz=" << scale.z;
	std::shared_ptr<C3DFilter > downscaler = C3DFilterPluginHandler::instance().produce(downscalerstr.str().c_str());

	P3DImage src_scaled = downscaler->filter(get_src());
	P3DImage ref_scaled = downscaler->filter(get_ref());
	return cloned(src_scaled, ref_scaled);
}

C3DBounds C3DImageFatCost::get_size() const
{
	return get_src().get_size();
}

C3DFatImageCostPlugin::C3DFatImageCostPlugin(const char *name):
	TFactory<C3DImageFatCost>(name),
	m_weight(1.0f)
{
	TRACE("C3DFatImageCostPlugin::C3DFatImageCostPlugin");
	add_parameter("src", new CStringParameter(m_src_name, true, "study image"));
	add_parameter("ref", new CStringParameter(m_ref_name, true, "reference image"));
	add_parameter("interp", make_param(m_interpolator, "bspline:d=3", false, "image interpolator kernel"));
	add_parameter("weight", new CFloatParameter(m_weight, 1e-10f, 1e+10f,
						    false, "weight of cost function"));
}

C3DImageFatCost *C3DFatImageCostPlugin::create_directly( P3DImage src, P3DImage ref, P3DInterpolatorFactory ipf, float weight)
{
	return do_create(src, ref, ipf, weight);
}

C3DImageFatCost *C3DFatImageCostPlugin::do_create()const
{
	TRACE("C3DFatImageCostPlugin::do_create");

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();
	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;


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

	P3DInterpolatorFactory ipf( new C3DInterpolatorFactory(m_interpolator, "mirror")); 
	return do_create((*source)[0], (*reference)[0], ipf, m_weight);
}

double C3DImageFatCostList::value() const
{
	TRACE("C3DImageFatCostList::value");
	double value = 0.0;
	for (const_iterator i = begin(); i != end(); ++i)
		value += (*i)->value();
	return value;
}

double C3DImageFatCostList::evaluate_force(C3DFVectorfield& force) const
{
	TRACE("C3DImageFatCostList::evaluate_force");
	double value = 0.0;
	for (const_iterator i = begin(); i != end(); ++i)
		value += (*i)->evaluate_force(force);
	return value;
}

C3DImageFatCostList C3DImageFatCostList::get_downscaled(const C3DBounds& scale) const
{
	C3DImageFatCostList result;

	for (const_iterator i = begin(); i != end(); ++i)
		result.push_back((*i)->get_downscaled(scale));

	return result;
}

C3DBounds C3DImageFatCostList::get_size() const
{
	if (!empty())
		return (*this)[0]->get_size();
	return C3DBounds(0,0, 0);
}

void C3DImageFatCostList::transform(const C3DTransformation& transform)
{
	TRACE("C3DImageFatCostList::transform");
	for (iterator i = begin(); i != end(); ++i)
		(*i)->transform(transform);
}


C3DFatImageCostPluginHandlerImpl::C3DFatImageCostPluginHandlerImpl(const std::list<boost::filesystem::path>& searchpath)
	:TFactoryPluginHandler<C3DFatImageCostPlugin>(searchpath)
{
}

C3DImageFatCost *C3DFatImageCostPluginHandlerImpl::create_directly(const std::string& plugin, P3DImage src,
								  P3DImage ref, P3DInterpolatorFactory ipf,
								  float weight) const
{
	C3DFatImageCostPlugin *factory = this->plugin(plugin.c_str());
	if (!factory)
		throw invalid_argument(string("C3DFatImageCostPluginHandlerImpl::create_directly: '") +
				       plugin + "' unknown");
	return factory->create_directly(src, ref, ipf, weight);
}

template <> const char *  const 
TPluginHandler<C3DFatImageCostPlugin>::m_help =  
   "These plug-ins implement image similarity measures (obsolete).";

template class TFatCost<C3DTransformation, C3DFVectorfield>;
template class TPlugin<C3DImage, fatcost_type>;
template class TFactory<C3DImageFatCost>;
template class TPluginHandler<C3DFatImageCostPlugin>;
template class TFactoryPluginHandler<C3DFatImageCostPlugin>;
template class THandlerSingleton<C3DFatImageCostPluginHandlerImpl>;

NS_MIA_END
