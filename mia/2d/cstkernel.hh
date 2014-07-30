/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_2d_cstkernel_hh
#define mia_2d_cstkernel_hh

#include <complex>
#include <fftw3.h>

#include <mia/core/defines.hh>
#include <mia/core/factory.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/cstplan.hh>
#include <mia/2d/datafield.hh>
#include <mia/2d/image.hh>

NS_MIA_BEGIN

/// plugin-helper structure 
struct EXPORT_2D cst2d_kernel {
	/// plugin path helper value 
	static const char *type_descr;
};

/**
   @ingroup filtering
   @brief Base class for cos/sin transformation filters 

   This class defines the interface for filter that uses cosinus and sinus transforms to move 
   the data into a dual space, runs some filter there and then transforms the data back. 
   This class may be used for gray scale image data and vector data. 
   
   @tparam The actual data type to run the filter on 
   @remark why is this called a kernel? 
*/

template <typename T>
class EXPORT_2D TCST2DKernel :public CProductBase {
public:
	/// some helper typedef for the plug-in handler 
	typedef typename plugin_data_type<T>::type plugin_data; 

	/// define the plugin-type helper to get the search path 
	typedef cst2d_kernel plugin_type; 

	/// define the type of the FFTW plan used 
	typedef TCSTPlan<T> CPlan;
	
	/**
	   Consruct the filter as either forward or backward 
	   @remark why is this? 
	 */
	TCST2DKernel(fftwf_r2r_kind forward);
	
	virtual ~TCST2DKernel();

	/**
	   Apply the transform 
	   @param[in] in 
	   @param[out] out 
	 */
	void apply(const T& in, T& out) const;

	/**
	   Prepare the transform based on the size of the data field to be transformed 
	   @param size 
	 */
	void prepare(const C2DBounds& size);

private:
        virtual CPlan *do_prepare(fftwf_r2r_kind fw_kind, const std::vector<int>& size) = 0;

	fftwf_r2r_kind m_forward;
	std::unique_ptr<CPlan> m_plan;
};

/**
  \cond NEEDS_REHAUL 
*/

extern template class EXPORT_2D TCST2DKernel<C2DFVectorfield>;
extern template class EXPORT_2D TCST2DKernel<C2DFImage>;


typedef TCST2DKernel<C2DFVectorfield> CCST2DVectorKernel;
typedef TCST2DKernel<C2DFImage>       CCST2DImageKernel;

typedef  std::shared_ptr<CCST2DImageKernel > PCST2DImageKernel;
typedef  std::shared_ptr<CCST2DVectorKernel > PCST2DVectorKernel;

extern template class EXPORT_2D TFactory<CCST2DVectorKernel>; 
typedef TFactory<CCST2DVectorKernel> CCST2DVectorKernelPlugin;

extern template class EXPORT_2D TPluginHandler<CCST2DVectorKernelPlugin>; 
extern template class EXPORT_2D TFactoryPluginHandler<CCST2DVectorKernelPlugin>; 
extern template class EXPORT_2D THandlerSingleton<TFactoryPluginHandler<CCST2DVectorKernelPlugin> >; 
typedef THandlerSingleton<TFactoryPluginHandler<CCST2DVectorKernelPlugin> > CCST2DVectorKernelPluginHandler;


extern template class EXPORT_2D TFactory<CCST2DImageKernel>; 
typedef TFactory<CCST2DImageKernel> CCST2DImgKernelPlugin;

extern template class EXPORT_2D TPluginHandler<CCST2DImgKernelPlugin>; 
extern template class EXPORT_2D TFactoryPluginHandler<CCST2DImgKernelPlugin>; 
extern template class EXPORT_2D THandlerSingleton<TFactoryPluginHandler<CCST2DImgKernelPlugin> >; 
typedef THandlerSingleton<TFactoryPluginHandler<CCST2DImgKernelPlugin> > CCST2DImgKernelPluginHandler;
/// \endcond 

NS_MIA_END

#endif

