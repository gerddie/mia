from lxml import etree
import re

def make_sec_ancor(key, text):
   """remove spaces and hyphens from the input string""" 
   return key + re.sub("[ -]", "", text)

def make_section_root_node(tag, name):
    secid = make_sec_ancor("Sec", name)
    section = etree.Element(tag, id=secid, xreflabel=name)
    title = etree.Element("title")
    title.text = name
    section.append(title)
    return section

def get_text_node(tag, renderas, text):
    node = etree.Element(tag, renderas=renderas)
    node.text = text
    return node

def get_text_node_simple(tag,  text):
    node = etree.Element(tag)
    node.text = text
    return node

def get_bridgehead(text):
    return get_text_node("bridgehead", "sect5", text)
   
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
    node = etree.Element("arg")
    node.append(get_text_node_simple("replaceable",  "options"))
    synopsis.append(node)
    
    # currently missing are the free parameters 
    return synopsis

def get_dict_table(dictionary):
    entry = etree.Element("informaltable", frame="none")
    tgroup = etree.Element("tgroup", cols="2", colsep="0", rowsep="0")
    tbody = etree.Element("tbody")
    for d in dictionary.keys(): 
        row = etree.Element("row")
        e = etree.Element("entry", align="left", valign="top")
        e.text = d + ":"
        row.append(e)
        e = etree.Element("entry", align="left", valign="top")
        e.text = dictionary[d]
        row.append(e)
        tbody.append(row)
    tgroup.append(tbody)
    entry.append(tgroup)
    return entry
    
def get_option_descr(option):
    entry = etree.Element("varlistentry")
    
    # create the terminal text 
    termtext = "-"
    if len(option.short) > 0: 
        termtext = termtext + option.short + ", -"
    termtext = termtext + "-" + option.long + "="
    if option.required: 
        termtext = termtext + "(required)"
    elif len(option.default)>0:
        termtext = termtext + option.default
    
    entry.append(get_text_node_simple("term",  termtext))
    item =  etree.Element("listitem")
    para = get_text_node_simple("para", option.text)
    if len(option.dict) > 0:
        opttable = get_dict_table(option.dict)
        para.append(opttable)
    item.append(para)
    entry.append(item)
    return entry
                

def get_program(program):
    section = make_section_root_node("sect2", program.name)
    section.append(get_bridgehead("Sysnopis:"))
    section.append(get_synopsis(program))
    section.append(get_bridgehead("Description:"))
    section.append(get_text_node_simple("para", program.description))
    section.append(get_bridgehead("Options:"))
    
    for g in program.option_groups:
        if len(g.name) > 0:
            section.append(get_bridgehead(g.name))
            
        if len(g.options) > 0:
            varlist = etree.Element("variablelist")
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
    section = make_section_root_node("sect1", name)
    for program in sect:
        section.append(get_program(program))
    return section
                      
    
    
    




