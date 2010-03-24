/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef mia_core_combiner_hh
#define mia_core_combiner_hh

#include <string>
#include <mia/core/shared_ptr.hh>
#include <mia/core/defines.hh>


NS_MIA_BEGIN

class EXPORT_CORE CCombinerResult {
public:
	virtual ~CCombinerResult();
	void save(const std::string& fname)const;
private:
	virtual void do_save(const std::string& fname) const = 0;
};

typedef EXPORT_CORE SHARED_PTR(CCombinerResult) PCombinerResult;

NS_MIA_END

#endif
