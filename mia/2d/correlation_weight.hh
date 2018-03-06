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

#ifndef mia_correlation_weight_hh
#define mia_correlation_weight_hh

#include <vector>
#include <mia/2d/image.hh>

NS_MIA_BEGIN

/**
   @ingroup perf
   \brief evaluate the correlation between neighboring pixels in an image series

   Evaluate time-intensity correlations of neighboring pixels in a series of images.
   This class is used to evaluate a pseude ground truth for motion compensation.
   See C2DGroundTruthEvaluator for more information

 */

class EXPORT_2D CCorrelationEvaluator
{
public:
       /**
          helper structure to hold the correlation data
        */
       typedef struct {
              /// data field to hold the correlation in horizontal direction
              C2DFImage horizontal;
              /// data field to hold the correlation in vertical direction
              C2DFImage vertical;
       } result_type;

       /**
          Construct the correlation evaluator
          @param thresh considers what correlation should identify pixels as belonging
          to the same tissue.
        */
       CCorrelationEvaluator(double thresh);

       ~CCorrelationEvaluator();

       /**
          Evaluate the time-intensity correlation images
          @param images image series
          @param skip number of images that should be skipped at the beginning of the series.
        */
       result_type operator() (const std::vector<P2DImage>& images, size_t skip = 0) const;

private:
       struct CCorrelationEvaluatorImpl *impl;
};

NS_MIA_END

#endif
