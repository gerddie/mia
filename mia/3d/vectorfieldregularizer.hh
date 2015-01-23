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

#ifndef mia_3d_vectorfieldregularizer_hh
#define mia_3d_vectorfieldregularizer_hh


#include <mia/core/factory.hh>
#include <mia/3d/vectorfield.hh>

NS_MIA_BEGIN

/**
   @ingroup registration 

   Regularize a 3D vector field with some model 

   This is the base class for a 3D vector field reguarlizer 
   that is typically used in non-linear registration to 
   evaluate a velocity field from a force field by applying 
   some regularization model on the input data. 
*/

class EXPORT_3D C3DFVectorfieldRegularizer : public CProductBase  {
public: 
        typedef C3DFVectorfieldRegularizer plugin_type; 
        typedef C3DFVectorfieldRegularizer plugin_data; 
        
        static const char *type_descr;
        static const char *data_descr;

        typedef std::shared_ptr< C3DFVectorfieldRegularizer > Pointer; 

        virtual ~C3DFVectorfieldRegularizer(); 

        /**
           The work routine for the regularizer

           \param [out] output  the vector field that contains the reguarlized field 
           \param[in] input  the vector field that containes the unregularized field, 
           e.g. a force field obtained from an image similarity measure. The field 
           may be overwritten by the regularization operation. 
           \returns the maximum of the norms of the vectors of the output field
        */
        double run(C3DFVectorfield& output, C3DFVectorfield& input) const; 

        /**
           Sets the size of the vector fields that will be regularized. This method 
           is implemented to give the regularizer a chance to initialize additional 
           data structures. To do so a derived class has to override the method 
           on_size_changed(). \a on_size_changed() is only called if the size changes.
        */
        void set_size(const C3DBounds& size); 
protected: 
        const C3DBounds& get_size() const; 
private: 
        
        virtual double do_run(C3DFVectorfield& output, C3DFVectorfield& input) const =  0; 
        
        virtual void on_size_changed(); 
        
        C3DBounds m_size; 
}; 


typedef C3DFVectorfieldRegularizer::Pointer P3DVectorfieldRegularizer; 

typedef TFactory<C3DFVectorfieldRegularizer> C3DFVectorfieldRegularizerPlugin; 

typedef THandlerSingleton<TFactoryPluginHandler<C3DFVectorfieldRegularizerPlugin> > C3DFVectorfieldRegularizerPluginHandler;


template <> const char *  const TPluginHandler<C3DFVectorfieldRegularizerPlugin>::m_help; 
extern template class EXPORT_3D  TFactory<C3DFVectorfieldRegularizer>; 
extern template class EXPORT_3D  TFactoryPluginHandler<C3DFVectorfieldRegularizerPlugin>; 
extern template class EXPORT_3D  THandlerSingleton<TFactoryPluginHandler<C3DFVectorfieldRegularizerPlugin> >; 


/**
   @cond NEVER 
   @ingroup traits 
   @brief  Trait to make C3DFVectorfieldRegularizerPluginHandler available for creation by command line parsing 
*/
FACTORY_TRAIT(C3DFVectorfieldRegularizerPluginHandler); 
/// @endcond 


NS_MIA_END

#endif 
