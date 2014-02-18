/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

void CProgressCallback::pulse() 
{
	do_pulse(); 
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
	delete impl; 
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

void CMsgStreamPrintCallback::do_pulse()
{
	cverb << '.'; 
}

CMsgStreamPrintCallbackImpl::CMsgStreamPrintCallbackImpl(const std::string& f):
	format(f), 
	range(0) 
{
}

NS_MIA_END
