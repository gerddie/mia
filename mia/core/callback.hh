/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#ifndef mia_callback_hh
#define mia_callback_hh

#include <string>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup misc
   \brief a class to provide a feedback mechanism to show progress 

   This class is used as a base class in order to show feedback about the 
   progress of certain operations that may take a long time. 
   The indicator supports two modes of operation: If the number if steps 
   is known beforehand, the indicator will act like a classical progress bar, and 
   If the number of stels is unknown, some kind of change indicator is applied 
   to show that "something happens". 
*/

class EXPORT_CORE CProgressCallback {
public: 
	virtual ~CProgressCallback(); 

	/**
	   Set the maximum number of the possible steps some operatoion may take 
	   \param range  
	 */
	void set_range(int range); 

	/**
	   Update the progress indicator by using the given value  - it should be 
	   less or equal to the value given in set_range(). 
	   \param step 
	 */
	void update(int step);

	/**
	   Update the progress indicator in some way to show that something is happening 
	   but without giving real target informations. 

	 */
	void pulse();
private:
	virtual void do_update(int step) = 0;
	virtual void do_set_range(int range) = 0; 
	virtual void do_pulse() = 0;
}; 

/**
   \ingroup helpers 
   \brief a class to provide a feedback mechanism to show progress based on textual output 
   
   This class provides progress feedback on textual outputs. The range based output 
   shows progress in percent, and the pluse output prints a '.' whenever it is called. 
*/
class  EXPORT_CORE CMsgStreamPrintCallback: public CProgressCallback {
public: 

	/**
	   Constructor for the text based callback that uses msgstream for output. 
	   \param format the output format. It should contain the format descriptors 
	   %1% for the current progress and %2% for the progress range. 
	 */
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
