/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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


#ifndef gslpp_pca_hh
#define gslpp_pca_hh

#include <gsl++/matrix.hh>
#include <gsl++/vector.hh>

namespace gsl {


/**
   This class implements a PCA estimator. The number of principal components 
   to be evaluated can be defined in two ways: Either a hard number is given 
   in the constructor, or the energy ratio is defined. In both cases PC are 
   added starting from the component with the eigenvalue with the largest 
   absolute value of the covariance matrix. In the first case the requested 
   number of components is returned, unless smaller eigenvalues have a lower 
   value than a given threshold value (default 1e-9). In the energy-ratio 
   based case values are added as long as the ratio of the sum of the added 
   eigenvalues with respect to the sum of all eigenvalues is below the 
   user-defined ratio. 
*/

class PCA {
public: 
        typedef struct {
                DoubleVector eval; 
                Matrix evec; 
        } Result;

        /**
           Construct the PCA analysis 
           \param max_pc maximum number of principal components to be returned. 
           if this parameter is set to zero, then the number is estimated based on 
           the energy ration given in the secons parameter 
           \param energy_thresh_ratio - if the number of principal components is to 
           be evaluated automatically, then this value defines the condition when to 
           stop adding components. The value must be in the range (0,1). 
         */
        PCA(unsigned max_pc, double energy_thresh_ratio); 
        
        /** 
            Set an alternative threshold for eigenvalues to be ignored. 

            \param min new minimum for acceptable eigenvalue values. 
         */
        void set_eigenvalue_minimum(double min); 


        /**
           Run the PCA on a given input data set. 
           \param matrix the input data the signals to be separated 
           are stored in the columns
         */
        Result analyze(const Matrix& signal) const; 
        
private: 
        unsigned m_max_pc; 
        double m_energy_thresh_ratio; 
        double m_eval_min; 

};

}


#endif 
