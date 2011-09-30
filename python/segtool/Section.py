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

import wx
import os.path
from lxml import etree
from string import atof
from string import atoi
from string import rsplit
import math

GlobalColors = ["white", "red", "blue", "green", "yellow", "cyan" ] 

def GetAverageMaskedIntensity(image, mask):
    val = 0.0
    n = len(mask)
    if n > 0:
        for m in mask:
            val = val + image.GetRed(m[0], m[1])
        return val / n
    else:
        return 0

class Section:
    def init_from_node(self, node):
        if node.tag != "section":
            raise ValueError("expected 'section' got '%s'" % (node.tag))
                
        self.points = []
        self.color = node.get("color")
        for n in node:
            if (n.tag == 'point'):
                x = atof(n.get("x"))
                y = atof(n.get("y"))
                self.points.append( (x,y) )


    def __init__(self, color=None, points=None, node=None):
        self.thickness = 1
        if node is not None:
            self.init_from_node(node)
        else:
            self.init_from_color_points(color,points)

    def init_from_color_points(self, color, points):
        if color is not None:
            self.color = color
        else:
            self.color = GlobalColors[0]
            
        if points:
            self.points = points
        else:
            self.points = []
        
    def Draw(self, dc):
        pen = wx.Pen(self.color, self.thickness, wx.SOLID)
        brush = wx.Brush(self.color, wx.TRANSPARENT)
        dc.SetPen(pen)
        dc.SetBrush(brush)
        dc.DrawPolygon(self.points, 0, 0)

    def Invert(self):
        self.points.reverse()


    def shift(self, dx, dy):
        points = []
        for p in self.points:
            points.append( (p[0] + dx, p[1] + dy ))
        self.points = points

    def Save(self, frame):
        section = etree.Element("section", color=self.color)
        frame.append(section)
        for p in self.points:
            cx = "%f" % (p[0])
            cy = "%f" % (p[1])
            section.append(etree.Element("point", x=cx, y=cy))

    def SetPoints(self, points, zoom):
        self.points = []
        for p in points:
            self.points.append( ( p[0] / zoom, p[1] / zoom) )
        
    def GetPoints(self, zoom):
        points = []
        for p in self.points:
            points.append( ( p[0] * zoom, p[1] * zoom) )
        return points

    def GetColor(self):
        return self.color

    def SetColor(self, color):
        self.color = color

    def GetMask(self, size, zoom):
        if zoom != 1.0:
            points = self.GetPoints(zoom)
        else:
            points = self.points
        
        buffer = wx.EmptyBitmap(size.x, size.y)

        dc = wx.MemoryDC()
        dc.SelectObject(buffer)
        dc.SetBrush(wx.Brush("white", wx.SOLID))
        dc.Clear()
        dc.SetPen(wx.Pen("black", 1, wx.SOLID))
        dc.SetBrush(wx.Brush("black", wx.SOLID))
        dc.DrawPolygon(points)
        dc.SelectObject(wx.NullBitmap)
        
        image = buffer.ConvertToImage()
        
        mask = []
        for y in range(size.y):
            for x in range(size.x):
                if image.GetRed(x,y) == 0:
                    mask.append((x,y))
        
        return mask

    def DrawSolid(self, dc, zoom):
        if zoom != 1.0:
            points = self.GetPoints(zoom)
        else:
            points = self.points
        
        dc.SetPen(wx.Pen(self.color, 1, wx.SOLID))
        dc.SetBrush(wx.Brush(self.color, wx.SOLID))
        dc.DrawPolygon(points)

                        
    def __eq__(self, other):
        if (self.color != other.color):
            return False
        if len(self.points) != len(other.points):
            return False
        return self.points == other.points


class Star:
    def __init__(self, points=None, value=None, node=None):
        if points is not None:
            self.__init_from_points(points)
        elif value is not None:
            self.__init_from_values(value)
        elif node is not None:
            self.__init_from_node(node)
        else:
            raise ValueError("No points given")


    def __init_from_node(self, node):
        if node.tag != "star":
            raise ValueError("expected 'star' got '%s'" % (node.tag))

        self.rays = []
        self.center = (atof(node.get("x")), atof(node.get("y")))
        rstring = node.get("r")
        if rstring is not None:
            self.radius = atof(rstring)
        else:
            self.radius = 1
        for n in node:
            if (n.tag == 'point'):
                sx = atof(n.get("x"))
                sy = atof(n.get("y"))
                self.rays.append( (sx,sy) )
        


    def __init_from_values(self, values):
        self.center = values[0]
        self.rays   = values[1]
        self.radius = values[2]

    def __len(self, a, b):
        x = a[0] - b[0] 
        y = a[1] - b[1] 
        return math.sqrt(x * x + y * y)

    def __calc_ray(self, point):
        x = point[0] - self.center[0]
        y = point[1] - self.center[1]
        norm = math.sqrt(x * x + y * y)
        x = x / norm
        y = y / norm
        return (x,y)

    def shift(self, dx, dy):
        self.center = (self.center[0] + dx, self.center[1] + dy)

    def Draw(self, dc, size, zoom):
        dc.SetPen(wx.Pen("gray", 1, wx.DOT))
        rays = self.GetRayLines(size)
        dc.SetBrush(wx.Brush("gray", wx.TRANSPARENT))
        dc.DrawCircle(zoom * self.center[0], 
                      zoom * self.center[1],
                      self.GetRadius())
            
        for r in rays:
            dc.DrawLine(zoom * r[0][0],
                        zoom * r[0][1],
                        zoom * r[1][0],
                        zoom * r[1][1])

    def __calc_bc(self, a,b,c):
        return a * a *( b * b + c * c - a *a )
        

    def __calc_circumcenter(self, p):
        if len(p) != 3:
            raise ValueError("Can only init from 3 points")
        
        
        a = self.__len(p[1], p[2])
        b = self.__len(p[2], p[0])
        c = self.__len(p[0], p[1])
        x = []
        x.append(self.__calc_bc(a, b, c))
        x.append(self.__calc_bc(b, c, a))
        x.append(self.__calc_bc(c, a, b))
        sum = x[0] + x[1] + x[2]
        for i in range(len(x)): 
            x[i] = x[i] / sum

        yy = 0.0
        yx = 0.0
        for i in range(len(x)):
            yx = yx + x[i] * p[i][0]
            yy = yy + x[i] * p[i][1]
        return (yx, yy)

    def __init_from_points(self, p):
        self.center = self.__calc_circumcenter(p)
        self.radius = self.__len(self.center, p[0])

        self.rays = []
        r = self.__calc_ray(p[0])


        self.rays.append( r )
        c = -0.5
        s = math.sqrt(0.75)

        ray = (  c * r[0] -  s * r[1],  c * r[1] +  s * r[0])
        if ray[0] > 0:
            self.rays.append( ray )
        else:
            self.rays.append( (-ray[0], -ray[1] ))

        ray = (  c * r[0] +  s * r[1],  c * r[1] -  s * r[0])
        if ray[0] > 0:
            self.rays.append( ray )
        else:
            self.rays.append( (-ray[0], -ray[1] ))


    def GetCenter(self):
        return self.center

    def GetRays(self):
        return self.rays

    def GetRadius(self):
        return self.radius

    def GetRayLines(self, size):
        lines = []
        for r in self.rays:
            if abs(r[0]) > abs(r[1]):
                a = r[1] / r[0]
                start = (0, self.center[1] - a * self.center[0])
                end   = (size[0], a * (size[0] - self.center[0]) +  self.center[1])
                lines.append((start, end))
            else:
                a = r[0] / r[1]
                start = (-a * self.center[1] + self.center[0], 0)
                end   = (a * (size[1] - self.center[1])  + self.center[0], size[1])
                lines.append((start, end))
        return lines
    
    def Save(self, root):
        cx = "%f" % (self.center[0])
        cy = "%f" % (self.center[1])
        cr = "%f" % (self.radius)

        star = etree.Element("star", x=cx, y=cy, r = cr )
        for r in self.rays:
            x = "%f" % (r[0])
            y = "%f" % (r[1])
            star.append(etree.Element("point", x=x, y=y ))
        root.append(star)
        
    def __eq__(self, other):
        if self.center != other.center:
            return False
        if self.radius != other.radius:
            return False
        return self.rays == other.rays
                   

def GetVariationValue(sum_sq, sum, n):
    if n > 1 : 
        return math.sqrt( (sum_sq - sum * sum / n ) / ( n - 1) )
    else:
        return 0

def GetVariation(image, mask):
    val = 0.0
    valsq = 0.0
    n = len(mask)
    for m in mask:
        v = image.GetRed(m[0], m[1])
        val = val + v
        valsq = valsq + v * v 
            
    return GetVariationValue(valsq, val, n)

class ImageSegmentation:
       
    def __init__(self, image=None, sections=None, node=None, basepath=None):
        self.star = None
        self.Sections = []
        if image is not None:
            self.__init_from_image_and_sections(image, sections)
        elif node is not None:
            self.__init_from_node(node, basepath)
        else:
            raise ValueError("No image file or node given")

        self.image =  wx.Image(self.image_file, type=wx.BITMAP_TYPE_ANY, index = -1)
        if self.image.GetSize().x == 0:
            raise ValueError("Image file %s not found" % (self.image_file))
    
    def __init_from_image_and_sections(self, image, sections):
        self.image_file = image

        if sections is not None:
            self.Sections = sections

    def __init_from_node(self, node, basepath):
        if node.tag != "frame":
            raise ValueError("Expected 'frame' got '%s' instead" % node.tag)
        
        nodedata = node.get("image")
        if basepath is None or os.path.isabs(nodedata):
            self.image_file = nodedata
        else:
            self.image_file = os.path.join(basepath, nodedata)

        for n in node:
            if n.tag == "section":
                self.Sections.append(Section(node=n))
            if n.tag == "star":
                self.star = Star(node=n)

    def RemoveImagepath(self, rootpath):
        splitted = os.path.split(self.image_file)
        if rootpath == splitted[0] or splitted[0] == "":
            self.image_file = splitted[1]

    def GetImage(self):
        return self.image

    def GetImageZoomed(self, zoom):
        zw = zoom * self.image.GetWidth()
        zh = zoom * self.image.GetHeight()
        return self.image.Scale(zw,zh)

    def Save(self, root):
        frame = etree.Element("frame", image=self.image_file)

        if self.star is not None:
            self.star.Save(frame)

        if len(self.Sections) > 0: 
            for sections in self.Sections:
                sections.Save( frame )
        else:
            frame.append(etree.Element("placeholder"))

        root.append(frame)

    def GetStar(self, zoom):
        if not self.star:
            return None
        new_c = (self.star.center[0] * zoom, self.star.center[1] * zoom)
        return Star( value=(new_c, self.star.rays, self.star.radius * zoom))
    
    def SetStar(self, star, zoom):
        new_c = (star.center[0] / zoom, star.center[1] / zoom)
        if self.star is not None:
            self.star.center = new_c
            self.star.rays = star.rays
            self.star.radius = star.radius/ zoom
        else:
            self.star = Star( value=(new_c, star.rays, star.radius/ zoom))
            

    def SetSections(self, sections, zoom):
        self.Sections = []
    
        for s in sections:
            self.Sections.append(Section(s.color, s.GetPoints(1.0/zoom)))


    def RotateSections(self):
        if not self.Sections:
            return 
        if len(self.Sections) < 2:
            return 
               
        c0 = self.Sections[0].GetColor()

        for i in range(len(self.Sections) - 1):
            c = self.Sections[i+1].GetColor()
            self.Sections[i].SetColor(c)

        self.Sections[len(self.Sections)-1].SetColor(c0)
        return self

    def GetMasksImage(self, with_image, zoom, style):
        image = self.GetImageZoomed(zoom)
        size = image.GetSize()
        buffer = wx.EmptyBitmap(size.x, size.y)

        dc = wx.MemoryDC()
        dc.SelectObject(buffer)
        dc.SetBrush(wx.Brush("black", wx.SOLID))
        dc.Clear()
        if with_image:
            dc.DrawBitmap(image.ConvertToBitmap(), 0,0, True)

        for s in self.Sections:
            s.DrawSolid(dc, zoom)
            
        dc.SelectObject(wx.NullBitmap)
        
        return buffer.ConvertToImage()
        
    def GetSections(self, zoom):
        sections = []
        
        for s in self.Sections:
            sections.append(Section(s.color, s.GetPoints(zoom)))
        
        return sections

    def GetMaskedIntensities(self, zoom):
        image = self.GetImageZoomed(zoom)
        size = image.GetSize()
        intensities = []
        for s in self.Sections:
            mask = s.GetMask(size, zoom)
            intensity = GetAverageMaskedIntensity(image, mask)
            intensities.append(intensity)
        return intensities

    def GetMaskedIntensityVariations(self, zoom):
        image = self.GetImageZoomed(zoom)
        size = image.GetSize()
        variations = []
        for s in self.Sections:
            mask = s.GetMask(size, zoom)
            variations.append(GetVariation(image, mask))
        return variations

    def Draw(self, dc, zoom, solid):
        image = self.GetImageZoomed(zoom)
        size = image.GetSize()

    def DrawToBitmap(self, image, sections, star):
        image = self.GetImageZoomed(1.0)
        size = image.GetSize()
        buffer = wx.EmptyBitmap(size.x, size.y)

        dc = wx.MemoryDC()
        dc.SelectObject(buffer)
        dc.SetBrush(wx.Brush("black", wx.SOLID))
        dc.Clear()
        if image:
            dc.DrawBitmap(image.ConvertToBitmap(), 0,0, True)
          
        if star:
            self.star.Draw(dc, size, 1.0)

        if sections:
            for s in self.Sections:
                s.Draw(dc)
            
        dc.SelectObject(wx.NullBitmap)
        
        return buffer.ConvertToImage()

    def DrawToBitmap2(self, sections):
        image = self.GetImageZoomed(1.0)
        size = image.GetSize()
        buffer = wx.EmptyBitmap(size.x, size.y)

        dc = wx.MemoryDC()
        dc.SelectObject(buffer)
        dc.SetBrush(wx.Brush("black", wx.SOLID))
        dc.Clear()
        if image:
            dc.DrawBitmap(image.ConvertToBitmap(), 0,0, True)
          
        for s in sections:
                s.Draw(dc,  (0,0))
            
        dc.SelectObject(wx.NullBitmap)
        
        return buffer.ConvertToImage()


    def DrawToBitmap2(self, sections):
        image = self.GetImageZoomed(1.0)
        size = image.GetSize()
        buffer = wx.EmptyBitmap(size.x, size.y)

        dc = wx.MemoryDC()
        dc.SelectObject(buffer)
        dc.SetBrush(wx.Brush("black", wx.SOLID))
        dc.Clear()
        if image:
            dc.DrawBitmap(image.ConvertToBitmap(), 0,0, True)
          
        for s in sections:
                s.SetColor("green")
                s.Draw(dc,  (0,0))

        if self.Sections:
            for s in self.Sections:
                s.SetColor("red")
                s.Draw(dc,  (0,0))

            
        dc.SelectObject(wx.NullBitmap)
        
        return buffer.ConvertToImage()
        
        

    def shift(self, dx, dy):
        for s in self.Sections:
            s.shift(dx,dy)
        self.star.shift(dx,dy)
       

    def __eq__(self, other):

        if len(self.Sections) != len(other.Sections):
            return False
        
        for s in self.Sections:
            if not (s in other.Sections):
                return False

        return  self.image_file == other.image_file


class WorkSet:
    def __init__(self, images=None, xmlpath=None):
        if xmlpath is not None:
            self.__init_from_xmlfile(xmlpath)
        else:
            self.__init(images)
        self.dirty = False

    def __init_from_xmlfile(self, xmlpath):
        file=open(xmlpath, "r")
        stringstree = file.read()
        root = etree.XML(stringstree)
        fileroot = os.path.dirname(xmlpath)
        self.__load(root, fileroot)

    def __load(self, workset, basename):
        self.frames = []
        if workset.tag != "workset":
            raise ValueError("Expected 'workset' got '%s'" % workset.tag)

        
        for f in workset:
            if f.tag == "frame":
                self.frames.append( ImageSegmentation( node=f, basepath=basename) )
            else:
                print "Ignore tag '%s'" % f.tag

    def __init(self, images):
        self.frames = []
        if not images:
            raise ValueError("No images given")
        for i in images:
            self.frames.append(ImageSegmentation(image=i))

    def GetFrameNumber(self):
        return len(self.frames)

    def GetFrameFileNum(self, i):
        if i > self.GetFrameNumber():
            raise ValueError("Requested frame out of range")
        frame = self.frames[i]
        filename = frame.image_file
        fileroot = rsplit(filename, '.', 1)
        number = fileroot[0][-4:]
        return atoi(number)
        
        

    def GetFrame(self, i):
        if i > self.GetFrameNumber():
            raise ValueError("Requested frame out of range")
        return self.frames[i]

    def GetFrameByFileNr(self, i):
        #first evaluate the pattern of the file name 
        file_name_format = GetFilenameFormat(self.frames[0].image_file)
        
    def RemoveImagepath(self, rootpath):
        for f in self.frames:
            f.RemoveImagepath(rootpath)
        

    def Save(self):
        root = etree.Element("workset")    
        for f in self.frames:
            f.Save(root)
        self.ClearDirty() 
        return etree.tostring(root, pretty_print=True)


    def shift(self, dx, dy):
        for f in self.frames:
            f.shift(dx,dy)
        self.SetDirty()

    def GetFilenameFormat(name):
        fileroot = rsplit(filename, '.', 1)
        

    def SetDirty(self):
        self.dirty = True

    def ClearDirty(self):
        self.dirty = False

    def IsDirty(self):
        return self.dirty

