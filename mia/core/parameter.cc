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

#include <sstream>
#include <mia/core/parameter.hh>
#include <mia/core/parameter.cxx>


NS_MIA_BEGIN
using namespace std;

CParameter::CParameter(const char type[], bool required, const char *descr):
    _M_required(required),
    _M_type(type),
    _M_descr(descr)
{
}

CParameter::~CParameter()
{
}

const char *CParameter::type() const
{
	return _M_type;
}

void CParameter::descr(ostream& os) const
{
	os << ":" << _M_type << "\t" << _M_descr << " ";
	do_descr(os);
	if (_M_required)
		os << "(required)";
}

const char *CParameter::get_descr() const
{
	return _M_descr;
}

const std::string CParameter::errmsg(const std::string& err_value) const
{
	std::stringstream msg;
	msg <<  "Parameter<"<< type() << ">::set(" << get_descr() <<
		"): value '" << err_value << "' not allowed";
	return msg.str();
}

bool CParameter::required_set() const
{
	return _M_required;
}

bool CParameter::set(const string& str_value)
{
	_M_required = false;
	return do_set(str_value);
}

const char  type_str_uint[5] = "uint";
const char  type_str_int[4] = "int";
const char  type_str_float[6] = "float";
const char  type_str_string[7] = "string";
const char  type_str_bool[5] = "bool";

template class TRangeParameter<unsigned int, type_str_uint>; 
template class TRangeParameter<int, type_str_int>;
template class TRangeParameter<float, type_str_float>;
template class CTParameter<string,type_str_string>;
template class CTParameter<bool, type_str_bool>;


NS_MIA_END
