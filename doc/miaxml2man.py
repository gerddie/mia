#!/bin/env python 

from lxml import etree
import re
import time
import calendar
import sys
import string
import htmlentitydefs

# supported tags: 
#   program      main tag 
#   name         text: program name
#   section      text: program type
#   description  text: basic description of the program 
#   basic_usage  text: generic call 
#   group        tag name: option group name  
#     option     tag short: short option 
#                tag long:  long option 
#                tag required: 1 if required argument 
#                tag default: default option value
#                text: help 
#  Example       text: example descripton 
#    Code        text: example text without program name 


class CTextNode: 
    def __init__(self, node, expect = None):
        if not expect is None and node.tag != expect:
            raise ValueError("expected '%s' got '%s'" % (expect, node.tag))

        self.entry = node.tag
        self.text  = node.text 

class COption(CTextNode):
    def __init__(self, node):
        CTextNode.__init__(self, node, "option")
        
        self.short = node.get("short")
        self.long =  node.get("long")
        self.required = int(node.get("required")) 
        self.default = node.get("default")
        
        
class CExample(CTextNode):
    def __init__(self, node):
        CTextNode.__init__(self, node, "Example")
        self.code = []

        for child in node:
            if child.tag == "Code": 
                self.code.append(CTextNode(child, "Code"))
            else:
                print "unexpected subnode '%s' in example"% (child.tag)


class CGroup:
    def __init__(self, node):
        if node.tag != "group":
            raise ValueError("expected 'group' got '%s'" % (node.tag))

        self.entry = node.tag
        self.name  = node.get("name")

        self.options = []
        for child in node:
            if child.tag == "option": 
                self.options.append(COption(child))
            else:
                print "unexpected subnode '%s' in 'group'"% (child.tag)

def get_text_element(root, name):
        xname = etree.SubElement(root, name)
        if xname is None:
            raise ValueError("Program description misses the tag %s"% (name))
        return xname.text

class CDescription: 
    def __init__(self, node):
        self.Example = None 
        self.option_groups = []
        
        for n in node:
            if n.tag == 'name':
                self.name = n.text
            elif n.tag == 'section':
                self.section = n.text
            elif n.tag == 'description':
                self.description = n.text
            elif n.tag == 'basic_usage':
                self.basic_usage = n.text
            elif n.tag == 'group': 
                self.option_groups.append(CGroup(n))
            elif n.tag == 'Example': 
                self.Example = CExample(n)
            else: 
               print "unknown tag '%s'"% (n.tag)
        
def parse_file(xmlpath):
    file=open(xmlpath, "r")
    stringstree = file.read()
    root = etree.XML(stringstree)
    if root.tag != "program": 
        raise ValueError("Expected tag 'program' not found. This is not a mia program descripion.")
    
    return CDescription(root)

def get_date_string():
    lt = time.localtime(time.time())
    return "%d %s %d"% (lt.tm_mday, calendar.month_name[lt.tm_mon], lt.tm_year)

#taken from http://effbot.org/zone/re-sub.htm#unescape-html
    
def unescape(text):
    def fixup(m):
        text = m.group(0)
        if text[:2] == "&#":
            # character reference
            try:
                if text[:3] == "&#x":
                    return unichr(int(text[3:-1], 16))
                else:
                    return unichr(int(text[2:-1]))
            except ValueError:
                pass
        else:
            # named entity
            try:
                text = unichr(htmlentitydefs.name2codepoint[text[1:-1]])
            except KeyError:
                pass
        return text # leave as is
    return re.sub("&#?\w+;", fixup, text)

def escape_dash(text): 
    return re.sub(r'-', r'\-', text) 

def clean (text):
    text = unescape(text)
    return escape_dash(text) 

def write_man_file(descr):
    name = escape_dash(descr.name)
    print ".TH %s 1 \"%s\" \"git-head\"  \"USER COMMANDS\"" %(name, get_date_string())
    print ".SH NAME"
    print name
    print ".SH SYNOPSIS"
    print ".B %s"% (clean(descr.basic_usage))
    print ".SH DESCRIPTION"
    print ".B %s"% (name)
    print descr.description
    print ".SH OPTIONS"
    for g in descr.option_groups:
        if len(g.name) > 0:
            print ".SS %s"% (g.name)
        for o in g.options:
            print ".TP"
            if len(o.short) > 0:
                short = "\-" + o.short
            else:
                short = "  "; 

            if o.required:
                print "%s \-\-%s[required]"% (short, o.long)
            else:
                print "%s \-\-%s=%s"% (short, o.long, escape_dash(o.default))
            print o.text
    if descr.Example.text is not None and len(descr.Example.text) > 0:
            print ".SH EXAMPLES"
            print clean(descr.Example.text)
            for c in descr.Example.code:
                print ".TP"
                print "%s %s" % (name, clean(c.text))
    
    print ".SH COPYRIGHT"
    print "This software is copyright (c) Gert Wollny et al."
    print "It comes with  ABSOLUTELY NO WARRANTY and you may redistribute it"
    print "under the terms of the GNU GENERAL PUBLIC LICENSE Version 3 (or later)."



X=parse_file(sys.argv[1])
write_man_file(X)
