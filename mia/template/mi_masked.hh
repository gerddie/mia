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

#ifndef mia_template_mi_masked_hh
#define mia_template_mi_masked_hh

#include <mia/core/splineparzenmi.hh>

NS_BEGIN(NS)

///  @cond DOC_PLUGINS

template <typename T>
class TMIMaskedImageCost: public T
{
public:
       typedef typename T::Data Data;
       typedef typename T::Mask Mask;
       typedef typename T::Force Force;

       TMIMaskedImageCost(size_t fbins, mia::PSplineKernel fkernel, size_t rbins, mia::PSplineKernel rkernel, double cut);
private:
       virtual double do_value(const Data& a, const Data& b, const Mask& m) const;
       virtual double do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const;
       virtual void post_set_reference(const Data& ref);
       mutable mia::CSplineParzenMI m_parzen_mi;

};



/**
   This is the plug-in declaration - the actual plugin needs to define the
   cost plugin type and the data type (this could be unified)
   do_test and do_get_descr need to be implemented
*/
template <typename CP, typename C>
class TMIMaskedImageCostPlugin: public CP
{
public:
       TMIMaskedImageCostPlugin();
       C *do_create()const;
private:
       const std::string do_get_descr() const;
       unsigned int m_rbins;
       unsigned int m_mbins;
       mia::PSplineKernel m_mkernel;
       mia::PSplineKernel m_rkernel;
       float m_histogram_cut;
};


///  @endcond
NS_END

#endif
