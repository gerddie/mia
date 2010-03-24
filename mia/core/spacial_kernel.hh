/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef mia_core_spacial_kernel_hh
#define mia_core_spacial_kernel_hh


#include <vector>

// MIA specific
#include <mia/core/defines.hh>
#include <mia/core/factory.hh>

NS_MIA_BEGIN

struct EXPORT_CORE spacial_kernel_data {
	static const char *type_descr;
};


struct EXPORT_CORE kernel_plugin_type {
	static const char *value;
};

/**
   prototype for a 1D spacial convolution kernel
   \todo put the convolution into this class, problem: it must work for all
         data types but should also be virtual
*/

class EXPORT_CORE C1DFilterKernel : public CProductBase {
protected:


	/** constructs the kernel
	    \param fsize width parameter of the kernel
	*/
	C1DFilterKernel(int fsize);

	virtual ~C1DFilterKernel();
public:

	/// returns the filter width definition parameter
	int get_fsize() const;

	/// returns the width of the kernel
	size_t size() const;

	void apply_inplace(std::vector<double>& data) const;

	std::vector<double> apply(const std::vector<double>& data) const;

private:
	/// returns the width of the kernel
	virtual size_t do_size() const = 0;

	virtual std::vector<double> do_apply(const std::vector<double>& data) const = 0;

	int m_fsize;

};

/**
   Base class for folding kernal types. 
 */

class EXPORT_CORE C1DFoldingKernel : public C1DFilterKernel {
public:
	typedef std::vector<double> vec_mask;

	typedef vec_mask::const_iterator const_iterator;

	C1DFoldingKernel(int fsize);


	/// returns a constant iterator at the begin of the filter kernel
	const_iterator begin()const;

	/// returns a constant iterator at the end of the filter kernel
	const_iterator end()const;

	/// returns a constant iterator at the begin of the derivative of the filter kernel
	const_iterator dbegin()const;

	/// returns a constant iterator at the end of the derivative of the filter kernel
	const_iterator dend()const;



	double& operator[](int i) {
		return m_mask[i];
	}
	double operator[](int i)const {
		return m_mask[i];
	}
protected:
        typedef vec_mask::iterator iterator;

	iterator begin();

	/// returns an iterator at the end if the derivative of the  kernel
	iterator end();

	iterator dbegin();

	/// returns an iterator at the end if the derivative of the  kernel
	iterator dend();

private:
	/// returns the width of the kernel
	virtual size_t do_size() const;

	vec_mask m_mask;
	vec_mask m_derivative;

};

typedef TFactory<C1DFoldingKernel, spacial_kernel_data, kernel_plugin_type> C1DSpacialKernelPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C1DSpacialKernelPlugin> > C1DSpacialKernelPluginHandler;

NS_MIA_END

#endif
