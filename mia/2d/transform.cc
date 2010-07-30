/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <mia/2d/transform.hh>
#include <mia/2d/deformer.hh>


#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN
using namespace boost::lambda;

C2DTransformation::C2DTransformation()
{

}

void C2DTransformation::reinit() const
{
}

void C2DTransformation::set_creator_string(const std::string& s)
{
	_M_creator_string = s; 
}


const std::string& C2DTransformation::get_creator_string()const
{
	return _M_creator_string; 
}

C2DTransformation *C2DTransformation::clone() const
{
	C2DTransformation *result = do_clone(); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result; 
}


NS_MIA_END
