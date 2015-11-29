/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/spacialkernel/cdiff.hh>

NS_MIA_USE
using namespace std;
using namespace cdiff_1d_folding_kernel;

C1DCDiffFilterKernel::C1DCDiffFilterKernel():
	C1DFoldingKernel(1)
{
	(*this)[0] = -1;
	(*this)[2] = 1; 
}

C1DSpacialCDiffKernelPlugin::C1DSpacialCDiffKernelPlugin():
	C1DSpacialKernelPlugin("cdiff")
{
}

C1DFoldingKernel *C1DSpacialCDiffKernelPlugin::do_create() const
{
		return new C1DCDiffFilterKernel();
}

std::vector<double> C1DCDiffFilterKernel::do_apply(const std::vector<double>& data) const
{
	std::vector<double> result(data.size(),0.0);
	transform(data.begin() + 2, data.end(),
		  data.begin(), 
		  result.begin() + 1,
		  [](double xp, double xm){return 0.5 * (xp - xm);}); 

	return result;
}

const string C1DSpacialCDiffKernelPlugin::do_get_descr()const
{
	return "Central difference filter kernel, mirror boundary conditions are used.";
}

extern "C" EXPORT CPluginBase  *get_plugin_interface()
{
	return new C1DSpacialCDiffKernelPlugin();
}

