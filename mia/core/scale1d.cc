/* -*- mia-c++  -*-
 *
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

#include <cassert>

#include <mia/core/scale1d.hh>

NS_MIA_BEGIN

C1DScalar::C1DScalar(P1DInterpolatorFactory ipf):
	m_ipf(ipf),
	m_fwidth(-1)
{
}

const C1DFoldingKernel& C1DScalar::get_downscale_kernel(int fwidth) const
{
	if (fwidth != m_fwidth) {
		m_fwidth = fwidth;
		stringstream gf_descr;
		gf_descr << "gauss:w=" << fwidth;
		m_kernel = C1DSpacialKernelPluginHandler::instance().produce(gf_descr.str().c_str());
	}
	assert(m_kernel);
	return dynamic_cast<const C1DFoldingKernel&>(*m_kernel);
}

NS_MIA_END
