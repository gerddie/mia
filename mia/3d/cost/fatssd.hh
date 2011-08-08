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


#include <mia/3d/cost.hh>
#include <mia/3d/fatcost.hh>

NS_BEGIN(ssd_3dimage_fatcost)

class CFatSSD3DImageCost : public mia::C3DImageFatCost {
public:
	CFatSSD3DImageCost(mia::P3DImage src, mia::P3DImage ref, float weight);
private:
	virtual mia::P3DImageFatCost cloned(mia::P3DImage src, mia::P3DImage ref) const;
	virtual double do_value() const;
	virtual double do_evaluate_force(mia::C3DFVectorfield& force) const;
	mia::P3DImageCost m_evaluator;
};

NS_END 
