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

#include <numeric>
#include <boost/algorithm/minmax_element.hpp>

#include <mia/core/msgstream.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fatcost.hh>
#include <mia/2d/nfg.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/2d/2dimageio.hh>


NS_BEGIN(nfg_2dimage_fatcost)

class CCostEvaluator {
public:
	struct param_pass {
		mia::C2DFVectorfield::const_iterator src;
		mia::C2DFVectorfield::const_iterator ref;
		mia::C2DFVectorfield::const_iterator srcp;
		mia::C2DFVectorfield::const_iterator srcm;
	};

	virtual const  char *do_get_name() const = 0;
	virtual float get_cost(size_t x,param_pass& p) const = 0;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g) const = 0;
};


class CCostEvaluatorCross: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;
	virtual const  char *do_get_name() const;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const;
};

class CCostEvaluatorScalar: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;
	virtual const  char *do_get_name() const ;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g) const ;
};


class CCostEvaluatorSQDelta: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;

	virtual const  char *do_get_name() const;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const;
private:
	static const float scale;
	float get_help_value(const mia::C2DFVector& ref, float dot_rs) const;
};

class CCostEvaluatorDeltaScalar: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;

	virtual const  char *do_get_name() const;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const;
private:
	static const float scale;
};


typedef std::shared_ptr<CCostEvaluator > PEvaluator;

class CFatNFG2DImageCost : public mia::C2DImageFatCost {
public:
	CFatNFG2DImageCost(mia::P2DImage src, mia::P2DImage ref, float weight, PEvaluator evaluator);
private:
	virtual mia::P2DImageFatCost cloned(mia::P2DImage src, mia::P2DImage ref) const;
	virtual double do_value() const;
	virtual double do_evaluate_force(mia::C2DFVectorfield& force) const;
	void prepare() const;
	mutable mia::C2DFVectorfield m_ng_ref;
	mutable bool m_jump_levels_valid;
	mutable float m_cost_baseline;

	PEvaluator m_evaluator;
	float m_intensity_scale;
};

class C2DNFGFatImageCostPlugin: public mia::C2DFatImageCostPlugin {
public:
	C2DNFGFatImageCostPlugin();
private:
	mia::C2DImageFatCost *do_create(mia::P2DImage src, mia::P2DImage ref, 
					mia::P2DInterpolatorFactory ipf, float weight)const;

	const std::string do_get_descr()const;
	std::string m_type;
};

NS_END
