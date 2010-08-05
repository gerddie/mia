/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/3d/transform.hh>
#include <mia/3d/deformer.hh>


#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN
using namespace boost::lambda;

C3DTransformation::C3DTransformation()
{

}

bool C3DTransformation::save(const std::string& filename, const std::string& type) const
{
	return do_save(filename, type);
}

size_t C3DTransformation::degrees_of_freedom() const
{
	return do_degrees_of_freedom();
}

P3DTransformation C3DTransformation::upscale(const C3DBounds& size) const
{
	return do_upscale(size);
}

void C3DTransformation::add(const C3DTransformation& a)
{
	do_add(a);
}


NS_MIA_END
