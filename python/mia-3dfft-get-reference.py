#!/usr/bin/env python
#
# Copyright (c) Leipzig, Madrid 2004-2010
# Max-Planck-Institute for Human Cognitive and Brain Science	
# Max-Planck-Institute for Evolutionary Anthropology 
# BIT, ETSI Telecomunicacion, UPM
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
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


import os
import sys

from Numeric import *
from numpy.fft import *

from string import atof
from string import atoi
from string import split

from subprocess import Popen
from subprocess import PIPE
from subprocess import call

from optparse import OptionParser

# some fixed code
cost_fkt = "mia-3dcost"

class SelfcostElement:
    def __init__(self, file, value, nr):
        self.file = file
        self.value = value
        self.nr = nr

    def __cmp__(self, other):
        if self.value < other.value:
            return -1;
        if self.value > other.value:
            return 1; 
        return 0

def get_cost(src, ref, function):
    """ Evaluate the cost function value between the images src and ref by using the given cost function"""
    helper = "%s %s" % (cost_fkt, function)
    command = helper % (src, ref)
    cs = Popen(command, shell=True, stdout=PIPE)
    retval = cs.communicate()
    token = split(retval[0])
    return atof(token[0])
    
def get_intensity_deviation(src):
    """Evaluate some image statistics"""
    command =  "mia-3dimagestats -i %s" % (src)
    cs = Popen(command, shell=True, stdout=PIPE)
    retval = cs.communicate()
    token = split(retval[0])
    return atof(token[0])

def get_ref_candidates(nr_files, file_descr):
    """Evaluate the best possible reference images based on the intensity deviation"""
    k = 0
    f = file_descr % (k)
    while not os.path.exists(f):
        k = k + 1
        f = file_descr % (k)
        
    startidx = k; 
    costs = []
    
    while (k < nr_files + startidx):
        if os.path.exists(f):
            value = -get_intensity_deviation(f)
            costs.append(SelfcostElement(f, value, k))
            if options.verbose == "message":
                print "%s -> %f" % (f, value)

        k = k + 1
        f = options.file_descr % (k)

    costs.sort()
    return (startidx, costs[0:10])


def get_reference_and_costs(nfiles, file_descr):
    (startidx, candidates) = get_ref_candidates(nfiles, file_descr)
    endidx = options.nr_files + startidx
    real_endidx = startidx
    all_vals = []
    maxval = 0.0
    for c in candidates:

        values = []
        k = startidx
        while k < endidx:
            f = options.file_descr % (k)
            if os.path.exists(f):
                value = get_cost(f, c.file, "ngf:src=%s,ref=%s,eval=delta")
                values.append(value)
                real_endidx = k
                if options.verbose == "message":
                    print "%s vs %s -> %f" % (c.file,f, value)
                    
            else:
                values.append(0.0)
            k = k + 1
    
        freq = fft(array(values), n=None, axis=-1)[1:len(values)/2]
        res = []
        
        l_max = 0.0
        
        for f in freq:
            val = abs(f)
            if val > l_max: 
                l_max = val

        if l_max > maxval: 
            maxval = l_max
            result = c.nr
            all_vals = values

    return (result, startidx, real_endidx, all_vals)


# Real start of script     

parser = OptionParser()
parser.add_option("-n", "--nr-files", type="int", dest="nr_files", 
                  action="store", help="number of input files", default=60)
parser.add_option("-f", "--filenamedescr", type="string", dest="file_descr", 
                  action="store", help="input file name pattern", default="data%04d.exr")
parser.add_option("-V", "--Verbose", type="string", dest="verbose", default="error",
                  action="store", help="Verbosity of output")

(options, args) = parser.parse_args()



(result, startidx, real_endidx, all_vals) = get_reference_and_costs(options.nr_files, options.file_descr)

listfile = open("costs.txt","w")

listfile.write("%d %d\n" % (startidx,real_endidx))

for v in all_vals:
    listfile.write("%f\n" % v)
listfile.close()

print result
