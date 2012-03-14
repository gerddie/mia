#!/bin/env python 

import sys
import time
import calendar
import string
from lxml import etree
import re
from os.path import walk
from fnmatch import fnmatch

modules = {'miareadxml' : [0, '', 'none://miareadxml.py' ],
           'miawritprogram' : [0, '', 'none://miawritprogram.py' ]
           }


from miareadxml import parse_file
from miawritprogram import make_sec_ancor
from miawritprogram import get_section
from miawritprogram import get_plugins
from miawritprogram import xml_namespace
from miawritprogram import xmlns
from miawritprogram import NSMAP


def create_text_node(tag, text):
    node = etree.Element(tag)
    node.text = text
    return node


def get_date_string():
    lt = time.localtime(time.time())
    return "%d %s %d"% (lt.tm_mday, calendar.month_name[lt.tm_mon], lt.tm_year)


class CPatternCollector:
    def __init__(self, pattern):
        self.pattern = pattern
        self.files=[]
        
      
def collect_files(arg, dirname, names):
    for f in names:
        if fnmatch(f, arg.pattern):
            arg.files.append(dirname + "/" + f)
            
def find_files(root, pattern): 
    arg = CPatternCollector(pattern)
    walk(root, collect_files, arg)
    return arg.files 

class CProgramSection:
    def __init__(self, node=None, name=None):
        self.programs=[]
        if node is not None: 
            self.name=node.get("name")
            self.description = node.iter("description")
        else:
            self.name=name
            self.description = None

def read_section_file(filename):
    file=open(filename, "r")
    stringstree = file.read()
    root = etree.XML(stringstree)

    program_sections = {}

    if root.tag != "outline": 
        raise ValueError("Expected tag 'outline' not found. sections.xml is not the expected file.")
    for n in root:
        section = CProgramSection(node=n)
        program_sections[section.name] = section
    return program_sections
    

# read all xml files 
files = find_files(".", "mia-*.xml")
descriptions=[]

for f in files:
    try:
        d = parse_file(f)
        descriptions.append(d)
    except ValueError as e:
        print e

program_sections = read_section_file("sections.xml")
plugin_types = {}




#sort the descriptions 
for d in descriptions:
   if not program_sections.has_key(d.section):
      program_sections[d.section] = CProgramSection(name=d.section)
      print "Warning: Program '%s' is listed in section '%s', but this section is not described in doc/sections.xml" % (d.name, d.section)
   program_sections[d.section].programs.append(d)

   for h in d.handlers.keys():
      k = d.handlers[h].name
      if not plugin_types.has_key(k):
         plugin_types[k] = d.handlers[h]
      else:
         plugin_types[k].merge_users(d.handlers[h].users)




#Now convert to linuxdoc format - these are also XML files 
prog_xml = etree.Element("chapter", nsmap=NSMAP)
prog_xml.set(xmlns + "id", "Programs")
title = create_text_node("title", "Program Reference")
prog_xml.append(title)

section_keys = program_sections.keys()
section_keys.sort()
for s in section_keys: 
    prog_xml.append(get_section(s, program_sections[s]))




programs_xml = etree.tostring(prog_xml, pretty_print=True)
prog_file = open("program.xml", "w")
prog_file.write(programs_xml)
prog_file.close()



plug_xml = etree.Element("chapter", id="plugins", nsmap=NSMAP)
title = create_text_node("title", "Plugin Reference")
plug_xml.append(title)

plugin_types_keys = plugin_types.keys()
plugin_types_keys.sort()
for s in plugin_types_keys: 
   plug_xml.append(get_plugins(s, plugin_types[s]))


plugins_xml = etree.tostring(plug_xml, pretty_print=True)
plug_file = open("plugins.xml", "w")
plug_file.write(plugins_xml)
plug_file.close()



