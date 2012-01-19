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
#include <mia/2d/cost.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/fatcost.hh>

using namespace std; 
using namespace boost; 
using namespace mia; 

NS_BEGIN(nmi_2dimage_fatcost)

class CFatNMI2DImageCost : public C2DImageFatCost {
public: 
	CFatNMI2DImageCost(P2DImage src, P2DImage ref, float weight);
private: 	
	virtual P2DImageFatCost cloned(P2DImage src, P2DImage ref, float weight) const; 
	virtual double do_value() const; 
	virtual double do_evaluate_force(C2DFVectorfield& force) const;
	C2DImageCostPlugin::ProductPtr m_evaluator; 
}; 


CFatNMI2DImageCost::CFatNMI2DImageCost(P2DImage src, P2DImage ref, float weight):
	C2DImageFatCost(src,  ref,  weight)
{
}

P2DImageFatCost CFatNMI2DImageCost::cloned(P2DImage src, P2DImage ref, float weight) const
{
	return P2DImageFatCost(new CFatNMI2DImageCost(src, ref,  weight)); 
}

double CFatNMI2DImageCost::do_value() const
{
	TRACE("CFatNMI2DImageCost::do_value"); 

	return 1000.0; 
}
double CFatNMI2DImageCost::do_evaluate_force(C2DFVectorfield& force) const
{
	TRACE("CFatNMI2DImageCost::do_evaluate_force"); 

	return value(); 
}

class C2DNMIFatImageCostPlugin: public C2DFatImageCostPlugin {
public: 
	C2DNMIFatImageCostPlugin(); 
private: 
	virtual C2DFatImageCostPlugin::ProductPtr do_create(P2DImage src, P2DImage ref, float weight)const;
	bool do_test() const; 
	const string do_get_descr()const; 
	
}; 

C2DNMIFatImageCostPlugin::C2DNMIFatImageCostPlugin():
	C2DFatImageCostPlugin("nmi")
{
	TRACE("C2DNMIFatImageCostPlugin::C2DNMIFatImageCostPlugin()"); 
}

C2DFatImageCostPlugin::ProductPtr C2DNMIFatImageCostPlugin::do_create(P2DImage src, P2DImage ref, float weight)const
{
	return C2DFatImageCostPlugin::ProductPtr(new CFatNMI2DImageCost(src, ref, weight)); 
}

bool C2DNMIFatImageCostPlugin::do_test() const
{
	return true; 
}

const string C2DNMIFatImageCostPlugin::do_get_descr()const
{
	return "Fat 2D NMI cost function";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNMIFatImageCostPlugin(); 
}

NS_END
