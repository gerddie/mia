#!/bin/env python 

import sys
import re
from lxml import etree

sys.dont_write_bytecode = True

modules = {'miareadxml' : [0, '', 'none://miareadxml.py' ]
           }
from miareadxml import parse_file

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

def create_text_node(tag, text):
   node = etree.Element(tag)
   node.set_text(text)

def get_program(prog):
   progsec = etree.Element("section", id=make_ancor(prog.name))
   title = create_text_node("title", prog.name)
   descr = create_text_node("para", prog.description)
   basic_usage = create_text_node("code", prog.description)
   progsec.append(title)
   progsec.append(descr)
   return progsec
   

def get_section(name, programs):
   section = etree.Element("section", id=make_ancor(name))
   title = etree.Element("title")
   title.set_text(name)
   section.append(title)
   for p in sec.programs:
      section.append(get_program(p))
   return section
      
def get_plugin(plugin):
   section = etree.Element("section", id=make_ancor(name))

def get_plugins(name, plugins):
   section = etree.Element("section", id=make_ancor(name))
   title = create_text_node("title", name)
   section.append(title)
   for p in plugins:
      section.append(get_plugin(p))
   return section 
                 
   

files = find_files(".", "mia-*.xml")

descriptions=[]

for f in files:
   d = parse_file(f)
   descriptions.append(d)

program_sections = {}


plugin_types = {}

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


plug_xml = etree.Element("chapter", id="plugins")
title = create_text_node("title", "Plugin Reference")
plug_xml.append(title)

for s in plugin_types.keys(): 
   plug_xml.append(get_plugins(s, program_sections[s]))
   



