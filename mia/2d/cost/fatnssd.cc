/* -*- mia-c++  -*-
 * Copyright (c) 2007 Gert Wollny 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/internal/normalize.hh>
#include <mia/2d/cost/fatnssd.hh>

using namespace std; 
using namespace boost; 
namespace bfs=::boost::filesystem; 
using namespace mia; 

NS_BEGIN(nssd_2dimage_fatcost)

CFatNSSD2DImageCost::CFatNSSD2DImageCost(P2DImage src, P2DImage ref,  P2DInterpolatorFactory ipf, float weight):
C2DImageFatCost(normalize(*src), normalize(*ref), ipf, weight), 
	_M_evaluator(C2DImageCostPluginHandler::instance().produce("ssd")) 
{
}

P2DImageFatCost CFatNSSD2DImageCost::cloned(P2DImage src, P2DImage ref) const
{
	return P2DImageFatCost(new CFatNSSD2DImageCost(src, ref,  get_ipf(), get_weight())); 
}

double CFatNSSD2DImageCost::do_value() const
{
	TRACE("CFatNSSD2DImageCost::do_value"); 

	return _M_evaluator->value(get_floating(),get_ref());
}
double CFatNSSD2DImageCost::do_evaluate_force(C2DFVectorfield& force) const
{
	TRACE("CFatNSSD2DImageCost::do_evaluate_force"); 
	_M_evaluator->evaluate_force(get_floating(), get_ref(), get_weight(), force);
	return value(); 
}

C2DNSSDFatImageCostPlugin::C2DNSSDFatImageCostPlugin():
	C2DFatImageCostPlugin("nssd")
{
	TRACE("C2DNSSDFatImageCostPlugin::C2DNSSDFatImageCostPlugin()"); 
}

C2DFatImageCostPlugin::ProductPtr C2DNSSDFatImageCostPlugin::do_create(P2DImage src, P2DImage ref,  
								       P2DInterpolatorFactory ipf, float weight)const
{
	// convert to float and scale 


	return C2DFatImageCostPlugin::ProductPtr(new CFatNSSD2DImageCost(src, ref, ipf, weight)); 
}

bool  C2DNSSDFatImageCostPlugin::do_test() const
{
	return true; 
}

void C2DNSSDFatImageCostPlugin::prepare_path() const
{
	TRACE("C2DNNSSDFatImageCostPlugin::prepare_path"); 
	list< bfs::path> costsearchpath; 
	costsearchpath.push_back( bfs::path(".")); 
	C2DImageCostPluginHandler::set_search_path(costsearchpath);

	cvdebug() << "C2DImageCostPluginHandler::instance().size(): " << 
		C2DImageCostPluginHandler::instance().size()<<"\n" ; 
}

const string C2DNSSDFatImageCostPlugin::do_get_descr()const
{
	return "Normalized 2D nssd cost function";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNSSDFatImageCostPlugin(); 
}

NS_END
