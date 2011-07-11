/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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



CFatSSD3DImageCost::CFatSSD3DImageCost(P3DImage src, P3DImage ref, P3DInterpolatorFactory ipf, float weight):
	C3DImageFatCost(src,  ref,  ipf, weight),
	m_evaluator(C3DImageCostPluginHandler::instance().produce("ssd"))
{
	m_evaluator->set_reference(*ref); 
}

P3DImageFatCost CFatSSD3DImageCost::cloned(P3DImage src, P3DImage ref) const
{
	return P3DImageFatCost(new CFatSSD3DImageCost(src, ref,  get_ipf(), get_weight()));
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
	virtual C3DFatImageCostPlugin::ProductPtr do_create(P3DImage src,
							    P3DImage ref, P3DInterpolatorFactory ipf, float weight)const;
	bool  do_test() const;
	const string do_get_descr()const;

};

C3DSSDFatImageCostPlugin::C3DSSDFatImageCostPlugin():
	C3DFatImageCostPlugin("ssd")
{
	TRACE("C3DSSDFatImageCostPlugin::C3DSSDFatImageCostPlugin()");
}

C3DFatImageCostPlugin::ProductPtr C3DSSDFatImageCostPlugin::do_create(P3DImage src, P3DImage ref,
								      P3DInterpolatorFactory ipf, float weight)const
{
	return C3DFatImageCostPlugin::ProductPtr(new CFatSSD3DImageCost(src, ref, ipf, weight));
}

bool  C3DSSDFatImageCostPlugin::do_test() const
{
	C3DBounds size(8,16,7);
	vector<float> init_test(8 * 16 * 7, 1.0);
	vector<float> init_ref (8 * 16 * 7, 2.0);

	P3DImage test_image(new C3DFImage(size, &init_test[0]));
	P3DImage ref_image(new C3DFImage(size, &init_ref[0]));

	P3DInterpolatorFactory ipf(create_3dinterpolation_factory(ip_bspline3, bc_mirror_on_bounds));
	CFatSSD3DImageCost cost(test_image, ref_image, ipf, 1.0);
	double scale = 1.0; 

	if (scale * cost.value() > 8 * 16 * 7 * 1.001 * 0.5 || scale * cost.value() <  8 * 16 * 7 *0.999 * 0.5 ) {
		cvfail() << "C3DSSDFatImageCostPlugin\n";
		return false;
	}

	return true;
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
