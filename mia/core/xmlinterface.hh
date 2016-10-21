/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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


#ifndef mia_core_xmlinterface_hh
#define mia_core_xmlinterface_hh

#include <mia/core/defines.hh>
#include <memory> 


NS_MIA_BEGIN

/**
   \brief This class implements a facade for the xml Element 

   This class implements a facade for the XML IO to make it (later)
   possible to change the used XML implementation. It is also useful 
   to reduce the compile time dependencies for programs that use MIA. 

   
*/


class EXPORT_CORE CXMLElement {
public:
	typedef std::shared_ptr<CXMLElement> Pointer; 

	CXMLElement(const char *name);
	~CXMLElement(); 
	
	// do not allow copying 
	CXMLElement(const CXMLElement& orig) = delete;
	CXMLElement& operator = (const CXMLElement& orig) = delete;

	/**
	   \brief add a new child element 

	   This method adds a new child element to this node
	   \param name name tag of the new child element 
	   \returns the newly created node 

	 */
	CXMLElement::Pointer add_child(const char *name);

	/**
	   \brief Set an attribute of the node 

	   This method sets an attribute of a node 
	   \param name attribute name 
	   \param value (string) value of the attribute 
	 */
	void set_attribute(const char *name, const std::string& value);

	/**
	   Set the child text of the node 
	   \param value text value to be set 
	 */
	void set_child_text(const std::string& value);
private:
	friend class CXMLDocument; 
	struct CXMLElementImpl *impl; 
}; 


/**
   \brief facate for an XML document 

   This class implements a facade for a XML document.  
   
   

*/

class EXPORT_CORE CXMLDocument {
public: 
	CXMLDocument();

	~CXMLDocument();

	CXMLDocument(const CXMLDocument& other) = delete;
	CXMLDocument& operator = (const CXMLDocument& other) = delete;
		
	
	CXMLElement::Pointer create_root_node(const char *);
	std::string write_to_string_formatted() const;

private:
	
	struct CXMLDocumentImpl *impl; 
}; 

NS_MIA_END

#endif 
