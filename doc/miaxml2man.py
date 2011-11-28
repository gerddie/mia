#!/bin/env python 

import sys
import time
import calendar
import string
import htmlentitydefs
import re

modules = {'miareadxml' : [0, '', 'none://miareadxml.py' ]
           }


from miareadxml import parse_file

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

        print ".RS"
        for o in g.options:

            if len(o.short) > 0:
                short = "\-" + o.short
            else:
                short = "  "; 

            if o.required:
                print ".IP \"%s \-\-%s=[required]\""% (short, o.long)
            else:
                print ".IP \"%s \-\-%s=%s\""% (short, o.long, escape_dash(o.default))
            print o.text
        print ".RE"
        
    for h in descr.handlers: 
        print ".SH PLUGINS: %s" % (h.name)
        for p in h.plugins:
            print ".TP"
            print ".B %s" % p.name
            print p.text
            print ".RS"
            for o in p.params:
                if o.required:
                    print ".IP \"%s:%s [required]\"" % (o.name, o.type)
                else:
                    print ".IP %s:%s" % (o.name, o.type)
                print o.text
            print ".RE"

    if descr.Example.text is not None and len(descr.Example.text) > 0:
            print ".SH EXAMPLE"
            print clean(descr.Example.text)
            for c in descr.Example.code:
                print ".HP"
                print "%s %s" % (name, clean(c.text))
    
    print ".SH COPYRIGHT"
    print "This software is copyright (c) Gert Wollny et al."
    print "It comes with  ABSOLUTELY NO WARRANTY and you may redistribute it"
    print "under the terms of the GNU GENERAL PUBLIC LICENSE Version 3 (or later)."



X=parse_file(sys.argv[1])
write_man_file(X)
