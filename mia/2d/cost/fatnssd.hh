/* -*- mia-c++  -*-
 *
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
#include <mia/2d/cost.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/fatcost.hh>

NS_BEGIN(nssd_2dimage_fatcost)

class CFatNSSD2DImageCost : public mia::C2DImageFatCost {
public:
	CFatNSSD2DImageCost(mia::P2DImage src, mia::P2DImage ref,  mia::P2DInterpolatorFactory ipf, float weight);
private:
	virtual mia::P2DImageFatCost cloned(mia::P2DImage src, mia::P2DImage ref) const;
	virtual double do_value() const;
	virtual double do_evaluate_force(mia::C2DFVectorfield& force) const;
	mia::C2DImageCostPlugin::ProductPtr m_evaluator;
};

class C2DNSSDFatImageCostPlugin: public mia::C2DFatImageCostPlugin {
public:
	C2DNSSDFatImageCostPlugin();
private:
	virtual mia::C2DFatImageCostPlugin::ProductPtr do_create(mia::P2DImage src,
								 mia::P2DImage ref,  mia::P2DInterpolatorFactory ipf, float weight)const;
	bool  do_test() const;
	void prepare_path() const;

	const std::string do_get_descr()const;

};

NS_END
