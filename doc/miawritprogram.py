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
                

def get_program(program):
    section = make_section_root_node("section", program.name)
    section.append(get_bridgehead("Sysnopis:"))
    section.append(get_synopsis(program))
    section.append(get_bridgehead("Description:"))
    section.append(get_text_node_simple("para", program.description))
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
                    screen = etree.Element("screen")
                    screen.text = program.name + " " + c.text
                    section.append(screen)
        else:
            print "%s doesn't provide example text" % (program.name) 
    else:
        print "%s doesn't provide an example" % (program.name) 

    return section 

def get_section(name, sect):
    section = make_section_root_node("section", name)
    para = etree.SubElement(section, "para", role="sectiontoc")
    for program in sect:
        subpara = etree.SubElement(para, "para", role="sectiontoc")
        etree.SubElement(subpara, "xref", linkend=program.anchor)
        descr = etree.SubElement(subpara, "para", role="progdescr")
        descr.text = program.description
        section.append(get_program(program))
    return section


def get_plugin(plugin):
   result = etree.Element("para", role="plugin")
   plugin.write_xml(result)
   return result

def get_plugins(name, handler):
#   print name
   section = make_section_root_node("section", "Plugin type: " + name)
   para = etree.SubElement(section, "para", role="plugdescr")
   para.text = handler.description 
   head = get_bridgehead("Plugin consumers:")
   section.append(head)
   para = etree.SubElement(section, "para", role="consumer")
   for u in handler.users:
      etree.SubElement(para, "xref", linkend=u)

   head = get_bridgehead("Plugins:")
   section.append(head)
   for p in handler.plugins:
      section.append(get_plugin(p))
   return section
