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


#ifndef mia_core_transformation_hh
#define mia_core_transformation_hh

#include <mia/core/iodata.hh>
#include <mia/core/attributes.hh>
#include <mia/core/vector.hh>

NS_MIA_BEGIN


/**
   \ingroup registration
   \brief generic base class for transformations

   Template of a generic tranformation function
   \tparam D data type of the object to be transformed
   \tparam I interpolator factory to create resampler for type D
   \todo the interpolator factory resp. the interpolator kernel
         should probably be part of the class
 */

template <typename D, typename I>
class Transformation : public CIOData, public CAttributedData
{
public:

       /// interface type for plugin implementation and search
       static const char *type_descr;

       virtual ~Transformation();

       /// typedef for the data type to be transformed by this transformation
       typedef D Data;

       /// type of the interpolator used by this transformation
       typedef I InterpolatorFactory;

       /**
          Constructor to create the transformstion
          \param ipf the interpolator factory to be used to create the interpolators for image interpolation
        */
       Transformation(const I& ipf);

       /** Apply the transformation to the input data
           \param input
           \returns a shared pointer to the transformed input data
       */
       std::shared_ptr<D> operator () (const D& input) const;

       /** Apply the transformation to the input data
           \param input
           \param ipf_override overide the image interpolator
           \returns a shared pointer to the transformed input data
       */
       std::shared_ptr<D> operator () (const D& input, const I& ipf_override) const;

       /**
          Set the interpolator factory
          \param ipf the new interpolator factory
        */
       void set_interpolator_factory(const I& ipf);


       /**
          Evaluate the transformation penalty and it's gradient
          \param[in,out] gradient at input an allocated vector of the size equal to the
          size of the degrees of freedom of the transformation, at output the
          enegy penalty  gradient with respect to the transformation parameters
          \returns the value of the transformation energy penalty
        */
       double get_energy_penalty_and_gradient(CDoubleVector& gradient) const;

       ///  \returns the value of the transformation energy penalty
       double get_energy_penalty() const;

       /// \returns true if the transformation provides a penalty term
       bool has_energy_penalty() const;
protected:

       /// \returns the interpolator factory
       const I& get_interpolator_factory() const;
private:
       virtual std::shared_ptr<D> do_transform(const D& input, const I& ipf) const = 0;
       virtual double do_get_energy_penalty_and_gradient(CDoubleVector& gradient) const;
       virtual double do_get_energy_penalty() const;
       virtual bool do_has_energy_penalty() const;

       I m_ipf;

};

/**
   \ingroup io
   \brief template to unify transformation loading
   \tparam T the type of the transformation to be loaded
   \param file the file name of the transformation to be loaded
   \returns the loaded transformation or an empty shared:ptr
*/
template <typename T>
T load_transform(const std::string& MIA_PARAM_UNUSED(file))
{
       static_assert(sizeof(T) == 0, "this needs to specialized for the handled type");
}

// implementation
template <typename D, typename I>
Transformation<D, I>::Transformation(const I& ipf):
       m_ipf(ipf)
{
}

template <typename D, typename I>
Transformation<D, I>::~Transformation()
{
}

template <typename D, typename I>
void Transformation<D, I>::set_interpolator_factory(const I& ipf)
{
       m_ipf = ipf;
}

template <typename D, typename I>
const I& Transformation<D, I>::get_interpolator_factory() const
{
       return m_ipf;
}

template <typename D, typename I>
std::shared_ptr<D> Transformation<D, I>::operator () (const D& input, const I& ipf_override) const
{
       return do_transform(input, ipf_override);
}

template <typename D, typename I>
std::shared_ptr<D > Transformation<D, I>::operator() (const D& input) const
{
       return do_transform(input, m_ipf);
}

template <typename D, typename I>
double Transformation<D, I>::get_energy_penalty_and_gradient(CDoubleVector& gradient) const
{
       return do_get_energy_penalty_and_gradient(gradient);
}


template <typename D, typename I>
double Transformation<D, I>::get_energy_penalty() const
{
       return do_get_energy_penalty();
}

template <typename D, typename I>
double Transformation<D, I>::do_get_energy_penalty_and_gradient(CDoubleVector& gradient) const
{
       std::fill(gradient.begin(), gradient.end(), 0.0);
       return 0.0;
}


template <typename D, typename I>
double Transformation<D, I>::do_get_energy_penalty() const
{
       return 0.0;
}

template <typename D, typename I>
bool Transformation<D, I>::has_energy_penalty() const
{
       return do_has_energy_penalty();
}


template <typename D, typename I>
bool Transformation<D, I>::do_has_energy_penalty() const
{
       return false;
}

template <typename D, typename I>
const char *Transformation<D, I>::type_descr = "transform";

NS_MIA_END


#endif
