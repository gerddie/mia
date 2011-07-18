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

#ifndef mia_core_dlloader_hh
#define mia_core_dlloader_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN
/**  
     \ingroup infrastructure 
     \brief Portabe Dynamic Link Library Loader
     
     This class provides a portable way to load dynamic link libraries. 
     \remark This class doesn't release the loaded library when it is destroyed.
     
     
*/
class EXPORT_CORE CDLLoader {
public:
        /** Constructor that loads the DLL. If the Library \a name is not found,
            the constructor will throw a \a std::runtime_error .
        \param name name of the library
        \param flags flags used during loading (only used in UNIX)
        */
	CDLLoader(const char *name, int flags);

        /** Destructor; currently it does nothing (should unload the library), but currently
            it is not ensured that the code is not referenced from some plug-in.
         */
	~CDLLoader();

        /** Get a function address based on its name
        \param name of the function to look up
        \returns pointer to the function or 0
        */
	void *get_function(const char *name) const;

	/// \returns the module name
	const std::string& get_name() const;
private:
	struct CDLLoaderData *impl;
};

NS_MIA_END

#endif
