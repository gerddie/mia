/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/parallelcxx11.hh>

NS_MIA_BEGIN

int CMaxTasks::get_max_tasks()
{
        if (max_tasks < 0) {
                max_tasks = std::thread::hardware_concurrency(); 
        }
        return max_tasks; 
}

void CMaxTasks::set_max_tasks(int mt)
{
        max_tasks = mt; 
}

int CMaxTasks::max_tasks = -1;


NS_MIA_END
