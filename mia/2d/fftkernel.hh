/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

// $Id: fftkernel.hh,v 1.1 2006-03-01 19:14:13 wollny Exp $

/*! \brief basic type of a plugin handler

A basis for fft filter plugins

\author Gert Wollny <wollny at die.upm.es>

*/

#ifndef mia_2d_fftkernel_hh
#define mia_2d_fftkernel_hh

#include <complex>
#include <fftw3.h>

#include <mia/core/defines.hh>
#include <mia/core/factory.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/defines2d.hh>
#include <mia/2d/2DVector.hh>

NS_MIA_BEGIN

struct EXPORT_2D fft2d_kernel_data {
	static const char *type_descr;
};

/**
   Base class for 2D FFT filters. Filter applications are applied like this: 
   first a real-to-complex transformation is run, then run the filter on 
   the half-complex transform and run the back-transform.  
*/
class EXPORT_2D CFFT2DKernel :public CProductBase {
public:
	typedef fft2d_kernel_data plugin_data; 
	typedef kernel_plugin_type plugin_type; 
		
	CFFT2DKernel();

	virtual ~CFFT2DKernel();

	/**
	   Run the FFT by first calling the forward plan, then 
	   call do_apply() and then running the backward plan. 
	   Normalization is left to the caller,  
	 */
	void apply() const;
	

	/**
	   Prepare the FFT structures and return the buffer where the data needs to be 
	   put when it should be processed. 
	 */
	float *prepare(const C2DBounds& size);
	
private:
	/*
	  free all the FFT structures 
	 */
	void tear_down();
	
	virtual void do_apply(const C2DBounds& m_size, size_t m_realsize_x, 
			      fftwf_complex *m_cbuffer) const = 0;

	C2DBounds m_size;
	fftwf_complex *m_cbuffer;
	float   *m_fbuffer;
	float m_scale;
	fftwf_plan m_forward_plan;
	fftwf_plan m_backward_plan;

	size_t m_realsize_x;
};

typedef  std::shared_ptr<CFFT2DKernel > PFFT2DKernel;
typedef TFactory<CFFT2DKernel> CFFT2DKernelPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<CFFT2DKernelPlugin> > CFFT2DKernelPluginHandler;

NS_MIA_END

#endif

