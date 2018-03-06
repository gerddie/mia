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

#ifndef mia_core_combiner_hh
#define mia_core_combiner_hh

#include <memory>
#include <string>
#include <boost/any.hpp>
#include <mia/core/defines.hh>


NS_MIA_BEGIN

class EXPORT_CORE CCombinerResult
{
public:
       virtual ~CCombinerResult();
       void save(const std::string& fname)const;
       boost::any get() const;
private:
       virtual void do_save(const std::string& fname) const = 0;
       virtual boost::any do_get() const = 0;
};

typedef std::shared_ptr<CCombinerResult > PCombinerResult;

NS_MIA_END

#endif
