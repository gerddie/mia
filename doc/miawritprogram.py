# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

from lxml import etree
import re




modules = {'miareadxml' : [0, '', 'none://miareadxml.py' ]}

from miareadxml import get_text_node_simple
from miareadxml import get_dict_table
from miareadxml import make_sec_ancor
from miareadxml import xml_namespace
from miareadxml import xmlns

NSMAP={"xml" : xml_namespace }

def make_section_root_node(tag, name):
    secid = make_sec_ancor("Sec", name)
    section = etree.Element(tag)
    section.set(xmlns + "id", secid)
    section.set("xreflabel", name)
    title = etree.Element("title")
    title.text = name
    section.append(title)
    return section

def get_text_node(tag, renderas, text):
    node = etree.Element(tag)
    node.set("renderas", renderas)
    node.text = text
    return node


def get_bridgehead(text):
    return get_text_node("bridgehead", "sect3", text)
   
def get_synopsis(program):
    synopsis = etree.Element("cmdsynopsis")
    synopsis.append(get_text_node_simple("command", program.name))
    
    # add required options 
    for g in program.option_groups:
        for o in g.options:
            if o.required:
                node = etree.Element("arg", choice="req")
                node.text = "-" + o.short + " "
                node.append(get_text_node_simple("replaceable",  o.type))
                synopsis.append(node)
    
    # add standard option hint 
    node = etree.SubElement(synopsis, "arg", rep="repeat")
    node.append(get_text_node_simple("replaceable",  "options"))
    
    #add free parameter if exists
    if not program.FreeParams is None:
           node = etree.SubElement(synopsis, "arg", rep="repeat")
           plugin = etree.SubElement(node, "replaceable")
           etree.SubElement(plugin, "xref", linkend=make_sec_ancor("SecPlugintype", program.FreeParams))
    
    return synopsis

   
def get_option_descr(option):
    entry = etree.Element("varlistentry")

    option.write_xml(entry)
    
    return entry

def get_text_node_with_doi_links(node, text):
    doi_split = re.split(r'(\[[^\]]*\]\([^)]*\))', text)
    reg = re.compile(r'\[([^\]]*)\]\(([^)]*)\)')
    subnode = None
    for d in doi_split:
        if len(d) == 0:
            continue
        if d[0] == "[":
            link = reg.split(d)
            subnode = etree.SubElement(node, 'para', role='citation')
            cite = etree.SubElement(subnode, 'ulink')
            cite.set('url',link[2])
            cite.text = link[1]
        else:
            if subnode is None:
                node.text = d
            else:
                subnode.tail = d
    return node

def get_text_node_without_doi_links(node, text):
    doi_split = re.split(r'(\[[^\]]*\]\([^)]*\))', text)
    reg = re.compile(r'\[([^\]]*)\]\(([^)]*)\)')
    subnode = None
    for d in doi_split:
        if len(d) == 0:
            continue
        if d[0] == "[":
            link = reg.split(d)
            subnode = etree.SubElement(node, 'quote')
            subnode.set('role', 'citation')
            subnode.text = link[1]
        else:
            if subnode is None:
                node.text = d
            else:
                subnode.tail = d
    return node


def get_program(program):
    section = make_section_root_node("section", program.name)
    section.append(get_bridgehead("Sysnopis:"))
    section.append(get_synopsis(program))
    section.append(get_bridgehead("Description:"))
    node = etree.Element('para', role='description')
    get_text_node_with_doi_links(node, program.description)
    section.append(node)
    section.append(get_bridgehead("Options:"))
    
    for g in program.option_groups:
        if len(g.name) > 0:
            section.append(get_bridgehead(g.name))
            
        if len(g.options) > 0:
            varlist = etree.Element("variablelist", spacing="compact")
            for o in g.options:
                varlist.append(get_option_descr(o))
            section.append(varlist)

    if not program.Example is None:
        if not program.Example.text is None:
            section.append(get_bridgehead("Example:"))
            section.append(get_text_node_simple("para", program.Example.text))
            for c in program.Example.code:
                if not c.text is None: 
                    screen = etree.SubElement(section, "screen")
                    screen.text = program.name + " " + c.text
        else:
            print( "Warning: {} doesn't provide example text".format (program.name)  )
    else:
        print( "Warning: {} doesn't provide an example".format (program.name)  )

    section.append(get_bridgehead("Author(s):"))
    section.append(get_text_node_simple("para", program.author))
    
    return section 

def translate_descr(section, description):
    descrpara = etree.SubElement(section, "para", role="sectiondescr")
    for d in description:
        para = etree.SubElement(descrpara, "para", role="sectiondescr")
        para.text = d.text
        for l in d.iter("link"):
            link = etree.SubElement(para, "xref", linkend=make_sec_ancor(l.get("key"), l.text))
            link.tail = l.tail
    return descrpara

def get_section(name, sect):
    section = make_section_root_node("section", name)
    if sect.description is not None:
        descr = translate_descr(section, sect.description)
    else:
        print( "Warning: Section ", name, " doesn't have a description" )

    para = etree.SubElement(section, "para", role="sectiontoc")
        
    sect = sorted(sect.programs, key=lambda p: p.name )
    for program in sect:
        subpara = etree.SubElement(para, "para", role="sectiontoc")
        etree.SubElement(subpara, "xref", linkend=program.anchor)
        descr = etree.SubElement(subpara, "para", role="progdescr")
        get_text_node_without_doi_links(descr, program.description)
        section.append(get_program(program))
    return section


def get_plugin(plugin):
   result = etree.Element("para", role="plugin")
   plugin.write_xml(result)
   return result

def get_plugins(name, handler):
#   print( name )
   section = make_section_root_node("section", "Plugin type: " + name)
   para = etree.SubElement(section, "para", role="plugdescr")
   para.text = handler.description 
   head = get_bridgehead("Plugins:")
   section.append(head)
   plugshort = etree.SubElement(section, "para", role="pluginlist")
   
   for p in handler.plugins:
       etree.SubElement(plugshort, "xref", linkend=p.ancor, endterm=p.altancor )
       section.append(get_plugin(p))
      
   head = get_bridgehead("Plugin consumers:")
   section.append(head)
   para = etree.SubElement(section, "para", role="consumer")
   handler_users = []
   for u in handler.users:
       handler_users.append(u)
   handler_users = sorted(handler_users)
   for u in handler_users:
      etree.SubElement(para, "xref", linkend=u)

   return section
