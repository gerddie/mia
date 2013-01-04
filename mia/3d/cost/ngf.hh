/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#ifndef mia_3d_cost_ngf_hh
#define mia_3d_cost_ngf_hh

#include <mia/3d/vectorfield.hh>
#include <mia/3d/cost.hh>
#include <mia/3d/matrix.hh>

NS_BEGIN(ngf_3dimage_cost)

class FEvaluator {
public:
	typedef double result_type; 
        virtual ~FEvaluator(){};
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const = 0;
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_range_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const = 0;

	double operator()(const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
protected:
	mia::C3DFMatrix get_gradient(mia::C3DFVectorfield::const_range_iterator& isrc, int nx, int nxy) const; 
};

class FScalar: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const;
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_range_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const;
};

class FCross: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
	
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_range_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const;
};

class FDeltaScalar: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_range_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const; 
};

typedef std::shared_ptr<FEvaluator > PEvaluator;


class C3DNFGImageCost : public mia::C3DImageCost {
public:
	C3DNFGImageCost(PEvaluator evaluator);
private:
	virtual double do_value(const mia::C3DImage& a, const mia::C3DImage& b) const ;
	virtual double do_evaluate_force(const mia::C3DImage& a, const mia::C3DImage& b, float scale, mia::C3DFVectorfield& force) const;

	virtual void post_set_reference(const mia::C3DImage& ref); 

	mia::C3DFVectorfield m_ng_ref;
	bool m_jump_levels_valid;
	float m_cost_baseline;

	PEvaluator m_evaluator;
	float m_intensity_scale;
};

class C3DNFGImageCostPlugin: public mia::C3DImageCostPlugin {
public:
	C3DNFGImageCostPlugin();
private:
	virtual mia::C3DImageCost *do_create()const;

	const std::string do_get_descr()const;
	std::string m_kernel;
};


NS_END

#endif
