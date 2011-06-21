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

#ifndef mia_callback_hh
#define mia_callback_hh

#include <string>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

class CProgressCallback {
public: 
	virtual ~CProgressCallback(); 
	void set_range(int range); 
	void update(int step);
	void pulse();
private:
	virtual void do_update(int step) = 0;
	virtual void do_set_range(int range) = 0; 
	virtual void do_pulse() = 0;
}; 

class CMsgStreamPrintCallback: public CProgressCallback {
public: 
	CMsgStreamPrintCallback(const std::string& format); 
	virtual ~CMsgStreamPrintCallback(); 
private: 
	virtual void do_update(int step);
	virtual void do_set_range(int range); 
	virtual void do_pulse();
	
	struct CMsgStreamPrintCallbackImpl *impl; 
}; 

NS_MIA_END

#endif