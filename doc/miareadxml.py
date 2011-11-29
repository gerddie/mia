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

class CParam: 
    def __init__(self, node):
        if node.tag != "param":
            raise ValueError("expected 'param' got '%s'" % (node.tag))
        self.name = node.get("name")
        self.type = node.get("type")
        self.default = node.get("default")
        self.required = int(node.get("required")) 
        self.text = node.text

class CPlugin: 
    def __init__(self, node):
        if node.tag != "plugin":
            raise ValueError("expected 'plugin' got '%s'" % (node.tag))
        self.name = node.get("name")
        self.text = node.text

        self.params = []
        for child in node:
            if child.tag == "param": 
                self.params.append(CParam(child))
            else:
                print "unexpected subnode '%s' in 'plugin'"% (child.tag)

class CHandler: 
    def __init__(self, node):
        if node.tag != "handler":
            raise ValueError("expected 'handler' got '%s'" % (node.tag))
        self.entry = node.tag
        self.name =  node.get("name")
        
        self.plugins = []
        for child in node:
            if child.tag == "plugin": 
                self.plugins.append(CPlugin(child))
            else:
                print "unexpected subnode '%s' in 'handler'"% (child.tag)
        

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
        raise ValueError("Expected tag 'program' not found. This is not a mia program descripion.")
    
    return CDescription(root)
