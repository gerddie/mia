/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <numeric>
#include <boost/algorithm/minmax_element.hpp>

#include <mia/core/msgstream.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/nfg.hh>
#include <mia/2d/datafield.hh>
#include <mia/2d/imageio.hh>


NS_BEGIN(nfg_2dimage_cost)


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

class C2DNFGImageCost : public mia::C2DImageCost {
public:
	C2DNFGImageCost(PEvaluator evaluator);
	virtual void prepare_reference(const mia::C2DImage& ref)__attribute__((deprecated)); 
private:
	virtual double do_value(const mia::C2DImage& a, const mia::C2DImage& b) const ;
	virtual double do_evaluate_force(const mia::C2DImage& a, const mia::C2DImage& b, mia::C2DFVectorfield& force) const;

	virtual void post_set_reference(const mia::C2DImage& ref); 

	mia::C2DFVectorfield m_ng_ref;
	PEvaluator m_evaluator;

};

class C2DNFGImageCostPlugin: public mia::C2DImageCostPlugin {
public:
	enum ESubTypes {st_unknown, st_delta, st_delta_scalar, st_scalar, st_cross};

	C2DNFGImageCostPlugin();
private:

	virtual mia::C2DImageCost *do_create()const;

	const std::string do_get_descr()const;
	ESubTypes m_kernel;
};

NS_END
