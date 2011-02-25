#!/usr/bin/env python
modules = {'Section' : [0, '', 'none://Section.py' ]
           }

from Section import *

from lxml import etree
import unittest

class TestImageSegmentation(unittest.TestCase):
    def setUp(self):
         self.points = [[(0,0), (1,2), (4,12)],[(1,2), (2,3), (5,2)]]
         self.colors = ["red","yellow"]

         Sections = []
         for i in range(2):
             Sections.append(Section(color=self.colors[i],points=self.points[i]))

         fn='testimage.png'
         self.iseg = ImageSegmentation(image=fn, sections=Sections)
         self.star = Star(value=( (120, 64), [(0.0, 1.0), (1.0, 0.5), (0.5, 1.0)], 0.1 ))
         self.iseg.SetStar(self.star, 1.0)
         
    def testEqualSelf(self):
        self.assertEqual(self.iseg, self.iseg, msg="iseg is not equal to itself")
        
    def testSave(self):
        set = etree.Element("workset")
        self.iseg.Save(set)
        self.assertEqual(len(set), 1)
        for frame in set:
            loaded = ImageSegmentation(node=frame)
            self.assertEqual(loaded.image_file, self.iseg.image_file)
            self.assertEqual(len(loaded.Sections), len(self.iseg.Sections))
            for s in loaded.Sections:
                self.assert_(s in loaded.Sections, "not all sections loaded")

            self.assertEqual(loaded, self.iseg, msg=etree.tostring(set, pretty_print=True))

    def testGetSection(self):
        sections = self.iseg.GetSections(2.0)
        self.assertEqual(len(sections), 2)
        for i in range(len(sections)):
            self.assertEqual(sections[i].color, self.colors[i])
            p = sections[i].points
            sp = self.points[i]
            self.assertEqual(len(p), len(self.points[i]))
            for k in range(len(p)): 
                self.assertEqual(p[k][0], sp[k][0] * 2)
                self.assertEqual(p[k][1], sp[k][1] * 2)

                
    def testSetSection(self):
        sections = self.iseg.GetSections(2.0)
        self.iseg.SetSections(sections, 4.0)
        sections = self.iseg.GetSections(1.0)

        self.assertEqual(len(sections), 2)
        for i in range(len(sections)):
            self.assertEqual(sections[i].color, self.colors[i])
            p = sections[i].points
            sp = self.points[i]
            self.assertEqual(len(p), len(self.points[i]))
            for k in range(len(p)): 
                self.assertEqual(p[k][0], sp[k][0] * 0.5)
                self.assertEqual(p[k][1], sp[k][1] * 0.5)

    def testEmptySet(self):
        frame = etree.Element("frame", image="testimage.png")
        frame.append(etree.Element("placeholder"))
        ImageSegmentation(node=frame)

    def testSetStar(self):
         self.iseg.SetStar(self.star, 4.0)
         self.assertEqual(self.iseg.star.center, (30,16))
         
    def testGetStar(self):
        star =  self.iseg.GetStar(2.0)
        self.assertEqual(star.center, (240,128))


def suite():
    suite = unittest.makeSuite(TestImageSegmentation, 'test')
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
         

