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



####################################################################################
#
# This code requires python 2.X, the packages numpy, optpars, subprocess, string, 
# os, and sys to run and MIA needs to be properly installed.
#  
# This program is used to run the step myocardial perfusion motion compensation of 
# free-breathing aquired data according to 
#
#   G. Wollny, M-J Ledesma-Cabryo, P.Kellman, and A.Santos, 
#   "Exploiting Quasiperiodicity in Motion Correction of Free-Breathing,"
#   IEEE Transactions on Medical Imaging, 29(8), 2010,
#
# for details of the algorithm. 
#
# 
# EXMAPLE: Run the quasi-periodic motion compensation algorithm over 60 images 
# named dataXXXX.exr in path datapath/, skip the first 20 images for reference 
# image search, and the first 2 images during registration
# weight the divcurl cost by value 10.0 and use the default parameters for 
# everything else.  The registered images are saved to outpath/regXXXX.exr. 
#
#   ./quasip-register-series.py -s datapath/ -n 60 -k 20 -K 2 -o outpath -w 10.0
# 
# for this example to work, MIA must be installed with OpenEXR support. 
#
# The algorithm is also implemented as program  mia-2dmyoperiodic-nonrigid
#
####################################################################################

modules = {'tools' : [0, '', 'none://tools.py' ]}

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

from tools import *

parser = OptionParser()
parser.add_option("-n", "--nr-files", type="int", dest="nr_files", 
                  action="store", help="number of input files", default=60)
parser.add_option("-s", "--sourcedir", type="string", dest="source", 
                  action="store", help="source directory", default=".")
parser.add_option("-f", "--filenamedescr", type="string", dest="file_descr", 
                  action="store", help="input file name pattern", default="data%04d.exr")
parser.add_option("-o", "--outout_dir", type="string", dest="output", default=".",
                 action="store", help="output directory")

parser.add_option("-l", "--mr-levels", type="int", dest="levels", default=3, 
                  action="store", help="number of input multi-resolution levels")
parser.add_option("-w", "--weight", type="float", dest="weight", default=2.0,
                  action="store", help="weight of the divcurl regularizer")
parser.add_option("-r", "--c-rate", type="int", dest="c_rate", default=10,
                  action="store", help="B-Spline coefficient rate")
parser.add_option("-O", "--optimizer", type="string", dest="optimizer", default="gsl:opt=gd,step=0.01",
                  action="store", help="Optimizer (gsl or nlopt based)")


parser.add_option("-1", "--comparecost", type="string", dest="comparecost", 
                  action="store", help="cost function for comparison and subset estimation", default="ngd:eval=ds")
parser.add_option("-S", "--startcost", type="string", dest="startcost", default="ngf:eval=ds",
                  action="store", help="cost measure for subset registration")
parser.add_option("-c", "--cost", type="string", dest="cost", default="ssd",
                  action="store", help="cost measure for 2nd pass registration")
parser.add_option("-V", "--Verbose", type="string", dest="verbose", default="error",
                  action="store", help="Verbosity of output")

parser.add_option("-k", "--skipsearch", type="int", dest="skipsearch", 
                  action="store", help="skip reference search at the beginning of the series", default="20")
parser.add_option("-K", "--skipreg", type="int", dest="skipreg", 
                  action="store", help="skip registration for files at the beginning of the series", default="20")


(options, args) = parser.parse_args()

if not os.path.isdir(options.output):
    os.mkdir(options.output)


#
# First step of the algorithm: estimate the global reference image and the pre-aligned subset 
#
( reference, costs ) = get_costs_and_reference(options)
seq = get_subsequence(options.skipreg, options.nr_files, reference, costs)


if options.verbose == "message":
    print "reference = %04d" % (reference), 
    print "sequence:", seq

#
# "generate" the registration result of the main reference image by creating a symbolic link
cmd = "ln -sf %s/data%04d.exr %s/reg%04d.exr" % (options.source, reference, options.output, reference)
retcode = call(cmd, shell=True )


#
# register the subsequence
#
for i in seq: 
    try:
        if i != reference:
            if options.verbose == "message":
                print "subsequence registration of %d]" % (i)
            refname= "%s/%s" % (options.output, "reg%04d.exr")
            (cmd, trgt) = create_registration_command(options.startcost, i, reference, refname, options)
            if not os.path.exists(trgt):
                retcode = call(cmd)
                if retcode < 0:
                    print >>sys.stderr, "Call was terminated by signal", -retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed", e


#
# register the remaining images
#
upper_idx = 0
upper_bound = seq[upper_idx]
lower_bound = -1

for i in range(options.nr_files):
    if i not in seq:
        l_cost = options.cost
        
        if options.verbose == "message":
            print "final registration of %d in [%d, %d]" % (i, lower_bound, upper_bound)
        if lower_bound == -1 or upper_bound == lower_bound:
            refname= "%s/%s" % (options.output, "reg%04d.exr")
            (cmd, trgt) = create_registration_command(l_cost, i, upper_bound, refname, options)
        else:
            generate_reference_image(lower_bound, upper_bound, i, options)
            refname= "%s/%s" % (options.output, "gen%04d.exr")
            (cmd, trgt) = create_registration_command(l_cost, i, i, refname, options)
        try:
            if not os.path.exists(trgt):
                retcode = call(cmd)
                if retcode < 0:
                    print >>sys.stderr, "Call was terminated by signal", -retcode    
        except OSError, e:
            print >>sys.stderr, "Execution failed", e
                
    i = i + 1; 
    if i > upper_bound:
        lower_bound = upper_bound
        upper_idx = upper_idx + 1
        if upper_idx < len(seq):
            upper_bound = seq[upper_idx]
    
if options.verbose == "message":  
    print "registration done"
        
