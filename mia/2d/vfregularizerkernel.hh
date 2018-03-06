/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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


#ifndef mia_2d_vfregularizerkernel_hh
#define mia_2d_vfregularizerkernel_hh

#include <mia/core/factory.hh>
#include <mia/2d/vectorfield.hh>

NS_MIA_BEGIN

/**
   @ingroup registration

   Kernel for iterative  2D vector field regularizer

   This is the base class for a 2D vector field reguarlizer
   kernels that is typically used in non-linear registration to
   evaluate a velocity field from a force field by applying
   some regularization model on the input data.
*/

class EXPORT_2D C2DFVectorfieldRegularizerKernel : public CProductBase
{
public:
       typedef C2DFVectorfieldRegularizerKernel plugin_type;
       typedef C2DFVectorfield plugin_data;

       class CBuffers
       {
       public:
              virtual ~CBuffers();
       };

       typedef std::unique_ptr<CBuffers> PBuffers;

       static const char *type_descr;

       typedef std::shared_ptr< C2DFVectorfieldRegularizerKernel > Pointer;

       C2DFVectorfieldRegularizerKernel(bool has_pertuberation);

       float evaluate_pertuberation_row(unsigned  y, CBuffers& buffers) const;

       virtual ~C2DFVectorfieldRegularizerKernel();

       void set_data_fields(C2DFVectorfield  *output, const C2DFVectorfield *input);

       void set_update_fields(const T2DDatafield<unsigned char> *update_flags,
                              T2DDatafield<unsigned char> *set_flags, T2DDatafield<float> *residua,
                              float residual_thresh);

       float evaluate_row(unsigned y, CBuffers& buffers);

       float evaluate_row_sparse(unsigned y, CBuffers& buffers);

       unsigned get_boundary_padding() const;

       PBuffers get_buffers() const;

       void start_row(unsigned y, CBuffers& buffers) const;

       bool has_pertuberation() const;
protected:
       C2DFVectorfield& get_output_field() const;
       const C2DFVectorfield& get_input_field() const;
       T2DDatafield<float>& get_residua() const;
       const T2DDatafield<unsigned char>& get_update_flags() const;
       T2DDatafield<unsigned char>& get_set_flags() const;
       float get_residual_thresh() const;
private:
       virtual void post_set_data_fields();

       virtual float do_evaluate_row(unsigned y, CBuffers& buffers) = 0;

       virtual float do_evaluate_row_sparse(unsigned y, CBuffers& buffers) = 0;

       virtual unsigned do_get_boundary_padding() const = 0;

       virtual PBuffers do_get_buffers() const;

       virtual void do_start_row(unsigned y, CBuffers& buffers) const;

       virtual float do_evaluate_pertuberation_row(unsigned  y, CBuffers& buffers) const;

       C2DFVectorfield *m_output;
       const C2DFVectorfield *m_input;
       T2DDatafield<float> *m_residua;
       const T2DDatafield<unsigned char> *m_update_flags;
       T2DDatafield<unsigned char> *m_set_flags;
       float m_residual_thresh;
       bool m_has_pertuberation;
};

inline
bool C2DFVectorfieldRegularizerKernel::has_pertuberation() const
{
       return m_has_pertuberation;
}

inline
C2DFVectorfield& C2DFVectorfieldRegularizerKernel::get_output_field() const
{
       return *m_output;
}

inline
const C2DFVectorfield& C2DFVectorfieldRegularizerKernel::get_input_field() const
{
       return *m_input;
}

inline
T2DDatafield<float>& C2DFVectorfieldRegularizerKernel::get_residua() const
{
       return *m_residua;
}

inline
const T2DDatafield<unsigned char>& C2DFVectorfieldRegularizerKernel::get_update_flags() const
{
       return *m_update_flags;
}

inline
T2DDatafield<unsigned char>& C2DFVectorfieldRegularizerKernel::get_set_flags() const
{
       return *m_set_flags;
}

inline
float C2DFVectorfieldRegularizerKernel::get_residual_thresh() const
{
       return m_residual_thresh;
}

typedef C2DFVectorfieldRegularizerKernel::Pointer P2DVectorfieldRegularizerKernel;

typedef TFactory<C2DFVectorfieldRegularizerKernel> C2DFVectorfieldRegularizerKernelPlugin;

typedef THandlerSingleton<TFactoryPluginHandler<C2DFVectorfieldRegularizerKernelPlugin>> C2DFVectorfieldRegularizerKernelPluginHandler;


template <> const char   *const TPluginHandler<C2DFVectorfieldRegularizerKernelPlugin>::m_help;
extern template class EXPORT_2D  TFactory<C2DFVectorfieldRegularizerKernel>;
extern template class EXPORT_2D  TFactoryPluginHandler<C2DFVectorfieldRegularizerKernelPlugin>;
extern template class EXPORT_2D  THandlerSingleton<TFactoryPluginHandler<C2DFVectorfieldRegularizerKernelPlugin>>;

/**
   @cond NEVER
   @ingroup traits
   @brief  Trait to make C2DFVectorfieldRegularizerPluginHandler available for creation by command line parsing
*/
FACTORY_TRAIT(C2DFVectorfieldRegularizerKernelPluginHandler);
/// @endcond



NS_MIA_END
#endif
