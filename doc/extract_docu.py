#!/usr/bin/python 
#
# Copyright (C) 2011 Gert Wollny <gw.fossdev@gmail.com>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

import sys
import re
import string 
from fnmatch import fnmatch
from os.path import walk



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


class PluginSection:
   def __init__(self):
      self.body = []
      self.subsections = {}
      self.unspecified = []
      
   def append_to_body(self, text_block): 
      self.body.extend(text_block)
      
   def append_plugin(self, text_block): 
      for l in text_block: 
         e = re.search("subsection\{(.*)\}", l)
         if e:
            self.subsections[e.group(1)] = text_block
            return 
      self.unspecified.extend(text_block)

   def write(self, output): 
      for l in self.body:
         output.write(l)
      for k in sorted(self.subsections.keys()):
         sys.stderr.write("  {}\n".format(k))
         for l in self.subsections[k]:
            output.write(l)

      for l in self.unspecified:
         output.write(l)
    
       
# this will be a list of lists containing all the documentation comment blocks
comment_blocks = []


root= sys.argv[1]
output_filename = sys.argv[2]
files = find_files(root, "*.cc")

# read all files and extract comment blocks 
for f in files: 
    infile = open(f,"r")
    lines = infile.readlines()

    current_block = []
    is_text = False

    for l in lines: 
        if is_text: 
            e = re.search("(?<=LatexEnd)", l)
            if not e: 
                current_block.append(l)
            else:
                comment_blocks.append(current_block)
                is_text = False
        else:
            m = re.search("LatexBegin", l)
            if not m: 
                continue
            else:

                current_block = [l]
                is_text = True
               
    if is_text:
        print "No LatexEnd in {}".format(f)
        exit(1)
    infile.close()

sections = {}

for b in comment_blocks: 
    m = re.search(" *LatexBegin([a-zA-Z]*){(.*)}", b[0])
   
    key = m.group(2)
    if not sections.has_key(key): 
        sections[key] = PluginSection()
    
    
    if m.group(1) == "Section":
        sections[key].append_to_body(b[1:])
        
    if m.group(1) == "Plugin":
        sections[key].append_plugin(b[1:])

output_file = open(output_filename,"w")

output_file.write("\\chapter{Plug-ins}\n")
output_file.write("In this chapter the plug-ins are described that are provided by the library.\n") 
output_file.write("\\label{ch:plugins}")

for s in sorted(sections.keys()): 
   sys.stderr.write("{}\n".format(s))
   output_file.write("\\section{{{0}}}".format(s)) 
   sections[s].write(output_file)
