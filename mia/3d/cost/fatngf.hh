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


#include <mia/3d/cost.hh>
#include <mia/3d/fatcost.hh>


NS_BEGIN(nfg_3dimage_fatcost)

class FEvaluator {
public:
        virtual ~FEvaluator(){};
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const = 0;
	virtual mia::C3DFVector  grad (int nx, int nxy, mia::C3DFVectorfield::const_iterator isrc,
			     const mia::C3DFVector& ref, double& cost) const = 0;
};

class FDeltaScalar: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
	virtual mia::C3DFVector  grad (int nx, int nxy, mia::C3DFVectorfield::const_iterator isrc,
				  const mia::C3DFVector& ref, double& cost) const; 
private:
	double get_dot(const mia::C3DFVector& src, const mia::C3DFVector& ref)const;
};


typedef std::shared_ptr<FEvaluator > PEvaluator;

class CFatNFG3DImageCost : public mia::C3DImageFatCost {
public:
	CFatNFG3DImageCost(mia::P3DImage src, mia::P3DImage ref, float weight, PEvaluator evaluator);
private:
	virtual mia::P3DImageFatCost cloned(mia::P3DImage src, mia::P3DImage ref) const;
	virtual double do_value() const;
	virtual double do_evaluate_force(mia::C3DFVectorfield& force) const;
	void prepare() const;
	mutable mia::C3DFVectorfield m_ng_ref;
	mutable bool m_jump_levels_valid;
	mutable float m_cost_baseline;

	PEvaluator m_evaluator;
	float m_intensity_scale;
};

NS_END
