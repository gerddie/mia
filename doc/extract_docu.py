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


sections = {}

for f in sys.argv[1:]: 
    infile = open(f,"r")
    lines = infile.readlines()

    is_text = False
    current_section = "" 

    for l in lines: 
        if is_text: 
            e = re.search("(?<=LatexEnd)", l)
            if not e: 
                sections[current_section].append(l)
            else:
                is_text = False
        else:
            m = re.search("(?<=LatexBeginPlugin{)[a-zA-Z 0-9]*", l)
            if not m: 
                continue
            else:
                current_section = m.group(0)
                is_text = True
            if not sections.has_key(current_section):
                sections[current_section] = []
                    
    if is_text:
        print "No LatexEnd in {}".format(f)
        exit(1)
    infile.close()

for s in sections.keys(): 
    print "\\section{{{0}}}".format(s), 
    for l in sections[s]:
        print l, 
        

    

    
    
