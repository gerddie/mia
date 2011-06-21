/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
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

#include <mia/core/msgstream.hh>
#include <mia/core/callback.hh>
#include <boost/format.hpp>

NS_MIA_BEGIN

CProgressCallback::~CProgressCallback()
{
}

void CProgressCallback::update(int step) 
{
	do_update(step); 
}

void CProgressCallback::set_range(int range) 
{
	do_set_range(range); 
}

struct CMsgStreamPrintCallbackImpl {
	CMsgStreamPrintCallbackImpl(const std::string& f); 
	boost::format format; 
	int range; 
}; 


CMsgStreamPrintCallback::CMsgStreamPrintCallback(const std::string& format):
	impl(new CMsgStreamPrintCallbackImpl(format))
{
}


CMsgStreamPrintCallback::~CMsgStreamPrintCallback()
{
	cvmsg() << "\n";
}

void CMsgStreamPrintCallback::do_set_range(int range)
{
	impl->range = range; 
}

void CMsgStreamPrintCallback::do_update(int step)
{
	impl->format % step % impl->range; 
	auto str = impl->format.str(); 
	cvmsg() << str << "\r"; 
}

CMsgStreamPrintCallbackImpl::CMsgStreamPrintCallbackImpl(const std::string& f):
	format(f), 
	range(0) 
{
}

NS_MIA_END
