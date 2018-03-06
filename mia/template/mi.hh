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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/splineparzenmi.hh>
#include <mia/core/property_flags.hh>

#include <numeric>
#include <limits>

NS_BEGIN(NS)

///  @cond DOC_PLUGINS

template <typename T>
class TMIImageCost: public T
{
public:
       typedef typename T::Data Data;
       typedef typename T::Force Force;

       TMIImageCost(size_t fbins, mia::PSplineKernel fkernel, size_t rbins, mia::PSplineKernel rkernel, double cut);
private:
       virtual double do_value(const Data& a, const Data& b) const;
       virtual double do_evaluate_force(const Data& a, const Data& b, Force& force) const;
       virtual void post_set_reference(const Data& ref);
       mutable mia::CSplineParzenMI m_parzen_mi;

};


struct FEvalMI : public mia::TFilter<double> {
       FEvalMI( mia::CSplineParzenMI& parzen_mi):
              m_parzen_mi(parzen_mi)
       {}


       template <typename  T, typename  R>
       FEvalMI::result_type operator () (const T& a, const R& b) const
       {
              m_parzen_mi.fill(a.begin(), a.end(), b.begin(), b.end());
              return  m_parzen_mi.value();
       }
       mia::CSplineParzenMI& m_parzen_mi;
};


template <typename T>
TMIImageCost<T>::TMIImageCost(size_t rbins, mia::PSplineKernel rkernel, size_t mbins,
                              mia::PSplineKernel mkernel, double cut):
       m_parzen_mi(rbins, rkernel, mbins,  mkernel, cut)

{
       this->add(::mia::property_gradient);
}

template <typename T>
double TMIImageCost<T>::do_value(const Data& a, const Data& b) const
{
       FEvalMI essd(m_parzen_mi);
       return filter(essd, a, b);
}

template <typename Force>
struct FEvalForce: public mia::TFilter<float> {
       FEvalForce(Force& force, mia::CSplineParzenMI& parzen_mi):
              m_force(force),
              m_parzen_mi(parzen_mi)
       {
       }
       template <typename T, typename R>
       float operator ()( const T& a, const R& b) const
       {
              Force gradient = get_gradient(a);
              m_parzen_mi.fill(a.begin(), a.end(), b.begin(), b.end());
              typename T::const_iterator ai = a.begin();
              typename R::const_iterator bi = b.begin();

              for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi) {
                     float delta = -m_parzen_mi.get_gradient_slow(*ai, *bi);
                     m_force[i] = gradient[i] * delta;
              }

              return m_parzen_mi.value();
       }
private:
       Force& m_force;
       mia::CSplineParzenMI& m_parzen_mi;
};


/**
   This is the force evaluation routine of the cost function
*/
template <typename T>
double TMIImageCost<T>::do_evaluate_force(const Data& a, const Data& b, Force& force) const
{
       assert(a.get_size() == b.get_size());
       assert(a.get_size() == force.get_size());
       FEvalForce<Force> ef(force, m_parzen_mi);
       return filter(ef, a, b);
}

template <typename T>
void TMIImageCost<T>::post_set_reference(const Data& MIA_PARAM_UNUSED(ref))
{
       m_parzen_mi.reset();
}

/**
   This is the plug-in declaration - the actual plugin needs to define the
   cost plugin type and the data type (this could be unified)
   do_test and do_get_descr need to be implemented
*/
template <typename CP, typename C>
class TMIImageCostPlugin: public CP
{
public:
       TMIImageCostPlugin();
       C *do_create()const;
private:
       const std::string do_get_descr() const;
       unsigned int m_rbins;
       unsigned int m_mbins;
       mia::PSplineKernel m_mkernel;
       mia::PSplineKernel m_rkernel;
       float m_histogram_cut;
};


/**
   This plugin will alwasy be "ssd"
*/
template <typename CP, typename C>
TMIImageCostPlugin<CP, C>::TMIImageCostPlugin():
       CP("mi"),
       m_rbins(64),
       m_mbins(64),
       m_histogram_cut(0.0)
{
       TRACE("TMIImageCostPlugin<CP,C>::TMIImageCostPlugin()");
       this->add_property(::mia::property_gradient);
       this->add_parameter("rbins", mia::make_ci_param(m_rbins, 1u, 256u, false,
                           "Number of histogram bins used for the reference image"));
       this->add_parameter("mbins", mia::make_ci_param(m_mbins, 1u, 256u, false,
                           "Number of histogram bins used for the moving image"));
       this->add_parameter("rkernel", mia::make_param(m_rkernel, "bspline:d=0", false,
                           "Spline kernel for reference image parzen hinstogram"));
       this->add_parameter("mkernel", mia::make_param(m_mkernel, "bspline:d=3", false,
                           "Spline kernel for moving image parzen hinstogram"));
       this->add_parameter("cut", mia::make_ci_param(m_histogram_cut, 0.0f, 40.0f,
                           false, "Percentage of pixels to cut at high and low "
                           "intensities to remove outliers"));
}

/**
   The creator routine is also generic
*/
template <typename CP, typename C>
C *TMIImageCostPlugin<CP, C>::do_create() const
{
       return new TMIImageCost<C>(m_rbins, m_rkernel, m_mbins,  m_mkernel, m_histogram_cut);
}

template <typename CP, typename C>
const std::string TMIImageCostPlugin<CP, C>::do_get_descr() const
{
       return "Spline parzen based mutual information.";
}

///  @endcond
NS_END
