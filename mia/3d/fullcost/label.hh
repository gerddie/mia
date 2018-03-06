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

#ifndef mia_3d_imagefullcost_hh
#define mia_3d_imagefullcost_hh


#include <mia/3d/fullcost.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/cost.hh>

NS_MIA_BEGIN

class EXPORT C3DLabelFullCost : public C3DFullCost
{
public:
       C3DLabelFullCost(const std::string& src,
                        const std::string& ref,
                        double weight,
                        int maxlabels);
private:
       double do_evaluate(const C3DTransformation& t, CDoubleVector& gradient) const;
       void do_set_size();

       static P3DImage get_from_pool(const C3DImageDataKey& key);

       double do_value(const C3DTransformation& t) const;
       bool do_get_full_size(C3DBounds& size) const;

       double value(int idx, int label) const;
       double value_and_gradient(int idx, const C3DUBImage::const_range_iterator_with_boundary_flag& i,
                                 C3DFVector& gradient) const;
       void prepare_distance_fields(const C3DUBImage& image);

       double do_value() const;
       void do_reinit();

       C3DImageDataKey m_src_key;
       C3DImageDataKey m_ref_key;

       P3DImage m_src;
       P3DImage m_ref;

       C3DUBImage m_src_scaled;
       C3DUBImage m_ref_scaled;

       std::vector<bool> m_ref_label_exists;
       std::vector<C3DFImage> m_ref_distances;

       enum EBoundaries {
              eb_none = 0,
              eb_xlow  = 1, /**< at low x-boundary */
              eb_xhigh = 2, /**< at high x-boundary */
              eb_x = 3, /**< at high x-boundary */
              eb_ylow = 4,  /**< at low y-boundary */
              eb_yhigh = 8, /**< at high y-boundary */
              eb_y = 12, /**< any y-boundary */
              eb_zlow = 0x10,  /**< at low y-boundary */
              eb_zhigh = 0x20, /**< at high y-boundary */
              eb_z = 0x30, /**< any y-boundary */

       };


};

// plugin implementation
class C3DLabelFullCostPlugin: public C3DFullCostPlugin
{
public:
       C3DLabelFullCostPlugin();
private:
       C3DFullCost *do_create(float weight) const;
       const std::string do_get_descr() const;


       std::string m_src_name;
       std::string m_ref_name;
       int m_maxlabel;
};


NS_MIA_END

#endif
