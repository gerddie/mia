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

#include <mia/core/product_base.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

void CProductBase::set_module(PPluginModule module)
{
	m_module = module;
}

void CProductBase::set_init_string(const char *init)
{
	m_init_string = init; 
}

const char *CProductBase::get_init_string() const
{
	return m_init_string.c_str(); 
}


NS_MIA_END

