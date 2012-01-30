#!/usr/bin/env python
#
# (c) 2007-2011 Gert Wollny <gw.fossdev at gmail.com> 
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
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


import os
import sys

from numpy.fft import *
from numpy import array

from string import atof
from string import atoi
from string import split

from subprocess import Popen
from subprocess import PIPE
from subprocess import call

from optparse import OptionParser

cost_fkt = "mia-2dcost image:src=%s,ref=%s,cost=[%s]"
reg_cmd="mia-2dimageregistration"


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

def get_cost(src, ref, mode):
    command = cost_fkt % (src, ref, mode)
    cs = Popen(command, shell=True, stdout=PIPE)
    retval = cs.communicate()
    token = split(retval[0])
    return atof(token[0])
    
def get_intensity_deviation(src):
    command =  "mia-2dimagestats -i %s" % (src)
    cs = Popen(command, shell=True, stdout=PIPE)
    retval = cs.communicate()
    token = split(retval[0])
    return atof(token[0])

def get_ref_candidates(nr_files, skip, file_descr):
    costs = []
    for k in range(nr_files)[skip:]:
        f = file_descr % (k)
        costs.append(SelfcostElement(f, -get_intensity_deviation(f), k))

    costs.sort()
    return costs[0:20]

def get_costs_and_reference(options):
    """ 
    Evaluate the cost series for all possible reference images and 
    select the image with the highest frequency peak as global reference. 
    
    """

    file_path = os.path.join(options.source, options.file_descr)
    candidates = get_ref_candidates(options.nr_files,options.skipsearch,file_path)
    

    maxval = 0.0
    for c in candidates:

        values = []
        for k in range(options.nr_files):
            f = file_path % (k)
            values.append(get_cost(f, c.file, options.cost))
            
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
            all_vals = []
            all_vals.extend(values)

    return ( result, all_vals )


def create_registration_command(cost, src_id, ref_id, ref_descr, options):
    """ Function to create the image registration command """
    result = []
    result.append(reg_cmd)
    
    src_file_descr = os.path.join(options.source, options.file_descr)

    #IO options 
    result.append("-i")
    result.append(src_file_descr % (src_id))
    result.append("-r"),
    result.append(ref_descr % (ref_id))
    result.append("-o")
    result.append("%s/reg%04d.exr" % (options.output, src_id))
    
    # registration options 
    result.append("-l")
    result.append("%d" % (options.levels))
    result.append("-O")
    result.append(options.optimizer)

    #transformation
    result.append("-f")
    result.append("spline:rate=%d" % (options.c_rate))
    
    result.append("-V")
    result.append(options.verbose)
    
    #cost functions 
    result.append("divcurl:weight=%f" % (options.weight))
    result.append("image:cost=[%s]" % (cost))

    if options.verbose == "message":
	print result
    target_name = "%s/reg%04d.exr" % (options.output, src_id) 
    return ( result, target_name) 


def get_subsequence(skip, nr_files, reference, max_values):
    """ The rather crude approach to estimate the pre-aligned subset."""

    sub_sequence = []

    sub_sequence.append(reference)
    
    k = reference + 3
    while k < nr_files - 3:
        if max_values[k] < max_values[k - 1] \
                and max_values[k] < max_values[k - 2] \
                and max_values[k] < max_values[k + 1] \
                and max_values[k] < max_values[k + 2]:
            sub_sequence.append(k)
            k = k + 3
        else: 
            k = k + 1
            
    # this should be refined 
    sub_sequence.append(nr_files - 1)

    k = reference - 3
    while k > skip:
        if max_values[k] < max_values[k - 1] \
                and max_values[k] < max_values[k - 2] \
                and max_values[k] < max_values[k + 1] \
                and max_values[k] < max_values[k + 2]:
            sub_sequence.append(k)
            k = k - 3
        else: 
            k = k - 1
            
    # this should be refined 
    sub_sequence.append(0)
    
    sub_sequence.sort()

    return sub_sequence


def generate_reference_image(lower_bound, upper_bound, i, options):
    cmd = []
    cmd.append("mia-2dlerp")
    cmd.append("-1")
    cmd.append("%s/reg%04d.exr" % (options.output, lower_bound))
    cmd.append("-2")
    cmd.append("%s/reg%04d.exr" % (options.output, upper_bound))
    cmd.append("-o")
    cmd.append("%s/gen%04d.exr" % (options.output, i))
    cmd.append("-p")
    cmd.append("%d %d %d" % (lower_bound, i, upper_bound))
    try:
        retcode = call(cmd)
        if retcode < 0:
            print >>sys.stderr, "Call was terminated by signal", -retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed", e    



