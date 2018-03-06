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

#include <mia/core/minimizer.hh>

NS_BEGIN(gdas)

class CGDSAMinimizer: public mia::CMinimizer
{

public:

       CGDSAMinimizer(double min_step, double max_step,
                      double gtol, double ftolr,
                      unsigned int maxiter);

private:
       void do_set_problem();
       double get_gradmax(mia::CDoubleVector& x);
       int do_run(mia::CDoubleVector& x);

       double m_min_step;
       double m_max_step;
       double m_ftolr;
       double m_xtol;
       unsigned int m_maxiter;

};

class CGDSAMinimizerPlugin: public mia::CMinimizerPlugin
{
public:
       CGDSAMinimizerPlugin();
private:
       mia::CMinimizer *do_create() const;
       const std::string do_get_descr() const;

       double m_min_step;
       double m_max_step;
       double m_ftolr;
       double m_xtol;
       unsigned int  m_maxiter;

};

NS_END
