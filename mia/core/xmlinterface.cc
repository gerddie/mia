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
#include <cstdarg>
#include <map>
#include <stack> 

#include <string>

#include <mia/core/xmlinterface.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

using std::string;
using std::stringstream;
using std::make_shared; 
using std::vector;
using std::map;
using std::runtime_error; 
using std::stack;
using std::pair;
using std::make_pair; 
using std::unique_ptr; 

struct CXMLElementImpl {
        CXMLElementImpl(const char *name);
        ~CXMLElementImpl();

        string m_name;
	std::string m_content; 
	map<std::string, std::string> m_attributes;
        vector<CXMLElement::Pointer> m_children;

	xmlNodePtr write_to_node();
};


struct CXMLErrorHandler {
	CXMLErrorHandler(const std::string& context, string& last_error);
	~CXMLErrorHandler(); 
	static void error_handler(void *ctx, const char *msg, ...);
private:
	string m_context;
	string& m_last_error;

}; 


CXMLErrorHandler::CXMLErrorHandler(const std::string& context, string& error_msg):
	m_context(context),
	m_last_error(error_msg)
{
	xmlSetGenericErrorFunc(this, (xmlGenericErrorFunc)CXMLErrorHandler::error_handler);
}

CXMLErrorHandler::~CXMLErrorHandler()
{
	xmlSetGenericErrorFunc(NULL, NULL);
}


void CXMLErrorHandler::error_handler(void *ctx, const char *msg, ...)
{
	CXMLErrorHandler* self = reinterpret_cast<CXMLErrorHandler*>(ctx); 
	
	const int TMP_BUF_SIZE = 1024; 
	char s[TMP_BUF_SIZE];
	va_list arg_ptr;
	
	va_start(arg_ptr, msg);
	vsnprintf(s, TMP_BUF_SIZE, msg, arg_ptr);
	va_end(arg_ptr);

	stringstream error_msg;
	error_msg << self->m_context << ":" << s; 
	self->m_last_error = error_msg.str(); 
}


CXMLElementImpl::CXMLElementImpl(const char *name):
	m_name(name)
{
}

CXMLElementImpl::~CXMLElementImpl()
{
}

xmlNodePtr CXMLElementImpl::write_to_node()
{
	auto this_node = xmlNewNode(nullptr, BAD_CAST m_name.c_str());
	// add attributes
	for (auto a: m_attributes) 
		xmlNewProp(this_node, BAD_CAST a.first.c_str(),  BAD_CAST a.second.c_str());
	
	// add content
	// missing: add encoding special chars
	if (!m_content.empty()) {
		xmlNodePtr text = xmlNewText(BAD_CAST m_content.c_str());
		xmlAddChild(this_node, text);
	}

	// add children 
	for (auto child: m_children) {
		auto child_node = child->impl->write_to_node();
		xmlAddChild(this_node, child_node );
	}

	return this_node;
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
        impl->m_children.push_back(result);
        return result; 
}

void CXMLElement::set_child_text(const std::string& value)
{
	assert(impl); 
	impl->m_content += value;
}

void CXMLElement::set_attribute(const char *name, const std::string& value)
{
	assert(impl); 
	impl->m_attributes[name] = value; 
}

const string& CXMLElement::get_name() const
{
	assert(impl); 
	return impl->m_name; 
}

const string CXMLElement::get_attribute(const string& name) const
{
	auto ip = impl->m_attributes.find(name);
	if (ip != impl->m_attributes.end()) {
		return ip->second; 
	}
	return string(); 
	
}

const vector<CXMLElement::Pointer>& CXMLElement::get_all_children() const
{
	assert(impl);
	return impl->m_children; 
}

vector<CXMLElement::Pointer> CXMLElement::get_children(const char *name) const
{
	assert(impl);
	vector<CXMLElement::Pointer> result; 
	for (auto c :  impl->m_children) {
		if (c->get_name() == name)
			result.push_back(c); 
	}
	return result; 
}

const std::string& CXMLElement::get_content() const
{
	return impl->m_content;
}
       


CXMLElement::Pointer CXMLDocument::create_root_node(const char *name)
{
	m_root = CXMLElement::Pointer(new CXMLElement(name));
	return m_root; 
}

CXMLElement::Pointer CXMLDocument::get_root_node() const
{
	return m_root; 
}


CXMLDocument::CXMLDocument()
{	
}

CXMLDocument::CXMLDocument(const char *init)
{
	if (!read_from_string(init)) {
		throw runtime_error(m_last_error);
	}
}


struct xmlDocDeleter {
	void operator() (xmlDoc *p) {
		xmlFreeDoc(p);
	}
}; 

bool CXMLDocument::read_from_string(const char *init)
{
	CXMLErrorHandler error_handler("XML Reader", m_last_error);

	unique_ptr<xmlDoc,  xmlDocDeleter> doc(xmlParseDoc(reinterpret_cast<const xmlChar *>(init)));

	if (!doc)
		return false; 

	auto root = xmlDocGetRootElement(doc.get());

	if (!root)
		return false;
	
	m_root  = make_shared<CXMLElement>(reinterpret_cast<const char*>(root->name));  

	{
		xmlChar *content = xmlNodeGetContent(root);
		if (content) {
			const char *c = reinterpret_cast<const char *>(content);
			m_root->set_child_text(c);
			xmlFree(content); 
		}
		// read attributes
		xmlAttr* attribute = root->properties;
		while(attribute) {
			const char* name = reinterpret_cast<const char*>(attribute->name); 
			xmlChar* value = xmlNodeListGetString(root->doc, attribute->children, 1);
			const char* v = reinterpret_cast<const char*>(value);
			m_root->set_attribute(name, v); 
			xmlFree(value); 
			attribute = attribute->next;
		}

	}

	
	std::stack<pair<CXMLElement::Pointer, xmlNodePtr> > s;
	s.push(make_pair(m_root, root->children));

	cvdebug() << "Parse document\n"; 
	while(!s.empty() && m_last_error.empty()) {
		auto current_level = s.top();
		s.pop();

	
		for (auto n = current_level.second; n; n = n->next) {
			if (n->type != XML_ELEMENT_NODE)  
				continue; 

			cvdebug() << s.size() << "  Got node '" << n->name
				  << "' of type " << n->type << "\n"; 
					
			
			auto name = reinterpret_cast<const char *>(n->name); 
			auto node = current_level.first->add_child(name);

			cvdebug() << s.size() << "Add child node to "
				 << current_level.first->get_name() << ": " << name << "\n"; 
			
			// read content
			xmlChar *content = xmlNodeGetContent(n);
			if (content) {
				const char *c = reinterpret_cast<const char *>(content);
				cvdebug() << s.size() << "  Got content '" << c << "'\n"; 
				node->set_child_text(c);
				xmlFree(content); 
			}
			
			// read attributes
			xmlAttr* attribute = n->properties;
			while(attribute) {
				const char* name = reinterpret_cast<const char*>(attribute->name); 
				xmlChar* value = xmlNodeListGetString(n->doc, attribute->children, 1);
				const char* v = reinterpret_cast<const char*>(value);
				cvdebug() << s.size()<< "  Got attribute '" << name
					  << "', value='" << value << "'\n";
				node->set_attribute(name, v); 
				xmlFree(value); 
				attribute = attribute->next;
			}

			if (n->children) {
				s.push(make_pair(node, n->children));
			}
		}
	}

	return m_last_error.empty(); 
}


std::string CXMLDocument::write_to_string(const char *encoding, bool formatted) const
{
	string error_msg; 
	CXMLErrorHandler error_handler("XML Writer", error_msg);
	
	xmlChar *doc_txt_ptr = NULL;
        int doc_txt_len = 0;

	unique_ptr<xmlDoc,  xmlDocDeleter> doc(xmlNewDoc(BAD_CAST "1.0"));
	
	xmlNodePtr root = m_root->impl->write_to_node();
	xmlDocSetRootElement(doc.get(), root);
	
        xmlKeepBlanksDefault(0);
	
	if (encoding) 
		xmlDocDumpFormatMemoryEnc(doc.get(), 
					  &doc_txt_ptr, 
					  &doc_txt_len,
					  encoding,
					  formatted ? 1 : 0);
	else
		xmlDocDumpFormatMemory(doc.get(), 
                               &doc_txt_ptr, 
                               &doc_txt_len, 
                               formatted ? 1 : 0);
	
	stringstream out_string;
        out_string << doc_txt_ptr;
        free(doc_txt_ptr);

	if (!error_msg.empty())
		throw runtime_error(error_msg); 
	
        return out_string.str(); 	

}

NS_MIA_END
