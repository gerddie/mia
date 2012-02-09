from lxml import etree
import re

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
#       dict          
#  Example       text: example descripton 
#    Code        text: example text without program name 


def make_sec_ancor(key, text):
   """remove spaces and hyphens from the input string""" 
   return key + re.sub("[ -/]", "", text)


def escape_dash(text): 
    return re.sub(r'-', r'\-', text) 


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
        self.type = node.get("type")

    def print_man(self):
       if len(self.short) > 0:
          short = "\-" + self.short
       else:
          short = "  "; 

       if self.required:
          print ".IP \"%s \-\-%s=(required)\""% (short, self.long)
       else:
          print ".IP \"%s \-\-%s=%s\""% (short, self.long, escape_dash(self.default))
          print self.text, 
          self.do_print_man()
       

    def do_print_man(self):
       print "" 


class CDictOption(COption):
    def __init__(self, node):
       COption.__init__(self, node)
       self.dict = {}
       for child in node.iter("dict"):
          for v in child:
                self.dict[v.get("name")] = v.text

    def do_print_man(self):
       if len(self.dict) > 0:
          print "" 
          for k in self.dict.keys(): 
             print ".RS 10"
             print ".I" 
             print k
             print "- %s" % (self.dict[k])
             print ".RE"



class CFactoryOption(COption):
    def __init__(self, node):
       COption.__init__(self, node)
       self.factory = "unknown/factory"
       for child in node.iter("factory"):
          self.factory = child.get("name")

    def do_print_man(self):
       print " For supported plugins see PLUGINS:%s" % (self.factory)

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
               p = {
                    "factory": lambda n: CFactoryOption(n), 
                    "dict":    lambda n: CDictOption(n),
                    }.get(child.get("type"), lambda n: COption(n))(child)
               self.options.append(p)
            else:
                print "unexpected subnode '%s' in 'group'"% (child.tag)

def get_text_element(root, name):
        xname = etree.SubElement(root, name)
        if xname is None:
            raise ValueError("Program description misses the tag %s"% (name))
        return xname.text

class CParam: 
    def __init__(self, node):
        if node.tag != "param":
            raise ValueError("expected 'param' got '%s'" % (node.tag))
        self.name = node.get("name")
        self.type = node.get("type")
        self.default = node.get("default")
        self.required = int(node.get("required")) 
        self.text = node.text

        m = re.search('[,=:]', self.default) 
        # if there is a ',' in the text make clean that it needs to be escaped  
        if m is not None: 
            self.default = "[" + self.default + "]"
        
    def print_man(self):
        print ".I"
        print self.name
        if self.required:
            print "= (required, %s) " % (self.type)
        else:
            print "= %s (%s) " % (self.default, self.type)
        print ".RS 2"
        print "%s." % (self.text)
        self.do_print_man()
        print ".RE"

    def do_print_man(self):
        print ""

    def print_xml_help(self, root):
        row = etree.SubElement(root, "row")
        e = etree.SubElement(row, "entry", align="center", valign="top")
        e.text = self.name 
        e = etree.SubElement(row, "entry", align="center", valign="top")
        e.text = self.type
        e = etree.SubElement(row, "entry", align="center", valign="top")
        if self.required: 
            e.text = "(required)"
        else:
            e.text = self.default
        
        self.do_print_xml_help_description(row)
        
    def do_print_xml_help_description(self, row):
        e = etree.SubElement(row, "entry", align="left", valign="top")
        e.text = self.text

class CRangeParam(CParam):
    def __init__(self, node):
        CParam.__init__(self,node)
        self.min = node.get("min")
        self.max = node.get("max")

    def do_print_man(self):
        print "in [%s, %s]" % (self.min, self.max)
        CParam.do_print_man(self)

    def do_print_xml_help_description(self, row):
        e = etree.SubElement(row, "entry", align="left", valign="top")
        e.text = p.test + " in [%s, %s]" % (self.min, self.max)


class CDictParam(CParam):
    def __init__(self, node):
        CParam.__init__(self,node)
        self.dict = {}
        for n in node:
            if n.tag == "dict":
                for v in n:
                    self.dict[v.get("name")] = v.text
                    
    def do_print_man(self):
        print "Supported values are:"
        for k in self.dict.keys(): 
            print ".RS 4"
            print ".I" 
            print k
            print "- %s" % (self.dict[k])
            print ".RE"
        CParam.do_print_man(self)

    def do_print_xml_help_description(self, row):
        e = etree.SubElement(row, "entry")
        table = etree.SubElement(e, "informaltable") 
        tgroup = etree.SubElement(table, "tgroup", cols="2")
        colspec = etree.SubElement(tgroup, "colspec", colname="sc0")
        colspec = etree.SubElement(tgroup, "colspec", colname="sc1")
        tbody = etree.SubElement(tgroup, "tbody")
        
        subrow = etree.SubElement(tbody, "row")
        e = etree.SubElement(subrow, "entry", align="left", valign="top", namest="sc0", nameend="sc1")
        e.text = self.text 
        for d in self.dict.keys(): 
            subrow = etree.SubElement(tbody, "row")
            e = etree.SubElement(subrow, "entry", align="left", valign="top")
            e.text = d + ":"
            e = etree.SubElement(subrow, "entry", align="left", valign="top")
            e.text = self.dict[d]
        return e

class CSetParam(CParam):
    def __init__(self, node):
        CParam.__init__(self,node)
        self.set = []
        for n in node:
            if n.tag == "set":
                for v in n:
                    if v.tag == "value":
                        self.set.append(v.get("name"))

    def do_print_man(self):
        print "Supported values are:(", 
        for k in self.set:
            print "%s, " % (k), 
        print ")"
        CParam.do_print_man(self)

    def do_print_xml_help_description(self, row):
        e = etree.SubElement(row, "entry", align="left", valign="top")
        str_list = [self.text, " Supported values are:("]

        for k in self.set:
            str_list.append("%s, " % (k))
        str_list.append(")")
        e.text = ''.join(str_list)
                    
class CFactoryParam(CParam):
    def __init__(self, node):
        CParam.__init__(self,node)
        self.factory = ""
        for n in node:
            if n.tag == "factory":
                self.factory = n.get("name")

    def do_print_man(self):
        print "For supported plug-ins see PLUGINS:%s" % (self.factory)
        CParam.do_print_man(self)

    def do_print_xml_help_description(self, row):
        e = etree.SubElement(row, "entry", align="left", valign="top")
        e.text = self.text + ". For supported plug-ins see "
        etree.SubElement(e, "xref", linkend=make_sec_ancor("Sec", self.factory))
       
class CPlugin: 
    def __init__(self, node):
        if node.tag != "plugin":
            raise ValueError("expected 'plugin' got '%s'" % (node.tag))
        self.name = node.get("name")
        self.text = node.text

        self.params = []
        for child in node:
            if child.tag == "param": 
                p = {
                    "range":   lambda n: CRangeParam(n), 
                    "factory": lambda n: CFactoryParam(n), 
                    "set":     lambda n: CSetParam(n),
                    "dict":    lambda n: CDictParam(n),
                    }.get(child.get("type"), lambda n: CParam(n))(child)
                
                self.params.append(p)
            else:
                print "unexpected subnode '%s' in 'plugin'"% (child.tag)

class CHandler: 
    def __init__(self, node):
        if node.tag != "handler":
            raise ValueError("expected 'handler' got '%s'" % (node.tag))
        self.entry = node.tag
        self.name =  node.get("name")

        self.users = []
        self.plugins = []

        for child in node:
            if child.tag == "plugin": 
                self.plugins.append(CPlugin(child))
            else:
                print "unexpected subnode '%s' in 'handler'" % (child.tag)
    def append_user(self, user):
            self.users.append(user)

class CDescription: 
    def __init__(self, node):
        self.Example = None 
        self.option_groups = []
        self.handlers = []
        for n in node:
            if n.tag == 'name':
                self.name = n.text
            elif n.tag == 'section':
                self.section = n.text
            elif n.tag == 'description':
                self.description = n.text
            elif n.tag == 'basic_usage':
                self.basic_usage = n.text
            elif n.tag == 'handler':
                self.handlers.append(CHandler(n))
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
        raise ValueError("Expected tag 'program' not found. '"+xmlpath+"' is not a mia program descripion.")
    
    return CDescription(root)
