#!/usr/bin/env python 
#
# Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

# this program is used to translate the XML files obtained by running 
# a mia-* program with --help-xml into a nipype interface 

import sys
import time
import calendar
import string
import htmlentitydefs
import re
from os import path

from argparse import ArgumentParser
from argparse import RawTextHelpFormatter

sys.dont_write_bytecode = True

modules = {'miareadxml' : [0, '', 'none://miareadxml.py' ]
           }


nipype_header = """from nipype.interfaces.base import (
        TraitedSpec,
        CommandLineInputSpec,
        CommandLine,
        File,
        traits
    )
import os"""


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

def dash_to_underscore(text): 
    return re.sub(r'-', r'_', text) 

def clean (text):
    text = unescape(text)
    return escape_dash(text) 


class  NipypeOutput: 
    def __init__(self, input_file, output_file):
        self.input_file = input_file
        self.output_file = output_file
        self.descr = parse_file(input_file)
        self.out=open(output_file, 'w')
        self.ParamTable = {
            "2dbounds" : lambda i : self.create_vint_param(i), 
            "3dbounds" : lambda i : self.create_vint_param(i), 
            "vshort":    lambda i : self.create_vint_param(i), 
            "vint":      lambda i : self.create_vint_param(i), 
            "vlong":     lambda i : self.create_vint_param(i), 
            "vushort":   lambda i : self.create_vint_param(i), 
            "vuint":     lambda i : self.create_vint_param(i), 
            "vulong":    lambda i : self.create_vint_param(i), 

            "2dfvector" :lambda i : self.create_vfloat_param(i), 
            "3dfvector" :lambda i : self.create_vfloat_param(i), 
            "vdouble"   :lambda i : self.create_vfloat_param(i), 
            "vfloat"    :lambda i : self.create_vfloat_param(i), 

            "bool"  : lambda i : self.create_Bool_param(i), 
            "short" : lambda i : self.create_Integral_param(i), 
            "int"   : lambda i : self.create_Integral_param(i), 
            "long"  : lambda i : self.create_Integral_param(i),
            "ushort": lambda i : self.create_Integral_param(i), 
            "uint"  : lambda i : self.create_Integral_param(i), 
            "ulong" : lambda i : self.create_Integral_param(i), 
            "float" : lambda i : self.create_Float_param(i), 
            "double": lambda i : self.create_Float_param(i), 

            "vstring": lambda i : self.create_input_VString_param(i), 

            "dict"  : lambda i : self.create_Dict_param(i), 
            "set"  : lambda i : self.create_Set_param(i), 
            "factory" : lambda i : self.create_Factory_param(i)
        }
        

    def create_trait_input_param_start(self, param, trait, enums=""):
        self.out.write ( '\tinput_{} = traits.{}({} desc="{}", '.format(dash_to_underscore(param.long), 
                                                                                trait, enums, param.text)), 

    def create_param_tail(self, param):
        if param.required: 
            self.out.write (', mandatory = True '), 
        self.out.write( ')\n')

    def create_Bool_param(self, param):
        self.create_trait_input_param_start(param, 'Bool')
        self.out.write('argstr="--{}" '.format(param.long)), 
        self.create_param_tail(param)

    def  create_vint_param(self, param):
        self.create_trait_input_param_start(param, 'ListInt')
        self.out.write('argstr="--{} %s", sep=","'.format(param.long)), 
        self.create_param_tail(param)
        
    def create_vfloat_param(self, param):
        self.create_trait_input_param_start(param, 'ListFloat')
        self.out.write('argstr="--{} %s", sep=","'.format(param.long)), 
        self.create_param_tail(param)

    def create_input_VString_param(self, param):
        self.create_trait_input_param_start(param, 'ListString')
        self.out.write('argstr="--{} %s", sep=","'.format(param.long)), 
        self.create_param_tail(param)

    def create_Integral_param(self, param):
        self.create_trait_input_param_start(param, 'Int')
        self.out.write('argstr="--{} %d" '.format(param.long)), 
        self.create_param_tail(param)

    def create_Float_param(self, param):
        self.create_trait_input_param_start(param, 'Float')
        self.out.write('argstr="--{} %f" '.format(param.long)), 
        self.create_param_tail(param)

    def create_Dict_param(self, param):
        self.create_trait_input_param_start(param, 'Enum', param.get_names_as_string())
        self.out.write('argstr="--{} %s" '.format(param.long)), 
        self.create_param_tail(param)

    def create_Set_param(self, param):
        self.create_trait_input_param_start(param, 'Enum', param.get_names_as_string())
        self.out.write('argstr="--{} %s" '.format(param.long)), 
        self.create_param_tail(param)

    def create_Factory_param(self, param):
        self.create_trait_input_param_start(param, 'String')
        self.out.write('argstr="--{} %s" '.format(param.long)), 
        self.create_param_tail(param)

    def create_input_String_param(self, param):
        self.create_trait_input_param_start(param, 'String')
        self.out.write('argstr="--{} %s" '.format(param.long)), 
        self.create_param_tail(param)

    def create_input_File_param(self, param):
        self.create_trait_input_param_start(param, 'File')
        self.out.write('argstr="--{} %s", exists=True '.format(param.long)), 
        self.create_param_tail(param)

    def create_output_String_param(self, param):
        self.out.write ( '\toutput_{} = traits.String(desc="{}", '.format(dash_to_underscore(param.long), param.text)), 
        self.out.write('argstr="--{} %s" '.format(param.long)), 
        self.create_param_tail(param)

    def create_output_File_param(self, param):
        self.out.write ( '\toutput_{} = File(desc="{}", '.format(dash_to_underscore(param.long), param.text)), 
        self.out.write('argstr="--{} %s" '.format(param.long)), 
        self.create_param_tail(param)

    def create_output_File_param_outspec(self, param):
        self.out.write ( '\toutput_{} = File(desc="{}"'.format(dash_to_underscore(param.long), param.text)), 
        if param.required: 
                self.out.write (', exists = True '), 
        self.out.write( ')\n')

    def write_unknown_type(self, param):
        print ("WARNING: Unknown  parameter type '{}' encounterd for option '{}', \n".format(param.long, param.type)), 
    
    def write_input_spec(self, name, inputs, params):

        self.out.write ("class {}_InputSpec(CommandLineInputSpec):\n".format(name))

        InputTable = self.ParamTable
        InputTable["string"] = lambda i :self.create_input_File_param(i)
        InputTable["io"] = lambda i :self.create_input_File_param(i)

        for i in inputs: 
            InputTable.get(i.type, self.write_unknown_type)(i)

        ParamTableCopy = self.ParamTable
        ParamTableCopy["string"] = lambda i :self.create_input_String_param(i)

        for i in params: 
            ParamTableCopy.get(i.type, self.write_unknown_type)(i)

    def write_input_outputs(self, name, outputs, params):

        InputTable = self.ParamTable
        InputTable["string"] = lambda i :self.create_output_File_param(i)
        InputTable["io"] = lambda i :self.create_output_File_param(i)

        for i in outputs: 
            InputTable.get(i.type, self.write_unknown_type)(i)

    def write_input_free_params_spec(self, freeparams):
        self.out.write ( '\tinput_free_params = traits.ListStr(desc="Plug-in specifications of type {}", '.format(freeparams)),
        self.out.write ( 'argstr="%s")')

    def write_output_spec(self, name, outputs, params):

        self.out.write ("class {}_OutputSpec(TraitedSpec):\n".format(name))

        ParamTableCopy = self.ParamTable
        ParamTableCopy["string"] = lambda i :self.create_output_File_param_outspec(i)
        ParamTableCopy["io"] = lambda i :self.create_output_File_param_outspec(i)

        for i in outputs: 
            ParamTableCopy.get(i.type, self.write_unknown_type)(i)

        if self.descr.stdout_is_result:
            self.out.write ( '\toutput_stdout = traits.Str()\n')
            
        self.out.write ("\n")

    def write_task(self, name):

        self.out.write('class {}(CommandLine):\n'.format(name))
        self.out.write('\tinput_spec = {}_InputSpec\n'.format(name))
        self.out.write('\toutput_spec = {}_OutputSpec\n'.format(name))
        self.out.write('\t_cmd = "{}"\n'.format(self.descr.name))
        if self.descr.stdout_is_result:
            self.out.write('\tdef aggregate_outputs(self, runtime=None, needed_outputs=None):\n'))
            self.out.write('\t\toutputs = self._outputs()\n')
            self.out.write('\t\toutputs.output_stdout = runtime.stdout\n')
            self.out.write('\t\treturn outputs\n')

    def write_main(self, name):

        self.out.write( 'if __name__ == "__main__":\n')
        self.out.write( '\tmia_prog = {}()\n'.format(name))
        self.out.write( '\tprint( mia_prog.cmdline)\n')

    def write_nipype_file(self):

        name = dash_to_underscore(self.descr.name)

        inputs =[]
        outputs = []
        params = [] 

        for g in self.descr.option_groups:
            for o in g.options:
                if o.no_nipype:
                    continue 
                if o.is_input:
                    inputs.append(o)
                elif o.is_output:
                    outputs.append(o)
                else:
                    params.append(o)

        self.out.write("# This file was generated by running 'miaxml2nipype.py {} {}'\n".format(
                self.input_file, self.output_file))
        self.out.write("# '{}' can be created by running '{} --help-xml >{}'\n".format(
                self.output_file, self.descr.name, self.output_file))
        self.out.write("\n")

        self.out.write(nipype_header)
        self.out.write("\n#\n# Input specification\n#\n"); 
        self.write_input_spec(name, inputs, params)
        self.write_input_outputs(name, outputs, params)
        if self.descr.FreeParams is not None:
            self.write_input_free_params_spec(self.descr.FreeParams)

        self.out.write("\n#\n# Output specification\n#\n"); 
        self.write_output_spec(name, outputs, params)

        self.out.write("\n#\n# Task class specification\n#\n"); 
        self.write_task(name)

        self.out.write("\n#\n# Main function used for testing\n#\n"); 
        self.write_main(name)
   
  

def SanitizeName(matchobj):
    s = matchobj.group(0)
    if s[0] == '-':
        s = s[1:]
    return s.upper() 
         

#
# main program 
#

parser = ArgumentParser(description='Convert MIA style command line tools description into a nipype interface.', 
                                         formatter_class=RawTextHelpFormatter)
        
group = parser.add_argument_group('File I/O')
group.add_argument("-i", "--input",  action="store", required=True,  help="input XML file containing the description")
group.add_argument("-o", "--output", action="store", help="output file name, if not given, the name will be deducted from the input file")
group.add_argument("-p", "--outpath", action="store", help="output path for the nipype interface file")


options = parser.parse_args()

if options.output is None:
    (name , ext) = path.splitext(options.input)
    options.output = re.sub(r'(^[a-z]|-[a-z]|-[0-9][a-z])', SanitizeName, name) + ".py"

if options.outpath is not None:
    options.output = path.join(options.outpath, options.output)


a = NipypeOutput(options.input, options.output)
a.write_nipype_file()

