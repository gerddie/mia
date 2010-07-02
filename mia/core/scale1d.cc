/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <cassert>

#include <mia/core/scale1d.hh>

NS_MIA_BEGIN

C1DScalar::C1DScalar(P1DInterpolatorFactory ipf):
	_M_ipf(ipf),
	_M_fwidth(-1)
{
}

const C1DFoldingKernel& C1DScalar::get_downscale_kernel(int fwidth) const
{
	if (fwidth != _M_fwidth) {
		_M_fwidth = fwidth;
		stringstream gf_descr;
		gf_descr << "gauss:w=" << fwidth;
		_M_kernel = C1DSpacialKernelPluginHandler::instance().produce(gf_descr.str().c_str());
	}
	assert(_M_kernel);
	return dynamic_cast<const C1DFoldingKernel&>(*_M_kernel);
}

NS_MIA_END
