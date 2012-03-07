from lxml import etree
import re

xml_namespace = "http://docbook.org/ns/docbook"
xmlns = "{%s}" % xml_namespace

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
    return key + re.sub("[ -/:]", "", text)


def get_text_node_simple(tag,  text):
    node = etree.Element(tag)
    node.text = text
    return node


def get_dict_table(dictionary, tabletype):
    entry = etree.Element(tabletype, frame="none")
    tgroup = etree.Element("tgroup", cols="2", colsep="0", rowsep ="0")
    colspec = etree.Element("colspec", colname="c1")
    tgroup.append(colspec)
    colspec = etree.Element("colspec", colname="c2")
    tgroup.append(colspec)

    tbody = etree.Element("tbody")
    for d in dictionary.keys(): 
        row = etree.Element("row")
        e = etree.Element("entry", align="left", valign="top")
        e.text = d + ":"
        row.append(e)
        e = etree.Element("entry", align="left", valign="top")
        e.text = dictionary[d]
        row.append(e)
        tbody.append(row)
    tgroup.append(tbody)
    entry.append(tgroup)
    return entry

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
            if not self.type == "bool":
                print ".IP \"%s \-\-%s=%s\""% (short, self.long, escape_dash(self.default))
            else:
                print ".IP \"%s \-\-%s\""% (short, self.long)
        print self.text, 
        self.do_print_man()


    def do_print_man(self):
        print "" 

    def write_xml(self, entry):
        # create the terminal text 
        termtext = "-"
        if len(self.short) > 0: 
            termtext = termtext + self.short + ", -"
        termtext = termtext + "-" + self.long
        if self.required: 
            termtext = termtext + "=(required)"
        elif self.type != "bool":
            termtext = termtext + "="
            if len(self.default)>0:
                termtext = termtext + self.default
            elif self.type == "string":
                termtext = termtext + "STRING"

        entry.append(get_text_node_simple("term",  termtext))
        item =  etree.Element("listitem")
        para = get_text_node_simple("para", self.text)
        self.do_write_xml(para)
        item.append(para)
        entry.append(item)

    def do_write_xml(self, parent):
        pass 


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


    def do_write_xml(self, parent):
        if len(self.dict) > 0:
            opttable = get_dict_table(self.dict, "informaltable")
            parent.append(opttable)


class CFactoryOption(COption):
    def __init__(self, node):
        COption.__init__(self, node)
        self.factory = "unknown/factory"
        for child in node.iter("factory"):
            self.factory = child.get("name")

    def do_print_man(self):
        print " For supported plugins see PLUGINS:%s" % (self.factory)

    def do_write_xml(self, parent):
        parent.text = parent.text + ". For supported plug-ins see "
        etree.SubElement(parent, "xref", linkend=make_sec_ancor("SecPlugintype", self.factory))


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

    def append_to_handler(self, handlers, link):
        pass

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
        table = etree.SubElement(e, "informaltable", pgwide="1") 
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
        etree.SubElement(e, "xref", linkend=make_sec_ancor("SecPlugintype", self.factory))


    def append_to_handler(self, handlers, link):
        if handlers.has_key(self.factory):
            handlers[self.factory].append_user(link)
        else:
            raise RuntimeError("Handler %s is used by plugin %s, but is not available" % (self.factory, link))

class CPlugin: 
    def __init__(self, node, handlername):
        if node.tag != "plugin":
            raise ValueError("expected 'plugin' got '%s'" % (node.tag))
        self.name = node.get("name")
        self.text = node.text
        self.handlername = handlername
        self.ancor = make_sec_ancor("plugin"+self.name, self.handlername)

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

    def append_to_handler(self, handlers):
        for p in self.params:
            p.append_to_handler(handlers, self.ancor)


    def write_xml(self, parent):
        head = etree.SubElement(parent, "bridgehead", renderas="sect4", xreflabel=self.name+":"+self.handlername)
        head.set(xmlns+"id", self.ancor)
        head.text = self.name 

        node = etree.SubElement(parent, "para", role="plugindescr")
        param_list = self.params

        if len(param_list) > 0:
            node.text = self.text + ". Supported parameters are:"
            table = etree.SubElement(node, "informaltable", frame="all", role="pluginparms", pgwide="1")
            tgroup = etree.SubElement(table, "tgroup", cols="3", colsep="0", rowsep ="0")
            colspec = etree.SubElement(tgroup, "colspec", colname="c1", colwidth="10%")
            colspec = etree.SubElement(tgroup, "colspec", colname="c2", colwidth="10%")
            colspec = etree.SubElement(tgroup, "colspec", colname="c3", colwidth="10%")
            colspec = etree.SubElement(tgroup, "colspec", colname="c4", colwidth="70%")
            thead = etree.SubElement(tgroup, "thead")
            row = etree.SubElement(thead, "row"); 
            e = etree.SubElement(row, "entry", align="center", valign="top")
            e.text = "Name"
            e = etree.SubElement(row, "entry", align="center", valign="top")
            e.text = "Type"
            e = etree.SubElement(row, "entry", align="center", valign="top")
            e.text = "Default"
            e = etree.SubElement(row, "entry", align="center", valign="top")
            e.text = "Description"
            tbody = etree.SubElement(tgroup, "tbody")

            for p in param_list: 
                p.print_xml_help(tbody)
        else:
            node.text = self.text + "(This plug-in doesn't take parameters)"

class CHandler: 
    def __init__(self, node):
        if node.tag != "handler":
            raise ValueError("expected 'handler' got '%s'" % (node.tag))
        self.entry = node.tag
        self.name =  node.get("name")
        self.users = set([])
        self.plugins = []

        for child in node:
            if child.tag == "plugin": 
                self.plugins.append(CPlugin(child, self.name))
            else:
                print "unexpected subnode '%s' in 'handler'" % (child.tag)

    def append_user(self, user):
        self.users.add(user)

    def merge_users(self, users):
        self.users = self.users.union(users)




class CDescription: 
    def __init__(self, node):
        self.Example = None 
        self.FreeParams = None
        self.option_groups = []
        self.handlers = {}
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
                handler = CHandler(n)
                self.handlers[handler.name] = handler
            elif n.tag == 'group': 
                self.option_groups.append(CGroup(n))
            elif n.tag == 'Example': 
                self.Example = CExample(n)
            elif n.tag == 'freeparams':
                self.FreeParams = n.get("name")
            else: 
                print "unknown tag '%s'"% (n.tag)
        self.anchor = make_sec_ancor("Sec", self.name)
        self.link_handler_consumers()

    def link_handler_consumers(self):
        for h in self.handlers.keys():
            self.handlers[h].append_user(self.anchor) 
            for p in self.handlers[h].plugins:
                p.append_to_handler(self.handlers)
        if not  self.FreeParams is None:
            handler = self.handlers[h] 
            handler.append_user(self.anchor)

def parse_file(xmlpath):
    file=open(xmlpath, "r")
    stringstree = file.read()
    root = etree.XML(stringstree)
    if root.tag != "program": 
        raise ValueError("Expected tag 'program' not found. '"+xmlpath+"' is not a mia program descripion.")

    return CDescription(root)
