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

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sstream>

#include <mia/core/xmlinterface.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

using std::string;
using std::stringstream; 
using std::make_shared; 
using std::vector; 

struct CXMLElementImpl {
        CXMLElementImpl(const char *name);
        ~CXMLElementImpl();
        
        xmlNodePtr element;
        vector<CXMLElement::Pointer> children;
}; 

CXMLElementImpl::CXMLElementImpl(const char *name)
{
        element = xmlNewNode(nullptr, BAD_CAST name);
}

CXMLElementImpl::~CXMLElementImpl()
{
        // here the element should be freed if it is not a child
        
}
        

CXMLElement::CXMLElement(const char *name):
        impl(new CXMLElementImpl(name))
{
        
}

CXMLElement::~CXMLElement()
{
        delete impl; 
}


CXMLElement::Pointer CXMLElement::add_child(const char *name)
{
        Pointer result = make_shared<CXMLElement>(name);
        impl->children.push_back(result);
        xmlAddChild(impl->element, result->impl->element); 
        return result; 
}

void CXMLElement::set_child_text(const std::string& value)
{
	xmlNodeAddContent(impl->element, BAD_CAST value.c_str()); 
}

void CXMLElement::set_attribute(const char *name, const std::string& value)
{
	xmlChar *attr = xmlGetProp(impl->element, BAD_CAST name);
	if (!attr)  {
		xmlNewProp(impl->element, BAD_CAST name,  BAD_CAST value.c_str());
	}else{
		xmlChar *path = xmlGetNodePath(impl->element); 
		cvdebug() << "CXMLElement: trying to add attribute " << name << " a second time to '"
                  << path << "'\n";
		xmlFree(path);
		xmlFree(attr);
	}
}


struct CXMLDocumentImpl {
        CXMLDocumentImpl();
        ~CXMLDocumentImpl();

        xmlDocPtr doc;
}; 

CXMLDocumentImpl::CXMLDocumentImpl()
{
        doc = xmlNewDoc(BAD_CAST "1.0");
}

CXMLDocumentImpl::~CXMLDocumentImpl()
{
        xmlFreeDoc(doc);
}

       
CXMLDocument::CXMLDocument():
        impl(new CXMLDocumentImpl)
{
}
        

CXMLDocument::~CXMLDocument()
{
        delete impl;
        xmlCleanupParser();
        
}

	
CXMLElement::Pointer CXMLDocument::create_root_node(const char *name)
{
        auto result = make_shared<CXMLElement>(name);
        xmlDocSetRootElement(impl->doc, result->impl->element);
        return result; 
}

string CXMLDocument::write_to_string_formatted() const
{
        xmlChar *doc_txt_ptr = NULL;
        int doc_txt_len = 0;
        xmlKeepBlanksDefault(0); 
        xmlDocDumpFormatMemory(impl->doc, 
                               &doc_txt_ptr, 
                               &doc_txt_len, 
                               1);

        stringstream out_string;
        out_string << doc_txt_ptr;
        free(doc_txt_ptr);
        return out_string.str(); 
}

NS_MIA_END
