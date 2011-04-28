/*  -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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

/* 
   LatexBeginPlugin{Spacial filter kernels}
   
   \subsection{Gauss}
   \label{skernel:gauss}
   
   \begin{description}
   
   \item [Plugin:] gauss
   \item [Description:] The spacial Gauss kernel 
   
   \plugtabstart
   w &  int & half kernel with, the full kernel with is 2*w+1 & 1  \\
   \plugtabend
   
   \end{description}

   LatexEnd  
 */


#include <mia/core/spacialkernel/gauss.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::lambda;
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
                transform(begin(), begin() + i, begin() + 1, tmp.begin() + 1, _1 + _2);
                copy(tmp.begin() + 1, tmp.begin()+i, begin() +1);
	}

	const double norm = pow(2.0,n-1);
	transform(begin(), end(), begin(), _1 / norm);

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

C1DSpacialGaussKernelPlugin::ProductPtr C1DSpacialGaussKernelPlugin::do_create() const
{
	if (m_w > 0)
		return C1DSpacialGaussKernelPlugin::ProductPtr(new C1DGaussFilterKernel(m_w));
	else
		return C1DSpacialGaussKernelPlugin::ProductPtr();
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
	for (int i = 0; i < get_fsize() && pos >= 0 && trgt != data.end(); ++i, --pos, ++trgt) {

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

		++start;
	}
	return result;
}

bool C1DSpacialGaussKernelPlugin::do_test() const
{
	C1DGaussFilterKernel kernel(1);
	bool success = kernel.size() == 3;
	success &= (kernel[0] == kernel[2]);
	success &= (kernel[1] == .5);
	success &= (kernel[2] == 0.25);

	const double input[5] = {0,0,1,0,0 };
	const double input2[5] = {1,1,1,1,1 };
	const double test_out_1[5] = {0,0.25,0.5,0.25,0 };
	const double test_out_2[5] = {0.0625,0.25,0.375,0.25,0.0625 };
	const double test_out_3[5] = {1,1,1,1,1};
	const double test_out_4[5] = {0.109375, 0.234375, 0.3125, 0.234375, 0.109375};

	vector<double> in1(5);
	copy(input, input+5, in1.begin());
	kernel.apply_inplace(in1);
	success &= (equal(in1.begin(), in1.end(), test_out_1));

        C1DGaussFilterKernel kernel2(2);
        success &= (kernel2.size() == 5);
	success &= (kernel2[0] == kernel2[4]);
	success &= (kernel2[1] == kernel2[3]);
	success &= (kernel2[0] == .0625);
        success &= (kernel2[1] == 0.25);
	success &= (kernel2[2] == 0.375);

	copy(input, input+5, in1.begin());
	kernel2.apply_inplace(in1);
	success &= (equal(in1.begin(), in1.end(), test_out_2, equal_double()));

	if (!success)  {
		for (size_t i = 0; i < 5; ++i)
			cvfail() << in1[i] << " vs " << test_out_2[i] << "\n";
	}

	copy(input2, input2+5, in1.begin());
	kernel2.apply_inplace(in1);
	success &= (equal(in1.begin(), in1.end(), test_out_3, equal_double()));

	if (!success)  {
		for (size_t i = 0; i < 5; ++i)
			cvfail() << in1[i] << " vs " << test_out_3[i] << "\n";
	}

	// add a test where the kernel is bigger then the input data
    C1DGaussFilterKernel kernel3(3);
	success &= (kernel3.size() == 7);
	success &= (kernel3[0] == kernel3[6]);
	success &= (kernel3[1] == kernel3[5]);
	success &= (kernel3[2] == kernel3[4]);
	success &= (kernel3[0] == 0.015625);
        success &= (kernel3[1] == 0.09375);
	success &= (kernel3[2] == 0.234375);
	success &= (kernel3[3] == 0.3125);

	copy(input, input+5, in1.begin());
	kernel3.apply_inplace(in1);
	success &= (equal(in1.begin(), in1.end(), test_out_4, equal_double()));

	if (!success)  {
		for (size_t i = 0; i < 5; ++i)
			cvfail() << in1[i] << " vs " << test_out_4[i] << "\n";
	}

	return success;
}

const string C1DSpacialGaussKernelPlugin::do_get_descr()const
{
	return "spacial Gauss filter kernel";
}

extern "C" EXPORT CPluginBase  *get_plugin_interface()
{
	return new C1DSpacialGaussKernelPlugin();
}

