/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <cmath>
#include <limits>
#include <mia/core/spacial_kernel.hh>

NS_BEGIN(gauss_1d_folding_kernel)

/**  Class for a one-dimensional Gaussian convolution filter kernel. */
class C1DGaussFilterKernel: public mia::C1DFoldingKernel {

public:
	/** contructor creates a kernel with the width = 2 * fsize +1
	    \param fsize is the filter with parameter */
	C1DGaussFilterKernel(unsigned fsize);
private:
	virtual std::vector<double> do_apply(const std::vector<double>& data) const;
};

class C1DSpacialGaussKernelPlugin: public mia::C1DSpacialKernelPlugin {
public:
	C1DSpacialGaussKernelPlugin();
	virtual mia::C1DFoldingKernel *do_create() const;
	virtual const std::string do_get_descr()const;
private:
	unsigned  m_w;
};

struct equal_double {
	bool operator () (double a , double b) {
		return ::fabs(a-b) < 0.0001;
	}
};

NS_END

