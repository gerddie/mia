/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/core/spacialkernel/gauss.hh>

NS_MIA_USE
using namespace std;
using namespace gauss_1d_folding_kernel;

C1DGaussFilterKernel::C1DGaussFilterKernel(int fsize):
	C1DFoldingKernel(fsize)
{
	size_t n = size();
        assert(n > 0);

        vec_mask tmp(n);

        (*this)[0] = 1.0;
        if (n < 2)
                return;

        (*this)[1] = 1.0;

	for (size_t i = 2; i < n; i++){
		(*this)[i] = 1.0;
                transform(begin(), begin() + i, begin() + 1, tmp.begin() + 1, 
			  [](double a, double b){return a + b;}); 
                copy(tmp.begin() + 1, tmp.begin()+i, begin() +1);
	}

	const double norm = pow(2.0,n-1);
	transform(begin(), end(), begin(), [norm](double x){return x / norm;});

	vec_mask::iterator ider    = dbegin();
	vec_mask::iterator ikern   = begin();
	for (size_t i = 0; i < n; ++i, ++ider, ++ikern) {
		*ider = 2.0 * (fsize - i) * *ikern;
	}
}

C1DSpacialGaussKernelPlugin::C1DSpacialGaussKernelPlugin():
	C1DSpacialKernelPlugin("gauss"),
	m_w(1)
{
	add_parameter("w", new CIntParameter(m_w, 0, numeric_limits<int>::max(),
					     false, "half filter width"));
}

C1DFoldingKernel *C1DSpacialGaussKernelPlugin::do_create() const
{
	if (m_w > 0)
		return new C1DGaussFilterKernel(m_w);
	else
		return NULL;
}

std::vector<double> C1DGaussFilterKernel::do_apply(const std::vector<double>& data) const
{
	typedef std::vector<double> vec_t;
	vec_t result(data.size(),0.0);

	C1DFoldingKernel::const_iterator  ek = end();

	vec_t::iterator  trgt = result.begin();

	int pos = data.size();

	// filter at left boundary
	for (int i = 0; i < get_fsize() && pos >= 0 ; ++i, --pos, ++trgt) {
		double ht = 0.0;
		C1DFoldingKernel::const_iterator  ik = begin();
		vec_t::const_iterator src = data.begin();

		for (int k = get_fsize() - i; k > 0;  --k, ++ik) {
			ht +=  src[k-1] * *ik;
		}

		while (ik != ek && src != data.end()){
			ht += *src++ * *ik++;
		}
		*trgt = ht;
	}

	// filter inside
	vec_t::const_iterator start = data.begin();
	while (pos-- > get_fsize()) {

		C1DFoldingKernel::const_iterator  ik = begin();

		vec_t::const_iterator help = start;

		double ht = 0.0;
		while (ik != ek){
			ht += *ik++ * *help++;
		}
		*trgt = ht;

		++trgt;
		++start;
	}

	// filter at right boundary
	for (int i = 0; i < get_fsize() && pos >= 0 && trgt != result.end() &&
		     start != data.end();   ++i, --pos, ++trgt, ++start) {
		
		C1DFoldingKernel::const_iterator  ik = begin();
		double  ht = 0.0;

		vec_t::const_iterator src = start;
		vec_t::const_reverse_iterator rsrc = data.rbegin();
		while (src != data.end() ){
			ht += *src++ * *ik++;
		}
		while (ik != end()) {
			ht += *rsrc++ * *ik++;
		}

		*trgt = ht;
	}
	return result;
}

const string C1DSpacialGaussKernelPlugin::do_get_descr()const
{
	return "spacial Gauss filter kernel";
}

extern "C" EXPORT CPluginBase  *get_plugin_interface()
{
	return new C1DSpacialGaussKernelPlugin();
}

