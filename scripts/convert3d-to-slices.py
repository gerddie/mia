#!/usr/bin/env python
#
# Copyright (c) Madrid 2008, 2009 
# BIT, ETSI Telecomunicacion, UPM
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


# call the program like 
#    convert3d-to-slices.py -i somadata%04d.hdr -o outputdir

import os
import sys

from optparse import OptionParser
from string import atoi
from string import split
from subprocess import Popen
from subprocess import PIPE
from subprocess import call

parser = OptionParser()

parser.add_option("-f", "--filenamedescr", type="string", dest="file_descr", 
                  action="store", help="input file name pattern", default="data%04d.hdr")
parser.add_option("-o", "--outout_dir", type="string", dest="output", default=".",
                 action="store", help="output directory")

(options, args) = parser.parse_args()

sequence_slices=-1
start_image = 0
while sequence_slices < 0:
    name = options.file_descr % (start_image)
    if os.path.exists(name):
        cmd = 'mia-3dgetsize -i %s | sed -e "s/<[0-9]*,[0-9]*,\([0-9]*\)>/\\1/"' % (name)
        cs = Popen(cmd, shell=True, stdout=PIPE)
        retval = cs.communicate()
        token = split(retval[0])
        sequence_slices=atoi(token[0])


for i in range(sequence_slices):
    path = "%s/%d" % (options.output, i)
    if not os.path.exists(path):
        os.mkdir(path)
    

run=True

while run: 
    name = options.file_descr % (start_image)
    tmpname = "%s/data" % options.output
    if os.path.exists(name):
        cmd = "mia-3dimagefilter -i %s -o ubyte.v convert && mia-3dgetslice -i ubyte.v -o %s -t png" % (name,tmpname)
        print cmd
        call(cmd, shell=True)
        for i in range(sequence_slices): 
            outname = "%s/%d/data%04d.png" % (options.output, i, start_image)
            tmpname2 = "%s%04d.png" % (tmpname, i)
            os.rename(tmpname2, outname)
        start_image = start_image + 1
    else:
        run=False


        
