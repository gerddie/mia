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

#include <cmath>
#include <limits>
#include <mia/core/spacial_kernel.hh>

NS_BEGIN(cdiff_1d_folding_kernel)

/**  Class for a one-dimensional central difference filter kernel. */
class C1DCDiffFilterKernel: public mia::C1DFoldingKernel {

public:
	/** contructor creates the kernel, is always -1, 0, 1 */
	C1DCDiffFilterKernel();
private:
	virtual std::vector<double> do_apply(const std::vector<double>& data) const;
};

class C1DSpacialCDiffKernelPlugin: public mia::C1DSpacialKernelPlugin {
public:
	C1DSpacialCDiffKernelPlugin();
	virtual mia::C1DFoldingKernel *do_create() const;
	virtual const std::string do_get_descr()const;
};

NS_END 
