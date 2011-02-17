#!/usr/bin/env python
modules = {'Section' : [0, '', 'none://Section.py' ]
           }

from Section import Section
from Section import Star
from Section import ImageSegmentation


from lxml import etree
import unittest
from math import sqrt

class TestStar(unittest.TestCase):
    def setUp(self):
        self.h = sqrt(3.0)
        self.l = sqrt(self.h * self.h / 9.0 + 1)
        help = self.h/(3.0 * self.l)
        self.testrays = [( 0.0         ,  1.0 ), 
                         ( 1.0 / self.l,  help), 
                         ( 1.0 / self.l, -help)]
        self.a = 1.0 / self.l / help
        self.r = self.h * 2.0 / 3.0
        
    def testCreation(self):
        
        star = Star(points=[(0.0,self.h), (-1.0,0.0),(1.0,0.0)])        
        center = star.GetCenter()
        self.assertAlmostEqual(center[0], 0.0)
        self.assertAlmostEqual(center[1], self.h / 3.0)
        
        rays = star.GetRays()
        self.assertEqual(len(rays), 3)
        
        for i in range(3):
            self.assertAlmostEqual(rays[i][0], self.testrays[i][0])
            self.assertAlmostEqual(rays[i][1], self.testrays[i][1])


    def testCreation2(self):

        star = Star(points=[(0.0,self.h), (0.0, -self.h/3.0),(1.0,0.0)])        
        center = star.GetCenter()
        self.assertAlmostEqual(star.GetRadius(), self.r)
        self.assertAlmostEqual(center[0], 0.0)
        self.assertAlmostEqual(center[1], self.h / 3.0)
        
        rays = star.GetRays()
        self.assertEqual(len(rays), 3)
        for i in range(3):
            self.assertAlmostEqual(rays[i][0], self.testrays[i][0])
            self.assertAlmostEqual(rays[i][1], self.testrays[i][1])


    def testCreation3(self):
        star = Star(value=( (128, 128), self.testrays, self.r ))
        rays = star.GetRays()
        self.assertEqual(len(rays), 3)
        for i in range(3):
            self.assertAlmostEqual(rays[i][0], self.testrays[i][0])
            self.assertAlmostEqual(rays[i][1], self.testrays[i][1])

    def testDrawRays(self):
        star = Star(value=( (120, 64), [(0.0, 1.0), (1.0, 0.5), (0.5, 1.0)], 1.0 ))
        rays = star.GetRayLines( (512, 500))
        self.assertEqual(len(rays), 3)
        
        self.assertAlmostEqual(rays[0][0][0], 120 )
        self.assertAlmostEqual(rays[0][0][1], 0   )
        self.assertAlmostEqual(rays[0][1][0], 120 )
        self.assertAlmostEqual(rays[0][1][1], 500 )

        self.assertAlmostEqual(rays[1][0][0],    0 )
        self.assertAlmostEqual(rays[1][0][1],    4 )
        self.assertAlmostEqual(rays[1][1][0],  512 )
        self.assertAlmostEqual(rays[1][1][1],  260 )

        self.assertAlmostEqual(rays[2][0][0], 120 - 32 )
        self.assertAlmostEqual(rays[2][0][1],   0      )
        self.assertAlmostEqual(rays[2][1][0], 120 + 218)
        self.assertAlmostEqual(rays[2][1][1], 500      )

    def testSaveLoad(self):
        root = etree.Element("frame")
        star = Star(value=( (120, 64), [(0.0, 1.0), (1.0, 0.5), (0.5, 1.0)], 1.0 ))
        star.Save(root)

        self.assertEqual(len(root),1)
        for n in root:
            load_star = Star(node=n)
        self.assertEqual(star, load_star, msg=etree.tostring(root, pretty_print=True))


def suite():
    suite = unittest.makeSuite(TestStar, 'test')
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')

    
