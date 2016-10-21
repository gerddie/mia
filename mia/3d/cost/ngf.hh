/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_3d_cost_ngf_hh
#define mia_3d_cost_ngf_hh

#include <mia/3d/vectorfield.hh>
#include <mia/3d/cost.hh>
#include <mia/3d/matrix.hh>

NS_BEGIN(ngf_3dimage_cost)

class FEvaluator {
public:
	typedef mia::C3DFVectorfield::const_range_iterator_with_boundary_flag field_range_iterator; 
	typedef double result_type; 
        virtual ~FEvaluator(){};
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const = 0;
	virtual mia::C3DFVector grad(int nx, int nxy, field_range_iterator& isrc,
				     const mia::C3DFVector& ref, double& cost) const = 0;

	double operator()(const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
protected:
	mia::C3DFMatrix get_gradient(field_range_iterator& isrc, int nx, int nxy) const; 
};

class FScalar: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const;
	virtual mia::C3DFVector grad(int nx, int nxy, field_range_iterator& isrc,
				     const mia::C3DFVector& ref, double& cost) const;
};

class FCross: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
	
	virtual mia::C3DFVector grad(int nx, int nxy, field_range_iterator& isrc,
				     const mia::C3DFVector& ref, double& cost) const;
};

class FDeltaScalar: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
	virtual mia::C3DFVector grad(int nx, int nxy, field_range_iterator& isrc,
				     const mia::C3DFVector& ref, double& cost) const; 
};

typedef std::shared_ptr<FEvaluator > PEvaluator;


class C3DNFGImageCost : public mia::C3DImageCost {
public:
	C3DNFGImageCost(PEvaluator evaluator);
private:
	virtual double do_value(const mia::C3DImage& a, const mia::C3DImage& b) const ;
	virtual double do_evaluate_force(const mia::C3DImage& a, const mia::C3DImage& b, mia::C3DFVectorfield& force) const;

	virtual void post_set_reference(const mia::C3DImage& ref); 

	mia::C3DFVectorfield m_ng_ref;
	PEvaluator m_evaluator;
};

class C3DNFGImageCostPlugin: public mia::C3DImageCostPlugin {
public:
	enum ESubTypes {st_unknown, st_delta_scalar, st_scalar, st_cross};
	C3DNFGImageCostPlugin();
private:
	virtual mia::C3DImageCost *do_create()const;

	const std::string do_get_descr()const;
	ESubTypes m_kernel;
};


NS_END

#endif
