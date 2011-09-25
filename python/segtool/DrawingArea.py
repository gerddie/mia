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

from Section import Section
from Section import GlobalColors
from Section import Star
#
# this class implements a viewer for images and 
# the tools to draw a colored overlay for segmentation
#

class DrawTool:
    def __init__(self, cursor):
        self.curLines = []
        self.pos = None
        self.Coursor = cursor

    def Clear(self):
        result = len(self.curLines) > 0
        self.curLines = []
        self.pos = None
        return result

    def ClearLast(self):
        result = len(self.curLines) > 1
        if result:
            self.curLines = self.curLines[0:-1]
            self.pos = self.curLines[-1]
        return result

    def DrawCurrent(self, dc):
        dc.SetPen(wx.Pen("white", 2, wx.SOLID))
        if len(self.curLines) > 1:
            p1 = self.curLines[0]
            for p2 in self.curLines[1:]:
                coords = p1 + p2
                dc.DrawLine(*coords)
                p1 = p2
        elif len(self.curLines) > 0:
            p1 = self.curLines[0]
            dc.DrawPoint(p1[0], p1[1])

    def LeftMouseDown(self, event, dc):
        dc.SetPen(wx.Pen("white", 2, wx.SOLID))
        if self.pos: 
            newPos = event.GetPositionTuple()
            coords = self.pos + newPos
            dc.DrawLine(*coords)
            self.pos = newPos
        else:
            self.pos = event.GetPositionTuple()
            dc.DrawPoint(self.pos[0], self.pos[1])
        pos = (self.pos[0], 
               self.pos[1])

        self.curLines.append(pos)
            
    def GetCursor(self): 
        return self.Coursor

class DrawStar(DrawTool):

    def __init__(self):
        DrawTool.__init__(self,wx.StockCursor(wx.CURSOR_CROSS))

    def RightMouseDown(self, parent):
        if len(self.curLines) == 3:
            parent.star = Star(points=self.curLines)
        self.pos = None
        self.curLines = []




class DrawAreas(DrawTool):

    def __init__(self):
        DrawTool.__init__(self,wx.StockCursor(wx.CURSOR_BULLSEYE))

                        
    def RightMouseDown(self, parent):
        if len(self.curLines) > 2:
            parent.sections.append(Section(GlobalColors[parent.index], self.curLines))
        self.curLines = []
        self.pos = None

        parent.index = parent.index + 1
        if parent.index > 5:
            parent.index = 0
        parent.Pen = wx.Pen(GlobalColors[parent.index], 1, wx.SOLID)

class DrawingArea (wx.Panel):
    def __init__ (self, parent, frame):
        wx.Panel.__init__(self, parent, size=(400,400), 
                          style=wx.WANTS_CHARS)
        self.set = None
        self.current_image = None
        self.sections = []
        self.index = 0
        self.Pen = wx.Pen(GlobalColors[self.index], 1, wx.SOLID)
        self.__init_key_lookup()
        self.wheel_scroll = 0
        self.parent  = parent
        self.frame = frame
 
        self.dragging = False

        self.star = None
        self.Tools = {}
        self.Tools["Star"] = DrawStar()
        self.Tools["Area"] = DrawAreas()
        self.SetDrawTool("Star")
        self.zoom = -1.0

        self.ResizeBuffer()

        self.Bind(wx.EVT_CHAR, self.OnKeyDown)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_IDLE, self.OnIdle)

        self.Bind(wx.EVT_MOUSEWHEEL, self.OnMouseWheel)
        self.Bind(wx.EVT_RIGHT_DOWN, self.OnMouseRightDown)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseLeftDown)
        self.Bind(wx.EVT_MIDDLE_DOWN, self.OnMouseMiddleDown)
        self.Bind(wx.EVT_MIDDLE_UP, self.OnMouseMiddleUp)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)



    def __init_key_lookup(self):
        self.__key_loopkup = {
            wx.WXK_ESCAPE:self.__Reset_Line,
            wx.WXK_SPACE: self.__Drop_Point,
            }
        
    def __Reset_Line(self, event):
        self.current_tool.Clear()

    def __Drop_Point(self, event):
        dc = wx.BufferedDC(wx.ClientDC(self), self.buffer)
        dc.SetPen(self.Pen)
        self.current_tool.LeftMouseDown(event,dc)
        

    def ResizeBuffer(self):
        size = self.GetClientSize()
        self.buffer = wx.EmptyBitmap(size.width, size.height)
        self.InitBuffer()

    def SetDrawTool(self, name):
        self.current_tool = self.Tools[name]

    def InitBuffer(self):
        dc = wx.BufferedDC(None, self.buffer)
        dc.Clear()
        self.DrawCurrentBitmap(dc)
        self.DrawStar(dc)
        self.current_tool.DrawCurrent(dc)
        self.Drawsections(dc)
        self.reInitBuffer = False
        
    def OnSize(self, size):
        self.ResizeBuffer()

    def OnIdle(self, dummy):
        if self.reInitBuffer:
            self.InitBuffer()
            self.Refresh(False)

        

    def OnPaint(self, event):
        dc=wx.BufferedPaintDC(self, self.buffer)

    def OnKeyDown(self,event):
        keycode = event.GetKeyCode()
        if self.__key_loopkup.has_key(keycode):
            self.__key_loopkup[keycode](event)
            self.InitBuffer()
            self.Refresh(False)
        else:
            print "unhandled key"
        event.skip()

    def OnMouseWheel(self, event):
        self.wheel_scroll = self.wheel_scroll + event.GetWheelRotation()

        if self.wheel_scroll >= event.GetWheelDelta():
            self.frame.OnNextFrame(event)
            self.wheel_scroll = self.wheel_scroll - event.GetWheelDelta()
        elif self.wheel_scroll <= -event.GetWheelDelta():
            self.frame.OnPrevFrame(event)
            self.wheel_scroll = self.wheel_scroll + event.GetWheelDelta()

    def OnMouseRightDown(self, event):
        self.current_tool.RightMouseDown(self)
        self.reInitBuffer = True
        self.Refresh(False)

    def OnMouseMiddleDown(self, event):
        self.dragging = True
        self.mouse_start_pos = self.parent.CalcScrolledPosition(event.GetPositionTuple())

    def OnMouseMiddleUp(self, event):
        self.dragging = False

    def OnMouseMove(self, event):
        if self.dragging:
            pos = self.parent.CalcScrolledPosition(event.GetPositionTuple())
            org = self.parent.GetViewStart()
            goto_x = org[0] + self.mouse_start_pos[0] - pos[0]
            goto_y = org[1] + self.mouse_start_pos[1] - pos[1]
            if  goto_x < 0:
                goto_x = 0
            if  goto_y < 0:
                goto_y = 0
            self.mouse_start_pos = pos
            self.parent.Scroll(goto_x, goto_y)
            

    def OnMouseLeftDown(self, event):
        dc = wx.BufferedDC(wx.ClientDC(self), self.buffer)
        dc.SetPen(self.Pen)
        self.current_tool.LeftMouseDown(event,dc)
        
    def DrawCurrentBitmap(self, dc):
        if self.current_image:
            dc.DrawBitmap(self.current_image, 0, 0, True)

    def Zoom(self, zoom_factor):
        self.SaveSlice()
        assert(zoom_factor > 0)
        self.zoom = self.zoom * zoom_factor
        self.SetSlice(self.set)

    def ZoomFit(self):
        self.SaveSlice()
        self.zoom = -1.0
        self.SetSlice(self.set)

    def Zoom1to1(self):
        self.SaveSlice()
        old_zoom = self.zoom
        self.zoom = 1.0
        self.SetSlice(self.set)

    def SetSlice(self, slice):
        self.set = slice
        image = slice.GetImage()
        if image: 
            if self.zoom < 0:
                size = self.GetClientSize(); 
                zw = 1.0 * size.width / image.GetWidth(); 
                zh = 1.0 * size.height / image.GetHeight(); 
                self.zoom = zw
                if zw > zh: 
                    self.zoom = zh
            self.current_image = image.Scale(self.zoom * image.GetWidth(), 
                                             self.zoom * image.GetHeight()).ConvertToBitmap()

        self.SetSize(self.current_image.GetSize())
        self.sections = slice.GetSections(self.zoom)
        self.star = slice.GetStar(self.zoom)
        self.index = len(self.sections)
        if self.index > 5:
            self.index = 0

        self.Pen = wx.Pen(GlobalColors[self.index], 1, wx.SOLID)
        self.InitBuffer()
        self.Refresh(False)

    def SaveSlice(self):
        self.set.SetSections(self.sections, self.zoom)
        if self.star:
            self.set.SetStar(self.star, self.zoom)

    def Drawsections(self, dc):
        for s in self.sections:
            s.Draw(dc)

    def DrawStar(self, dc):
        if self.star:
            dc.SetPen(wx.Pen("gray", 1, wx.DOT))
            rays = self.star.GetRayLines(self.GetClientSize())
            dc.SetBrush(wx.Brush("gray", wx.TRANSPARENT))
            dc.DrawCircle(self.star.center[0], 
                          self.star.center[1],
                          self.star.GetRadius())
            
            for r in rays:
                dc.DrawLine(r[0][0],
                            r[0][1],
                            r[1][0],
                            r[1][1])

    def DeleteSections(self):
        self.Clearsections()
        self.current_tool.Clear()
        self.InitBuffer()
        self.Refresh(False)

    def Clearsections(self):
        self.sections = []
        self.index = 0
        self.Pen = wx.Pen(GlobalColors[self.index], 1, wx.SOLID)
        
    def RestartSection(self,event):
        if not self.current_tool.ClearLast():
            if len(self.sections) > 0:
                self.sections = self.sections[0:-1]            
                self.index = len(self.sections)
        self.Pen = wx.Pen(GlobalColors[self.index], 1, wx.SOLID)
        self.InitBuffer()
        self.Refresh(False)

    def GetSectionColors(self):
        colors = []
        for s in self.sections:
            colors.append(s.color)
        return colors


    def DeleteSectionWithColor(self, color):
        new_sections = []
        for s in self.sections:
            if s.color != color:
                new_sections.append(s)
        self.sections = new_sections
        self.InitBuffer()
        self.Refresh(False)
