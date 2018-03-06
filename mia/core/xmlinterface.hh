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


#ifndef mia_core_xmlinterface_hh
#define mia_core_xmlinterface_hh

#include <mia/core/defines.hh>
#include <memory>
#include <vector>


NS_MIA_BEGIN

class EXPORT_CORE CXMLDocument;

/**
   \brief This class implements a facade for the xml Element

   This class implements a facade for the XML IO to makes it
   possible to change the used underlying XML implementation.
   It is also useful to reduce the compile time dependencies
   for programs that use MIA.

   The implemented support for XML is limted, i.e. there is no
   support for namespaces, schemes, and interleaved node
   content.

*/


class EXPORT_CORE CXMLElement
{
public:
       typedef std::shared_ptr<CXMLElement> Pointer;

       /**
          Create a new node
          \param name tag of the node
        */
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

       /// \returns Get name tag of the node
       const std::string& get_name() const;

       /**
          retrive an attribute value
          \param name attribute name to retrive
          \returns attribute value as string, if the attribute doesn't
          extist an empty string is returned
       */
       const std::string get_attribute(const std::string& name) const;


       /// \returns all child nodes

       const std::vector<CXMLElement::Pointer>& get_all_children() const;


       /**
          retrive first level children with a certain tag

          \param name tag of the nodes to be retrived
          \returns a vector containing the nodes with the requested tag
       */
       std::vector<CXMLElement::Pointer> get_children(const char *name) const;

       /// 	   \returns the content of the node as string
       const std::string& get_content() const;

private:

       struct CXMLElementImpl *impl;
       friend CXMLElementImpl;
       friend class CXMLDocument;
};


/**
   \brief facate for an XML document

   This class implements a very simple facade for a XML document that
   only supports reading, writing, and retriving the root node of the
   document.

*/
class EXPORT_CORE CXMLDocument
{
public:
       /**
          Create an empty document
        */

       CXMLDocument();

       /**
          Read the document from a charactar string that comprises
          valid XML.
        */
       CXMLDocument(const char *init);


       /**
          Create a new root node and return it
          \param name tag of the root node
          \returns the newly created node
        */
       CXMLElement::Pointer create_root_node(const char *name);

       /**
           \returns the root node of the document, it may be an
           empty shared pointer
       */
       CXMLElement::Pointer get_root_node() const;

       /**
          Read the document tree from an XML formatted string
          \param init the valid XML string
          \returns true if successful, false otherwise
       */
       bool read_from_string(const char *init);

       /**
          Write the document data as XML to a string.
          \param encoding specify the encoding to be used
          \param formatted write the xml in a nicely formatted, human readable way
          \returns the string containing the xml tree
       */
       std::string write_to_string(const char *encoding = NULL, bool formatted = true) const;

private:
       friend class CXMLElement;
       CXMLElement::Pointer m_root;

       std::string m_last_error;
};

NS_MIA_END

#endif
