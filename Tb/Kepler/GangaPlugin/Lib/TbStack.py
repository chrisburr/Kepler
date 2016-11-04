from Ganga.GPIDev.Base import GangaObject
from Ganga.GPIDev.Adapters.IPostProcessor import PostProcessException, IPostProcessor
from Ganga.GPIDev.Base.Proxy import GPIProxyObject
from Ganga.GPIDev.Schema import ComponentItem, FileItem, Schema, SimpleItem, Version
from Ganga.Utility.Config import makeConfig, ConfigError, getConfig
from Ganga.Utility.Plugin import allPlugins
from Ganga.Utility.logging import getLogger, log_user_exception
import commands
import copy
import os
import string
import re
from ROOT import *

def displace(th1, dist=0,name=""):
  graph = TGraphErrors()
  counter=0
  for x in range( 0 , th1.GetNbinsX() ) :
   # print "Setting point %d %f %f" %( x, dist + x*th1.GetBinWidth(0), th1.GetBinContent(x) )
    if th1.GetBinContent(x) == 0 : continue
    graph.SetPoint( counter , dist + x*th1.GetBinWidth(0) , th1.GetBinContent(x) )
    graph.SetPointError( counter, 0, th1.GetBinError(x) )
    counter = counter + 1
  return graph
class TbStack(GangaObject):

  _category = 'postprocessor'
  _exportmethods = ['merge']
  _name = 'TbStack'
  _schema = Schema(Version(1,0), {
    'Output' : SimpleItem(defvalue="Kepler-Meta-Histos.root"),
    'Labels' : SimpleItem(defvalue={}),
    'DrawOptions' : SimpleItem(defvalue=""),
    'StackOptions' : SimpleItem(defvalue=""),
    'Hist' : SimpleItem(defvalue={}),
    'Displace' : SimpleItem(defvalue={}),
    'Title' : SimpleItem(defvalue="")
  })
  def merge(self, job):
    out_file = TFile(self.Output,"UPDATE")
    for key in self.Hist:
      print "Making histogram: " + key
      histogram_name = self.Hist[key]
      stack = TMultiGraph(key, self.Title ) 
      legend = TLegend(0.8,0.3,0.995,0.4)
      counter=0
      color_counter=1
      xtitle=""
      ytitle=""
      for run in job.splitter.Files.keys():
        if run not in self.Labels.keys(): 
          counter = counter + 1
          continue 
          
        j=job.subjobs(counter)
        print "Reading " + j.name + " output files"
        in_file = TFile(j.outputdir+"Kepler-histos.root" ,'READ')
        
        if in_file.IsOpen() == False: continue
        obj = in_file.Get(histogram_name)
        if obj == 0: continue 
        gROOT.cd()
        dd = 0
        if run in self.Displace.keys(): dd = self.Displace[run]
        if self.Title == "": title = obj.GetTitle()
        if xtitle == "": xtitle = obj.GetXaxis().GetTitle()
        if ytitle == "": ytitle = obj.GetYaxis().GetTitle()
        thing = displace( obj, dd , j.name )
        thing.SetLineColor( color_counter )
        thing.SetLineWidth( 1 )
        stack.Add( thing , self.DrawOptions )
        legend.AddEntry( thing, self.Labels[ run ],"L")
        in_file.Close()
        del in_file
        counter = counter + 1
        color_counter = color_counter + 1 
      stack.SetDrawOption("nostack")
      stack.ls()
      out_file.cd()
      stack.Write()
      legend.Write()
      c1 = TCanvas(key+"_canvas","",800,600)
      # c1.SetLogy() 
      stack.Draw(self.StackOptions)
      print "Titles = %s %s %s" %( title, xtitle, ytitle)
      stack.GetXaxis().SetTitle(xtitle)
      stack.GetYaxis().SetTitle(ytitle)
      stack.SetTitle(self.Title) 
      stack.Draw(self.StackOptions)
      legend.Draw()
      c1.Write()
    

