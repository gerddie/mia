#!/usr/bin/env python
#
# Copyright (c) Leipzig, Madrid 2004 - 2009
# Max-Planck-Institute for Human Cognitive and Brain Science	
# Max-Planck-Institute for Evolutionary Anthropology 
# BIT, ETSI Telecomunicacion, UPM
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#


#!/usr/bin/env python
import os
import sys

from optparse import OptionParser
from math import sqrt
from string import atof
from string import atoi
from string import split
from subprocess import Popen
from subprocess import PIPE
from subprocess import call


parser = OptionParser()
parser.add_option("-n", "--nr-files", type="int", dest="nr_files", 
                  action="store", help="number of input files", default=60)
parser.add_option("-f", "--filenamedescr", type="string", dest="file_descr", 
                  action="store", help="input file name pattern", default="data%04d.hdr")
parser.add_option("-l", "--mr-levels", type="int", dest="levels", default=3, 
                  action="store", help="number of input multi-resolution levels")
parser.add_option("-m", "--mgstart", type="int", dest="mgstart", default=16,
                  action="store", help="multigrid start size")
parser.add_option("-o", "--outout_dir", type="string", dest="output", default=".",
                 action="store", help="output directory")
parser.add_option("-V", "--Verbose", type="string", dest="verbose", default="error",
                  action="store", help="Verbosity of output")
parser.add_option("-R", "--ref", type="int", dest="reference", default=2,
                  action="store", help="Series reference image number")

(options, costfkts) = parser.parse_args()

if not os.path.isdir(options.output):
    os.mkdir(options.output)

reg_cmd="mia-3dmulti-nrreg"
defo_cmd="mia-3ddeform"

class Subsequence:
    def __init__(self, ref, seq):
        self.ref = ref
        self.seq = seq




def GetRegCommandList(costs, src, ref, ref_descr = "data%04d.hdr"):
    result = []
    result.append(reg_cmd)
    result.append("-o")
    result.append("%s/field%04d-%04d.v" % (options.output, src, ref))
    result.append("-s")
    result.append("%d" % (options.mgstart))
    result.append("-m")
    result.append("navierpsse:iter=100,epsilon=0.0001")
    result.append("-V")
    result.append(options.verbose)

    refname = ref_descr % (ref)
    srcname = options.file_descr % (src)
    for c in costs:
        print c
        result.append(c % (srcname,refname) )

    if options.verbose == "message":
	print result
    return result

def GetDeformCommandList(src, ref, output):
    result = []
    result.append(defo_cmd)
    result.append("-i")
    result.append(options.file_descr % (src))
    result.append("-t")
    result.append("%s/field%04d-%04d.v" % ( output, src, ref))
    result.append("-o") 
    result.append("%s/reg%04d.hdr" % (output, src))
    return result

def get_subsequence(startidx, endidx, reference, max_values):
    
    sub_sequence = []

    sub_sequence.append(reference)
    
    k = reference + 3 - startidx
    
    while k < endidx - 2 - startidx:
        if max_values[k] == 0: 
            k = k + 1
            continue
        if max_values[k] < max_values[k - 1] \
        and max_values[k] < max_values[k + 1]:
            sub_sequence.append(k + startidx)
            k = k + 3
        else: 
            k = k + 1

    sub_sequence.append(endidx)

    k = reference - 3 - startidx
    while k > startidx:
        if max_values[k] == 0: 
            k = k - 1
            continue
        if max_values[k] < max_values[k - 1] \
        and max_values[k] < max_values[k + 1]:
            sub_sequence.append(k + startidx)
            k = k - 3
        else: 
            k = k - 1

    sub_sequence.append(startidx)
    
    sub_sequence.sort()
    return Subsequence(reference, sub_sequence)

def GenerateTest(lower_bound, upper_bound, i):
    cmd = []
    cmd.append("mia-3dlerp")
    cmd.append("-1")
    cmd.append("%s/reg%04d.hdr" % (options.output, lower_bound))
    cmd.append("-2")
    cmd.append("%s/reg%04d.hdr" % (options.output, upper_bound))
    cmd.append("-o")
    cmd.append("%s/gen%04d.hdr" % (options.output, i))
    cmd.append("-p")
    cmd.append("%d %d %d" % (lower_bound, i, upper_bound))
    try:
        retcode = call(cmd)
        if retcode < 0:
            print >>sys.stderr, "Call was terminated by signal", -retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed", e    


# read costs
file = open("costs.txt", "r")
costs=[]
first = True
startidx = 0
endidx = options.nr_files

for l in file: 
    if first:
        tokens = split(l,' ')
        startidx = atoi(tokens[0])
        endidx   = atoi(tokens[1])
        first = False
    else:
        costs.append(atof(l))
file.close(); 
        
sequence = get_subsequence(startidx, endidx, options.reference, costs)

print "%04d" % (sequence.ref)

for k in range(len(sequence.seq)):
    print "%04d" % sequence.seq[k]


#
# "generate" the registration result of the main reference image
#
#
#  getting the general reference image should be split off from the rest of the code
#
reforigin = options.file_descr % (sequence.ref)
dataorigin = reforigin[0:-3]+'img'
if options.file_descr[0] == '/':
    cmd1 = "ln -sf %s %s/reg%04d.hdr" % (reforigin, options.output, sequence.ref)
    cmd2 = "ln -sf %s %s/reg%04d.img" % (dataorigin, options.output, sequence.ref)
else:
    cmd1 = "ln -sf $(pwd)/%s %s/reg%04d.hdr" % (reforigin, options.output, sequence.ref)
    cmd2 = "ln -sf $(pwd)/%s %s/reg%04d.img" % (dataorigin, options.output, sequence.ref)

retcode = call(cmd1, shell=True )
retcode = call(cmd2, shell=True )
#
# register the subsequence
#
for i in sequence.seq: 
    try:
        if i != sequence.ref:
            print "subsequence registration of %d]" % (i)
            refname= "%s/%s" % (options.output, "reg%04d.hdr")
            cmd = GetRegCommandList(["ngf:src=%s,ref=%s,eval=delta"], i, sequence.ref, refname)
            retcode = call(cmd)
            if retcode < 0:
                print >>sys.stderr, "Call was terminated by signal", -retcode
            cmd = GetDeformCommandList(i, sequence.ref, options.output)
            print cmd 
            retcode = call(cmd)
            if retcode < 0:
                print >>sys.stderr, "Call was terminated by signal", -retcode
        
    except OSError, e:
        print >>sys.stderr, "Execution failed", e

#
# register the remaining images
#
upper_idx = 0
upper_bound = sequence.seq[upper_idx]
lower_bound = -1

i = startidx
while i <= endidx:
    if i not in sequence.seq:
        print "final registration of %d in [%d, %d]" % (i, lower_bound, upper_bound)
        if lower_bound == -1 or upper_bound == lower_bound:
            refname= "%s/%s" % (options.output, "reg%04d.hdr")
            cmd = GetRegCommandList(["ssd:src=%s,ref=%s"], i, upper_bound, refname)
        else:
            # generate a reference image
            GenerateTest(lower_bound, upper_bound, i)
            refname= "%s/%s" % (options.output, "gen%04d.hdr")
            cmd = GetRegCommandList(["ssd:src=%s,ref=%s"], i, i, refname)
            
        try:
            retcode = call(cmd)
            if retcode < 0:
                print >>sys.stderr, "Call was terminated by signal", -retcode    
            cmd = GetDeformCommandList(i, i, options.output)
            retcode = call(cmd)
            if retcode < 0:
                print >>sys.stderr, "Call was terminated by signal", -retcode    
        except OSError, e:
            print >>sys.stderr, "Execution failed", e
                
    i = i + 1; 
    if i > upper_bound:
        lower_bound = upper_bound
        upper_idx = upper_idx + 1
        if upper_idx < len(sequence.seq):
            upper_bound = sequence.seq[upper_idx]
    

print "registration should be done" 
        
