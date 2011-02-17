#!/usr/bin/env python
modules = {'Section' : [0, '', 'none://Section.py' ]
           }

import wx
from Section import Section
from Section import ImageSegmentation
from Section import GlobalColors
from lxml import etree
import unittest

class TestSectionIO(unittest.TestCase):
    def setUp(self):
         self.points = [(0,0), (1,2), (4,12)]
         self.points_upscaled = [(0,0), (2,4), (8,24)]
         color = "red"
         self.section = Section(color=color,points=self.points)
         
    def testEqualSelf(self):
        self.assertEqual(self.section, self.section)

    def testModel(self):
        root = etree.Element("frame")
        self.section.Save(root)
        
        self.assertEqual(len(root), 1, 
                         msg=etree.tostring(root, pretty_print=True))
        for s in root:
            load_section = Section(node=s)
            self.assertEqual(load_section, self.section, msg=etree.tostring(s, pretty_print=True) )

    def testGetPoints(self):
        points = self.section.GetPoints( 2.0 )
        self.assertEqual(points, self.points_upscaled)

    def testSetPoints(self):
        self.section.SetPoints( self.points_upscaled, 2.0 )
        self.assertEqual(self.section.points, self.points)
        
    def testGetMask(self):
        app = wx.App()
        points =  [(1.0, 1.0), (3.0,1.0), (3.0, 2.5), (1.0,2.0)]
        section = Section(color="black",points=points)
        mask_points = [(1.0, 1.0), (2.0,1.0), (3.0, 1.0), 
                       (1.0, 2.0), (2.0,2.0), (3.0, 2.0)]
        
        mask = section.GetMask(wx.Size(4,4), 1.0)

        self.assertEqual(len(mask), len(mask_points))
        for p in mask_points: 
            assert(p in mask)

    def testGetMaskZoomed(self):
        app = wx.App()
        points =  [(1.0, 1.0), (3.0,1.0), (3.0, 1.5)]
        section = Section(color="black",points=points)
        mask_points = [(2.0, 2.0), (3.0, 2.0), (4.0, 2.0), (5.0, 2.0), (6.0, 2.0),
                                               (4.0, 3.0), (5.0, 3.0),(6.0, 3.0)]
        
        mask = section.GetMask(wx.Size(8,8), 2.0)

        self.assertEqual(len(mask), len(mask_points))
        for p in mask_points: 
            assert(p in mask)


    def testGetMask2(self):
        app = wx.App()
        points =  [(1.0, 1.0), (4.0,1.0), (4.0, 4.0), (1.0, 4.0)]
        section = Section(color="black",points=points)
        mask_points = [(1.0, 1.0), (1.0, 2.0), (1.0, 3.0), (1.0, 4.0), 
                       (2.0, 1.0), (2.0, 2.0), (2.0, 3.0), (2.0, 4.0), 
                       (3.0, 1.0), (3.0, 2.0), (3.0, 3.0), (3.0, 4.0), 
                       (4.0, 1.0), (4.0, 2.0), (4.0, 3.0), (4.0, 4.0)]
        
        mask = section.GetMask(wx.Size(6,6), 1.0)
        print mask

        self.assertEqual(len(mask), len(mask_points))
        for p in mask_points: 
            assert(p in mask)
        

def suite():
    suite = unittest.makeSuite(TestSectionIO, 'test')
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
         

