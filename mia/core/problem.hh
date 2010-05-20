/* -*- mona-c++  -*-
 *
 * Copyright (c) Madrid 2010 BIT, ETSI Telecomunicacion, UPM
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


#ifndef mia_core_problem_hh
#define mia_core_problem_hh

#include <vector>
#include <mia/core/property_flags.hh>

NS_MIA_BEGIN

class EXPORT_CORE CProblem  {
public: 
	void setup(); 
	void finalize();
	bool has_all_properties_in(const CPropertyFlagHolder& holder) const; 
	const char *get_name() const; 
	double get_criterion(std::vector<float>& gradient) const; 
	void update_params(const std::vector<float>& delta); 
private: 
	virtual void do_setup() = 0; 
	virtual void do_finalize() = 0; 
	virtual bool do_has_all_properties_in(const CPropertyFlagHolder& holder) const = 0; 
	virtual const char *do_get_name() const = 0; 
	virtual double do_get_criterion(std::vector<float>& gradient) const = 0; 
	virtual void do_update_params(const std::vector<float>& delta) = 0; 

}; 

NS_MIA_END

#endif
