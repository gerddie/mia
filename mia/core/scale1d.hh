/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#ifndef mia_core_scale1d_hh
#define mia_core_scale1d_hh

#include <cassert>
#include <vector>
#include <memory>

#include <mia/core/msgstream.hh>
#include <mia/core/interpolator1d.hh>
#include <mia/core/spacial_kernel.hh>

NS_MIA_BEGIN

/**
   \ingroup interpol 
   \brief Class for scaling of one-dimensional arrays. 
   
   Class for scaling of one-dimensional arrays. For downscaling always a gaussian smoothing is used.
   for upscaling, an interpolator is created by using the provided interpolator factory.
   \remark obsolete use C1DScalerFixed instead 
*/

class EXPORT_CORE C1DScalar {
public:
	/**
	    Create the scaler prividing the given interpolator factory.
	    \param ipf
	 */

	C1DScalar(P1DInterpolatorFactory ipf) __attribute__((deprecated));


	/**
	   Scaling operator.
	   \param input input data
	   \param[out] output when calling the function, the size of this vector must be set to the requested
	   size. The path for down or upscaling is automatically selected.
	 */
	template <typename T>
	void operator () (const std::vector<T>& input, std::vector<T>& output) const;
private:

	template <class T, class HoldType>
	void  downscale__(const std::vector<T>& input, std::vector<T>& output) const;

	template <class T>
	void  downscale(const std::vector<T>& input, std::vector<T>& output) const;

	template <typename T>
	void  upscale(const std::vector<T>& input, std::vector<T>& output) const;

	const mia::C1DFoldingKernel& get_downscale_kernel(int fwidth) const;

	P1DInterpolatorFactory m_ipf;
	mutable int m_fwidth;
	mutable C1DSpacialKernelPlugin::SharedProduct m_kernel;
};

template <typename T>
void C1DScalar::operator () (const std::vector<T>& input, std::vector<T>& output) const
{
	if (input.size() == output.size()) {
		copy(input.begin(), input.end(), output.begin());
		return;
	}

	if (input.size() > output.size())
		return downscale(input, output);
	else
		return upscale(input, output);
}

template <class T, class HoldType>
void  C1DScalar::downscale__(const std::vector<T>& input, std::vector<T>& output) const
{
	double step = double(input.size() + 1) / (output.size() + 1);
	int fwidth= int (step + 0.5) >> 1;

	const C1DFoldingKernel& kernel = get_downscale_kernel(fwidth);
	for(size_t i = 0; i < output.size(); ++i) {
		HoldType value = HoldType();

		C1DFoldingKernel::const_iterator k = kernel.begin();
		for(size_t j = (size_t)i * step; j < input.size() && k != kernel.end();
		    ++j, ++k) {
			const HoldType h(input[j]);
			value += h * *k;
		}
		output[i] = T(value);
	}
}

template <class T>
void  C1DScalar::downscale(const std::vector<T>& input, std::vector<T>& output) const
{
	downscale__<T, typename max_hold_type<T>::type >(input, output);
}

template <typename T>
void C1DScalar::upscale(const std::vector<T>& input, std::vector<T>& output) const
{
	const size_t outsize = output.size();
	double step = double(input.size() - 1) / (outsize - 1);

	double x = 0.0;
	std::auto_ptr< T1DInterpolator<T> >  data( m_ipf->create(input));

        for (size_t i = 0; i < outsize; ++i, x += step) {
		output[i] = (*data)(x);
	}
}

NS_MIA_END


#endif
