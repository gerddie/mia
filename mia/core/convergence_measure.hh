/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Genoa 1999-2016 Gert Wollny
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


#ifndef mia_core_convergence_hh
#define mia_core_convergence_hh

#include <queue>
#include <cstdint>

#include <mia/core/defines.hh> 

namespace mia {

class EXPORT_CORE CConvergenceMeasure {
public: 
        CConvergenceMeasure(uint32_t size);

        void push(double value);

        double value() const;

        uint32_t fill() const;

        bool is_full_size() const;

private:
        std::queue<double> m_values;
        uint32_t m_size;
        double m_current_sum; 
        
}; 

}; // namespace mia

#endif 
