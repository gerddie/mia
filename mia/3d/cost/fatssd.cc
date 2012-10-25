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

#include <mia/core/msgstream.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/cost/fatssd.hh>

using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;
using namespace mia;


NS_BEGIN(ssd_3dimage_fatcost)



CFatSSD3DImageCost::CFatSSD3DImageCost(P3DImage src, P3DImage ref, float weight):
	C3DImageFatCost(src,  ref,  weight),
	m_evaluator(C3DImageCostPluginHandler::instance().produce("ssd"))
{
	m_evaluator->set_reference(*ref); 
}

P3DImageFatCost CFatSSD3DImageCost::cloned(P3DImage src, P3DImage ref) const
{
	return P3DImageFatCost(new CFatSSD3DImageCost(src, ref,  get_weight()));
}

double CFatSSD3DImageCost::do_value() const
{
	TRACE("CFatSSD3DImageCost::do_value");
	
	return m_evaluator->value(get_floating());
}
double CFatSSD3DImageCost::do_evaluate_force(C3DFVectorfield& force) const
{
	TRACE("CFatSSD3DImageCost::do_evaluate_force");
	m_evaluator->evaluate_force(get_floating(), get_weight(), force);
	return value();
}

class C3DSSDFatImageCostPlugin: public C3DFatImageCostPlugin {
public:
	C3DSSDFatImageCostPlugin();
private:
	virtual C3DImageFatCost *do_create(P3DImage src, P3DImage ref, 
					   P3DInterpolatorFactory ipf, float weight)const;
	const string do_get_descr()const;

};

C3DSSDFatImageCostPlugin::C3DSSDFatImageCostPlugin():
	C3DFatImageCostPlugin("ssd")
{
	TRACE("C3DSSDFatImageCostPlugin::C3DSSDFatImageCostPlugin()");
}

C3DImageFatCost *C3DSSDFatImageCostPlugin::do_create(P3DImage src, P3DImage ref,
						     P3DInterpolatorFactory /*ipf*/, float weight)const
{
	return new CFatSSD3DImageCost(src, ref, weight);
}

const string C3DSSDFatImageCostPlugin::do_get_descr()const
{
	return "Fat 3D ssd cost function";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DSSDFatImageCostPlugin();
}

NS_END
