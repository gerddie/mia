#!/bin/env python 
#
# Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#
# This program is used to read the mia-*.xml files that result from running 
# the mia-* programs with the --help-xml option and convert the information 
# into two files program.xml and plugins.xml that are included into the userref.xml 
# file which forms a Docbook 5.0 document then ready to be feeded to xslproc 
# for the creation of the HTML reference. 
#


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
    def __init__(self, node=None, name=None, idx=0):
        self.programs=[]
        self.sortindex = idx
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
    index = 0
    for n in root:
        section = CProgramSection(node=n, idx=index)
        program_sections[section.name] = section
        index = index+1
    return program_sections
    

rootpath = sys.argv[1]
outpath = sys.argv[2]
#
# read all xml files created by the command line tools 
#
files = find_files(outpath, "mia-*.xml")
descriptions=[]

for f in files:
    try:
        d = parse_file(f)
        descriptions.append(d)
    except ValueError as e:
        print e


#
# Read the program section description, this also provides the general structure  
# of the program reference
#
program_sections = read_section_file(rootpath + "/sections.xml")
index = len(program_sections)

#
# Sort the programs into the sections and read the plug-in types 
#
plugin_types = {}
for d in descriptions:
   if not program_sections.has_key(d.section):
      program_sections[d.section] = CProgramSection(name=d.section, idx=index)
      index = index + 1
      print "Warning: Program '%s' is listed in section '%s', but this section is not described in doc/sections.xml" % (d.name, d.section)
   program_sections[d.section].programs.append(d)

   for h in d.handlers.keys():
      k = d.handlers[h].name
      if not plugin_types.has_key(k):
         plugin_types[k] = d.handlers[h]
      else:
         plugin_types[k].merge_users(d.handlers[h].users)



#
# Now convert to the docbook 5.0 format - these are also XML files 
# The output can then be parsed and converted by xsltproc 
#
prog_xml = etree.Element("chapter", nsmap=NSMAP)
prog_xml.set(xmlns + "id", "Programs")
title = create_text_node("title", "Program Reference")
prog_xml.append(title)


sorted_sections = {}

for s in program_sections.keys():
    sorted_sections[program_sections[s].sortindex] = s

sorted_sections_keys = sorted_sections.keys()
sorted_sections_keys.sort()

for i in sorted_sections_keys:
    s = sorted_sections[i]
    prog_xml.append(get_section(s, program_sections[s]))

programs_xml = etree.tostring(prog_xml, pretty_print=True)
prog_file = open(outpath + "/program.xml", "w")
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
plug_file = open(outpath + "/plugins.xml", "w")
plug_file.write(plugins_xml)
plug_file.close()



