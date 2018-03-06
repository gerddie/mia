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

#ifndef mia_2d_vectorfieldregularizer_hh
#define mia_2d_vectorfieldregularizer_hh


#include <mia/core/factory.hh>
#include <mia/2d/vectorfield.hh>

NS_MIA_BEGIN

/**
   @ingroup registration

   Regularize a 2D vector field with some model

   This is the base class for a 2D vector field reguarlizer
   that is typically used in non-linear registration to
   evaluate a velocity field from a force field by applying
   some regularization model on the input data.
*/

class EXPORT_2D C2DFVectorfieldRegularizer : public CProductBase
{
public:
       typedef C2DFVectorfieldRegularizer plugin_type;
       typedef C2DFVectorfield plugin_data;

       static const char *type_descr;

       typedef std::shared_ptr< C2DFVectorfieldRegularizer > Pointer;

       virtual ~C2DFVectorfieldRegularizer();

       /**
          The work routine for the regularizer

          \param [out] velocity  the vector field that contains the reguarlized field
          \param[in] force  the vector field that containes the unregularized field,
          e.g. a force field obtained from an image similarity measure. The field
          may be overwritten by the regularization operation.
         \param[in] deform the currently valid deformation
          \returns the maximum of the norms of the vectors of the output field
       */
       double run(C2DFVectorfield& velocity, C2DFVectorfield& force, const C2DFVectorfield& deform) const;

       /**
          Sets the size of the vector fields that will be regularized. This method
          is implemented to give the regularizer a chance to initialize additional
          data structures. To do so a derived class has to override the method
          on_size_changed(). \a on_size_changed() is only called if the size changes.
       */
       void set_size(const C2DBounds& size);
protected:
       const C2DBounds& get_size() const;
private:

       virtual double do_run(C2DFVectorfield& velocity, C2DFVectorfield& force, const C2DFVectorfield& deform) const =  0;

       virtual void on_size_changed();

       C2DBounds m_size;
};


typedef C2DFVectorfieldRegularizer::Pointer P2DVectorfieldRegularizer;

typedef TFactory<C2DFVectorfieldRegularizer> C2DFVectorfieldRegularizerPlugin;

typedef THandlerSingleton<TFactoryPluginHandler<C2DFVectorfieldRegularizerPlugin>> C2DFVectorfieldRegularizerPluginHandler;


template <> const char   *const TPluginHandler<C2DFVectorfieldRegularizerPlugin>::m_help;
extern template class EXPORT_2D  TFactory<C2DFVectorfieldRegularizer>;
extern template class EXPORT_2D  TFactoryPluginHandler<C2DFVectorfieldRegularizerPlugin>;
extern template class EXPORT_2D  THandlerSingleton<TFactoryPluginHandler<C2DFVectorfieldRegularizerPlugin>>;


/**
   @cond NEVER
   @ingroup traits
   @brief  Trait to make C2DFVectorfieldRegularizerPluginHandler available for creation by command line parsing
*/
FACTORY_TRAIT(C2DFVectorfieldRegularizerPluginHandler);
/// @endcond


NS_MIA_END

#endif
