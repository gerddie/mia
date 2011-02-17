#!/usr/bin/env python
#
# Copyright (c) Madrid 2008 
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
#
#


from Section import Section
from Section import ImageSegmentation
from Section import GlobalColors
from Section import WorkSet

import os
import unittest
from lxml import etree

class TestWorkSet(unittest.TestCase):
    def setUp(self):
        self.image_files = ["testimage.png",  "testimage2.png"]        
        self.colors = [["blue", "yellow"],["green", "red"]]
         
    def testLoadedEnough(self):
        self.workset = WorkSet(images=self.image_files)
        self.assertEqual(self.workset.GetFrameNumber(), len(self.image_files))

    def testLoadExample(self):
        workset = WorkSet(xmlpath="testset.set")
        self.assertEqual(workset.GetFrameNumber(), 3)
        for i in range(workset.GetFrameNumber() - 1):
            frame = workset.GetFrame(i)
            self.assertEqual(frame.image_file, self.image_files[i])
            self.assertEqual(len(frame.Sections), 2)
            for k in range(len(frame.Sections)):
                self.assertEqual(frame.Sections[k].color, self.colors[i][k])
                self.assertEqual(len(frame.Sections[k].points), 3)
                
    def testLoadStore(self):
        file=open("testset.set", "r")
        stringstree = file.read()
        workset = WorkSet(xmlpath="testset.set")
        savestringstree = workset.Save()
        
        file=open("saveset.set", "w")
        file.write(savestringstree)
        file.close()

        self.assertEqual(savestringstree, stringstree)

def suite():
    suite = unittest.makeSuite(TestWorkSet, 'test')
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
