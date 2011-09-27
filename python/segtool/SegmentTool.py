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
modules = {'DrawingArea' : [0, '', 'none://DrawingArea.py' ], 
           'Section' : [0, '', 'none://Section.py' ]
           }
import wx
import os
import sys
import os.path

from lxml import etree

from Section import *
from DrawingArea import DrawingArea

class MainWindow (wx.Frame):
    def __init__ (self, parent, id, title):
        wx.Frame.__init__(self,parent,id, title, size=(600,600))
        self.tools = {}
        self.CreateDisplay()
        self.CreateMenu()
        self.CreateToolbar()

        self.nimages = 0
        self.image_nr = 0
        self.current_slice = None
        self.current_set = None
        self.current_set_name = None

        if len(sys.argv) > 1:
            self.__read_workspace(sys.argv[1])
        self.dirty = False
    
    def CreateMenu(self):
        menuBar = wx.MenuBar()
        Filemenu = wx.Menu()

        FileOpenSet = Filemenu.Append(wx.ID_NEW, "&New Set")
        FileOpenSet = Filemenu.Append(wx.ID_OPEN, "&Open Set")
        FileSaveSet = Filemenu.Append(wx.ID_SAVE, "&Save Set")
        FileSaveSetAs = Filemenu.Append(wx.ID_SAVEAS, "Save Set &As")
        Filemenu.AppendSeparator()

        FileOpenImage = Filemenu.Append(-1, "Import &Images")

        Filemenu.AppendSeparator()
        FileExit = Filemenu.Append(wx.ID_EXIT, "E&xit")

        menuBar.Append(Filemenu, "&File")

        Viewmenu = wx.Menu()
        ViewPrev = Viewmenu.Append(wx.ID_BACKWARD, "&Prev Frame\tctrl-left")
        ViewNext = Viewmenu.Append(wx.ID_FORWARD, "&Next Frame\tctrl-right")
        Viewmenu.AppendSeparator()
        ViewZoomIn = Viewmenu.Append(wx.ID_ZOOM_IN, "Zoom &in\tctrl-+")
        ViewZoomOut = Viewmenu.Append(wx.ID_ZOOM_OUT, "Zoom &out\tctrl--")
        ViewZoomFit = Viewmenu.Append(wx.ID_ZOOM_FIT, "Zoom &fit\tctrl+f")
        ViewZoom1to1 = Viewmenu.Append(wx.ID_ZOOM_100, "Zoom &1:1\tctrl-1")

        menuBar.Append(Viewmenu, "&View")

        Editmenu = wx.Menu()
        EditDelete =Editmenu.Append(wx.ID_DELETE, "&Delete all sections\tdel")
        EditRemove =Editmenu.Append(wx.ID_REMOVE, "&Restart current section \tesc")
        EditRotateSections =Editmenu.Append(-1, "&Rotate sections\tdel")
        EditDelSect =Editmenu.Append(-1, "Delete a &Section \tctrl-del")
        menuBar.Append(Editmenu, "&Edit")
        
        self.SetMenuBar(menuBar)
        self.Bind(wx.EVT_MENU, self.OnFileExit, FileExit)
        self.Bind(wx.EVT_MENU, self.OnFileOpenImage, FileOpenImage)
        self.Bind(wx.EVT_MENU, self.OnFileOpenSet, FileOpenSet)
        self.Bind(wx.EVT_MENU, self.OnFileSaveSet, FileSaveSet)
        self.Bind(wx.EVT_MENU, self.OnFileSaveSetAs, FileSaveSetAs)

        self.Bind(wx.EVT_MENU, self.OnNextFrame, ViewNext)
        self.Bind(wx.EVT_MENU, self.OnPrevFrame, ViewPrev)
        self.Bind(wx.EVT_MENU, self.OnDelete,    EditDelete)
        self.Bind(wx.EVT_MENU, self.da.RestartSection,    EditRemove)
        self.Bind(wx.EVT_MENU, self.OnDeleteSection, EditDelSect)
        self.Bind(wx.EVT_MENU, self.OnZoomIn,    ViewZoomIn)
        self.Bind(wx.EVT_MENU, self.OnZoomOut,   ViewZoomOut)
        self.Bind(wx.EVT_MENU, self.OnZoomFit,    ViewZoomFit)
        self.Bind(wx.EVT_MENU, self.OnZoom1to1,   ViewZoom1to1)
        self.Bind(wx.EVT_MENU, self.OnEditRotateSections,   EditRotateSections)


    def CreateToolbar(self):
        ap = wx.ArtProvider()
        toolbar = self.CreateToolBar()
        toolbar.AddSimpleTool(wx.ID_NEW, ap.GetBitmap(wx.ART_NEW, wx.ART_TOOLBAR), \
                                  "New", "Create a new working set")
        toolbar.AddSimpleTool(wx.ID_OPEN, ap.GetBitmap(wx.ART_FILE_OPEN, wx.ART_TOOLBAR),\
                                  "Open", "Open a new working set")
        toolbar.AddSimpleTool(wx.ID_SAVE, ap.GetBitmap(wx.ART_FILE_SAVE, wx.ART_TOOLBAR), \
                                  "Save", "Save the current working set")
        toolbar.AddSimpleTool(wx.ID_SAVEAS, ap.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_TOOLBAR), \
                                  "Save", "Save the current working set")
        toolbar.AddSeparator()
        
        toolbar.AddSimpleTool(wx.ID_BACKWARD, ap.GetBitmap(wx.ART_GO_BACK, wx.ART_TOOLBAR), \
                                  "Prev", "Go to previous frame")
        toolbar.AddSimpleTool(wx.ID_FORWARD, ap.GetBitmap(wx.ART_GO_FORWARD, wx.ART_TOOLBAR), \
                                  "Next", "Go to next frame")
        self.ImageNumSelect = wx.SpinCtrl(toolbar, 0, size=(50,-1), min=0, max=0, initial=0)
        toolbar.AddControl(self.ImageNumSelect)
        self.Bind(wx.EVT_SPINCTRL, self.OnChangeImage, self.ImageNumSelect)
        
        toolbar.AddSeparator()

        self.ToolSetStar = wx.RadioButton(toolbar, -1, "Star", style=wx.RB_GROUP)
        self.ToolDrawArea = wx.RadioButton(toolbar, -1, "Area")
        for r in [self.ToolSetStar, self.ToolDrawArea]:
            self.Bind(wx.EVT_RADIOBUTTON, self.OnToolSelect, r)

        toolbar.AddControl(self.ToolSetStar)
        toolbar.AddControl(self.ToolDrawArea)
        
        toolbar.AddSeparator()
        toolbar.AddSimpleTool(wx.ID_DELETE, ap.GetBitmap(wx.ART_DELETE, wx.ART_TOOLBAR), \
                                  "Delete", "Delete all sections in frame")
        toolbar.AddSimpleTool(wx.ID_REMOVE, ap.GetBitmap(wx.ART_UNDO, wx.ART_TOOLBAR), \
                                  "Restart current section")
       
        toolbar.AddSeparator()
        toolbar.AddSimpleTool(wx.ID_EXIT, ap.GetBitmap(wx.ART_QUIT, wx.ART_TOOLBAR),\
                                  "Exit", "Exit program")

        toolbar.Realize()
        
    def OnZoomIn(self, event):
        self.da.Zoom(2.0)
        self.scroll.SetVirtualSize(self.da.GetSize())

    def OnZoomOut(self, event):
        self.da.Zoom(0.5)
        self.scroll.SetVirtualSize(self.da.GetSize())

    def OnZoomFit(self, event):
        self.da.ZoomFit()
        self.scroll.SetVirtualSize(self.da.GetSize())

    def OnZoom1to1(self, event):
        self.da.Zoom1to1()
        self.scroll.SetVirtualSize(self.da.GetSize())

    def CreateDisplay(self):
        self.scroll = wx.ScrolledWindow(self,-1)
        self.da = DrawingArea(self.scroll, self)
        self.scroll.SetScrollRate(1,1)
        self.scroll.SetVirtualSize(self.da.GetSize())

    def change_slice(self):
        self.da.SaveSlice()
        self.current_slice = self.current_set.GetFrame(self.image_nr)
        self.da.SetSlice( self.current_slice  )

    def OnToolSelect(self, event):
        rs = event.GetEventObject()
        self.da.SetDrawTool(rs.GetLabel())

    def OnDeleteSection(self, event):
        colors = da.GetSectionColors()

    def OnEditRotateSections(self, event):
        self.da.SetSlice( self.current_slice.RotateSections()  )

    def OnDelete(self, event):
        self.da.DeleteSections()

    def OnNextFrame(self, event):
        if self.image_nr < self.nimages - 1:
            self.image_nr = self.image_nr + 1
            self.change_slice()
            self.ImageNumSelect.SetValue(self.image_nr)

    def OnPrevFrame(self, event):
        if self.image_nr > 0:
            self.image_nr = self.image_nr - 1
            self.change_slice()
            self.ImageNumSelect.SetValue(self.image_nr)

    def OnChangeImage(self, event):
        self.image_nr = self.ImageNumSelect.GetValue()
        if self.image_nr < self.nimages: 
            self.change_slice()

    def OnFileExit(self, event):
        result = wx.MessageDialog(self, "Save set before exiting?", "End program",
                               wx.CANCEL | wx.YES_NO | wx.ICON_QUESTION).ShowModal()
        if result == wx.ID_YES:
            self.SaveSet()
            self.Close(True)
        if result == wx.ID_NO:
            self.Close(True)
        

    def SetInitialImage(self):
        self.nimages = self.current_set.GetFrameNumber()
        self.ImageNumSelect.SetRange(0, self.nimages - 1)
        if self.nimages > 0:
            self.current_slice = self.current_set.GetFrame(0)
            self.da.SetSlice(self.current_slice)
            self.scroll.SetVirtualSize(self.da.GetSize())
            
    def OnFileOpenImage(self, event):
        wildcard = "PNG Images (*.png)|*.png|All Files (*.*)|*.*"
        flags = wx.OPEN | wx.MULTIPLE
        files = self.FileOpen("Open Image files", wildcard, True)
        
        if not files:
            return 
        
        self.current_set = WorkSet(images=files)
        self.SetInitialImage()
        
    def FileOpen(self, message, wildcard, multiple):
        if multiple: 
            flags = wx.OPEN |wx.MULTIPLE
        else:
            flags = wx.OPEN
        dialog = wx.FileDialog(self, message, os.getcwd(),"", wildcard, flags)
        if dialog.ShowModal() == wx.ID_OK:
            if multiple:
                files = dialog.GetPaths()
            else:
                files = dialog.GetPath()
        else:
            files = None 
        dialog.Destroy()
        return files

    def __read_workspace(self, file_name):
        self.current_set = WorkSet(xmlpath=file_name)
        if self.current_set:
            self.current_set_name = file_name
            self.SetInitialImage()

    def OnFileOpenSet(self, event):
        wildcard = "Workset (*.set)|*.set|All Files (*.*)|*.*"
        file_name = self.FileOpen("Open Working set", wildcard, False)
        if file_name:
            self.__read_workspace(file_name)
            
    def SaveSet(self):
        self.da.SaveSlice()
        data = self.current_set.Save()
        file = open(self.current_set_name, "w")
        file.write(data)
        file.close()
       
    def OnFileSaveSet(self, event):
        if not self.current_set_name:
            self.OnFileSaveSetAs(event)
        else:
            self.SaveSet()
            
    def OnFileSaveSetAs(self, event):
        wildcard = "Workset Images (*.set)|*.set"
        flags = wx.SAVE
        dialog = wx.FileDialog(self, "Save Workset", os.getcwd(),"", wildcard, wx.SAVE)
        if dialog.ShowModal() == wx.ID_OK:
                self.current_set_name = dialog.GetPath()
                if self.current_set_name[-4:0] != ".set":
                    print "append '.set'"
                    self.current_set_name = self.current_set_name + ".set"
                rootpath = os.path.dirname(self.current_set_name)
                self.current_set.RemoveImagepath(rootpath)
                self.SaveSet()
        dialog.Destroy()

class App(wx.App):
    def OnInit(self):
        self.frame = MainWindow(parent=None, id=-1, title='Segment')
        self.frame.Show()
        self.SetTopWindow(self.frame)
        

        return True

if __name__ == '__main__':
    app = App()
    app.MainLoop()
