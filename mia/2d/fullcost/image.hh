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

#ifndef mia_2d_imagefullcost_hh
#define mia_2d_imagefullcost_hh


#include <mia/2d/fullcost.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/cost.hh>

NS_MIA_BEGIN

class EXPORT C2DImageFullCost : public C2DFullCost
{
public:
       C2DImageFullCost(const std::string& src,
                        const std::string& ref,
                        C2DImageCostPluginHandler::ProductPtr cost,
                        double weight,
                        bool debug);
private:
       double do_evaluate(const C2DTransformation& t, CDoubleVector& gradient) const;
       void do_set_size();

       static P2DImage get_from_pool(const C2DImageDataKey& key);

       bool do_has(const char *property) const;
       double do_value(const C2DTransformation& t) const;
       bool do_get_full_size(C2DBounds& size) const;

       double do_value() const;
       void do_reinit();

       C2DImageDataKey m_src_key;
       C2DImageDataKey m_ref_key;

       P2DImage m_src;
       P2DImage m_ref;

       P2DImage m_src_scaled;
       P2DImage m_ref_scaled;

       P2DImageCost m_cost_kernel;
       bool m_debug;
};

NS_MIA_END

#endif
