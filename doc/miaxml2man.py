#!/usr/bin/env python 
#
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

# this program is used to translate the XML files obtained by running a mia-* program 
# into a Unix man page. 

import sys
import time
import calendar
import string

python_version = sys.version_info.major
if python_version >= 3:
    from html.entities import name2codepoint
else:
    from htmlentitydefs  import name2codepoint

import re

sys.dont_write_bytecode = True

modules = {'miareadxml' : [0, '', 'none://miareadxml.py' ]
           }


from miareadxml import parse_file

def get_date_string():
    lt = time.localtime(time.time())
    return "{} {} {}".format (lt.tm_mday, calendar.month_name[lt.tm_mon], lt.tm_year)

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
                text = unichr(name2codepoint[text[1:-1]])
            except KeyError:
                pass
        return text # leave as is
    return re.sub("&#?\w+;", fixup, text)

def escape_dash(text): 
    return re.sub(r'-', r'\-', text) 

def clean (text):
    text = unescape(text)
    return escape_dash(text)

def print_description(text):
    doi_split = re.split(r'(\[[^\]]*\]\([\w\s/:.)]*\))', text)
    reg = re.compile(r'\[([^\]]*)\]\(([\w/:.)]*)\)')
    for d in doi_split: 
        if d[0] == "[":
            link = reg.split(d)
            print("")
            print(".RS")
            print(link[1])
            print(".I {}".format(link[2]))
            print(".RE")
        else:
            print(d)

def write_man_file(descr):
    name = escape_dash(descr.name)
    print(".TH {} 1 \"v{}\"  \"USER COMMANDS\"".format(escape_dash(descr.name), descr.version))
    print(".SH NAME")
    print(name,) 
    print("\- {}".format (clean(descr.whatis)))
    print(".SH SYNOPSIS")
    print(".B {}".format (clean(descr.basic_usage)))
    print(".SH DESCRIPTION")
    print(".B {}".format (name))
    print_description(descr.description)
    print(".SH OPTIONS")
    for g in descr.option_groups:
        if len(g.name) > 0:
            print(".SS {}".format(g.name))

        print(".RS")
        for o in g.options:
            o.print_man()
        print(".RE")
    
    handlerkeys = sorted(descr.handlers.keys())
    for k in handlerkeys: 
        h = descr.handlers[k]
        print(".SH PLUGINS: {}".format (h.name))
        for p in h.plugins:
            print(".TP 10")
            print(".B {}".format(p.name))
            if len(p.params) > 0: 
                print("{}, supported parameters are: ".format (p.text))
                print(".P")
                for o in p.params:
                    print(".RS 14")
                    o.print_man()
                    print(".RE")
            else:
                print(p.text)
                print(".P")
                if not p.no_params_info: 
                    print(".RS 14")
                    print("(no parameters)")
                    print(".RE")
                    
            if p.suffixes is not None:
                print(".RS 14")
                print("Recognized file extensions: ", p.suffixes)
                print(".RE")
                print(" ")

            if p.supported_types is not None:
                print(".RS 14")
                print("Supported element types: ")
                print(".RS 2")
                print(p.supported_types)
                print(".RE")
                print(" ")
                print(".RE")


    if descr.Example.text is not None and len(descr.Example.text) > 0:
            print(".SH EXAMPLE")
            print(clean(descr.Example.text))
            for c in descr.Example.code:
                print(".HP")
                print("{} {}".format (name, clean(c.text)))

    print(".SH AUTHOR(s)")
    print(clean(descr.author))
    
    print(".SH COPYRIGHT")
    print("""This software is Copyright (c) 1999\(hy2015 Leipzig, Germany and Madrid, Spain.
It comes  with  ABSOLUTELY  NO WARRANTY  and  you  may redistribute it under the terms of the GNU 
GENERAL PUBLIC LICENSE Version 3 (or later). For more 
information run the program with the option '\-\-copyright'.""")


X=parse_file(sys.argv[1])
write_man_file(X)
