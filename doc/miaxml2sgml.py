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
from miawritprogram import get_section


def create_text_node(tag, text):
   node = etree.Element(tag)
   node.text = text
   return node


def get_date_string():
    lt = time.localtime(time.time())
    return "%d %s %d"% (lt.tm_mday, calendar.month_name[lt.tm_mon], lt.tm_year)


def make_ancor(text):
   """remove spaces and hyphens from the input string""" 
   return re.sub("[ -]", "", text)

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

# read all xml files 
files = find_files(".", "mia-*.xml")
descriptions=[]

for f in files:
    try:
        d = parse_file(f)
        descriptions.append(d)
    except ValueError as e:
        print e

program_sections = {}
plugin_types = {}


#sort the descriptions 
for d in descriptions:
   if not program_sections.has_key(d.section):
      program_sections[d.section] = []
   program_sections[d.section].append(d)

   for h in d.handlers:
      if not plugin_types.has_key(h.name):
         plugin_types[h.name] = h
      plugin_types[h.name].append_user(d.name)
   


#Now convert to linuxdoc format - these are also XML files 
prog_xml = etree.Element("chapter", id="programs")
title = create_text_node("title", "Program Reference")
prog_xml.append(title)

for s in program_sections.keys(): 
    prog_xml.append(get_section(s, program_sections[s]))

programs_xml = etree.tostring(prog_xml, pretty_print=True)
prog_file = open("program.xml", "w")
prog_file.write(programs_xml)
prog_file.close()



#plug_xml = etree.Element("chapter", id="plugins")
#title = create_text_node("title", "Plugin Reference")
#plug_xml.append(title)
#
#for s in plugin_types.keys(): 
#   plug_xml.append(get_plugins(s, program_sections[s]))



