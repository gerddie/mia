#!/bin/env python 

import sys


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

files = find_files(".", "mia-*.xml")

sections = []


X=parse_file(sys.argv[1])

