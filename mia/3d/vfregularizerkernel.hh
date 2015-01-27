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


#ifndef mia_3d_vfregularizerkernel_hh
#define mia_3d_vfregularizerkernel_hh

#include <mia/core/factory.hh>
#include <mia/3d/vectorfield.hh>

NS_MIA_BEGIN

/**
   @ingroup registration 

   Kernel for iterative  3D vector field regularizer 

   This is the base class for a 3D vector field reguarlizer 
   kernels that is typically used in non-linear registration to 
   evaluate a velocity field from a force field by applying 
   some regularization model on the input data. 
*/



class EXPORT_3D C3DFVectorfieldRegularizerKernel : public CProductBase  {
public: 
        typedef C3DFVectorfieldRegularizerKernel plugin_type; 
        typedef C3DFVectorfieldRegularizerKernel plugin_data;

        static const char *type_descr;
        static const char *data_descr;

        typedef std::shared_ptr< C3DFVectorfieldRegularizerKernel > Pointer; 
        
        virtual ~C3DFVectorfieldRegularizerKernel(); 

        void set_data_fields(C3DFVectorfield  *output, C3DFVectorfield *input, T3DDatafield<float> *residua);
        
        void set_update_fields(const T3DDatafield<unsigned char> *update_flags, T3DDatafield<unsigned char> *set_flags); 
        
        float evaluate_row(unsigned y, unsigned z); 

        float evaluate_row_sparse(unsigned y, unsigned z); 
 protected: 
        C3DFVectorfield& get_output_field() const; 
        const C3DFVectorfield& get_input_field() const; 
        T3DDatafield<float>& get_residua() const;  
        const T3DDatafield<unsigned char>& get_update_flags() const; 
        T3DDatafield<unsigned char>& get_set_flags() const; 
 private: 
        virtual float do_evaluate_row(unsigned y, unsigned z) = 0; 

        virtual float do_evaluate_row_sparse(unsigned y, unsigned z) = 0; 


        C3DFVectorfield *m_output; 
        C3DFVectorfield *m_input; 
        T3DDatafield<float> *m_residua; 
        const T3DDatafield<unsigned char> *m_update_flags; 
        T3DDatafield<unsigned char> *m_set_flags; 
}; 


typedef C3DFVectorfieldRegularizerKernel::Pointer P3DVectorfieldRegularizerKernel; 

typedef TFactory<C3DFVectorfieldRegularizerKernel> C3DFVectorfieldRegularizerKernelPlugin; 

typedef THandlerSingleton<TFactoryPluginHandler<C3DFVectorfieldRegularizerKernelPlugin> > C3DFVectorfieldRegularizerKernelPluginHandler;


template <> const char *  const TPluginHandler<C3DFVectorfieldRegularizerKernelPlugin>::m_help; 
extern template class EXPORT_3D  TFactory<C3DFVectorfieldRegularizerKernel>; 
extern template class EXPORT_3D  TFactoryPluginHandler<C3DFVectorfieldRegularizerKernelPlugin>; 
extern template class EXPORT_3D  THandlerSingleton<TFactoryPluginHandler<C3DFVectorfieldRegularizerKernelPlugin> >; 



NS_MIA_END
#endif 
