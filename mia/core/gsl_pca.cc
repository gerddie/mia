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


#include <mia/core/gsl_pca.hh>
#include <algorithm>
#include <cassert>
#include <vector>

#include <gsl/gsl_eigen.h>

namespace gsl
{

using std::pair;
using std::sort;
using std::vector;

PCA::PCA(unsigned max_pc, double energy_thresh_ratio):
       m_max_pc(max_pc),
       m_energy_thresh_ratio(energy_thresh_ratio),
       m_eval_min(1e-9)
{
       assert(max_pc > 0  ||
              (m_energy_thresh_ratio >= 0.0 || m_energy_thresh_ratio <= 1.0));
}

void PCA::set_eigenvalue_minimum(double min)
{
       assert(m_eval_min >= 0);
       m_eval_min = min;
}

PCA::Result PCA::analyze(const Matrix& signal) const
{
       auto  covariance_matrix = signal.row_covariance();
       // evaluate eigen-decomposition of covariance matrix
       CSymmvEvalEvec see(covariance_matrix);
       double ev_energy = 0.0;

       for (unsigned i = 0; i < see.eval.size(); ++i) {
              ev_energy += see.eval[i];
       }

       gsl_eigen_symmv_sort (see.eval, see.evec, GSL_EIGEN_SORT_ABS_DESC);
       unsigned max_useful_eigen = 0;

       if (m_max_pc < 1) {
              // automatic number based on culmulative energy threshold
              double ev_part_energy = 0.0;
              double ev_part_thresh = m_energy_thresh_ratio * ev_energy;

              while ((ev_part_energy < ev_part_thresh) &&
                     (max_useful_eigen < see.eval.size())) {
                     ev_part_energy += see.eval[max_useful_eigen];
                     ++max_useful_eigen;
              }
       } else {
              // fixed number of PC/IC
              while ((see.eval[max_useful_eigen] > m_eval_min) &&
                     (max_useful_eigen < see.eval.size()) &&
                     (max_useful_eigen < m_max_pc))
                     ++max_useful_eigen;
       }

       PCA::Result result;

       if (max_useful_eigen >= 1) {
              // copy the useful eigenvalues and eigenvectors over
              result.eval = Vector(max_useful_eigen, false);
              result.evec.reset(covariance_matrix.rows(), max_useful_eigen, false);

              for (unsigned i = 0; i < max_useful_eigen; ++i) {
                     result.eval[i] = see.eval[i];
                     auto in_col = gsl_matrix_column (see.evec, i);
                     auto out_col = gsl_matrix_column (result.evec, i);
                     gsl_vector_memcpy (&out_col.vector, &in_col.vector);
              }
       }

       return result;
}

}
