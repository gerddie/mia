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

class Section:
   def __init__(self):
       self.body = []
       self.subsections = []

   def append_to_body(self, text_block): 
       self.body.extend(text_block[1:])

   def append_plugin(self, text_block): 
       self.subsections.extend(text_block[1:])

   def write(self): 
    for l in self.body:
        print l, 
    for l in self.subsections:
        print l, 
    
       
# this will be a list of lists containing all the documentation comment blocks
comment_blocks = []

# read all files and extract comment blocks 
for f in sys.argv[1:]: 
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

print len(comment_blocks)

sections = {}

for b in comment_blocks: 
    m = re.search(" *LatexBegin([a-zA-Z]*)({.*})", b[0])
   
    key = m.group(2)
    if not sections.has_key(key): 
        sections[key] = Section()
    
    
    if m.group(1) == "Section":
        sections[key].append_to_body(b)
        
    if m.group(1) == "Plugin":
        sections[key].append_plugin(b)


for s in sections.keys(): 
    print "\\section{{{0}}}".format(s), 
    sections[s].write()
