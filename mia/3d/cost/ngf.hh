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


#ifndef mia_3d_cost_ngf_hh
#define mia_3d_cost_ngf_hh

#include <mia/3d/3DVectorfield.hh>

NS_BEGIN(ngf_3dimage_cost)

class FEvaluator {
public:
        virtual ~FEvaluator(){};
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const = 0;
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const = 0;
};

class FScalar: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const;
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const;
};

class FCross: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
	
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const;
};

class FDeltaScalar: public FEvaluator {
public:
	virtual double cost (const mia::C3DFVector& src, const mia::C3DFVector& ref) const; 
	virtual mia::C3DFVector grad(int nx, int nxy, mia::C3DFVectorfield::const_iterator isrc,
				     const mia::C3DFVector& ref, double& cost) const; 
private:
	double get_dot(const mia::C3DFVector& src, const mia::C3DFVector& ref)const; 
};

typedef std::shared_ptr<FEvaluator > PEvaluator;

NS_END

#endif
