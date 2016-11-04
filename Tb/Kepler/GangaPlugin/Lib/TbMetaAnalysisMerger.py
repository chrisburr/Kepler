from ROOT import *
from RecursiveSearch import RecursiveSearch, makeDirectories
import os
from Ganga.GPIDev.Base import GangaObject
from Ganga.GPIDev.Base.Proxy import GPIProxyObjectFactory
from Ganga.GPIDev.Schema import *
import operator

class TbBinning2D :
  name = ""
  title = ""
  minY = 0
  widthY = 0
  nBinsY = 0
  minX = 0 
  widthX = 0 
  nBinsX = 0 
  maxX = 0 
  maxY = 0 
  path = ""
  def __init__(self,name,title,path,minX,widthX,nBinsX,minY,widthY,nBinsY) :
    self.name = name
    self.title = title
    self.path = path
    self.minX = minX
    self.widthX = widthX
    self.nBinsX = nBinsX
    self.minY = minY
    self.widthY = widthY
    self.nBinsY = nBinsY
    self.maxX = self.minX + self.nBinsX*self.widthX
    self.maxY = self.minY + self.nBinsY*self.widthY
   
class TbProfile :
  path = ""
  title = ""
  name = ""
  def __init__(self,name,title,path):
    self.title = title
    self.path = path
    self.name = name
  

class TbMetaAnalysisMerger(GangaObject):

  _category = 'postprocessor'
  _exportmethods = ['merge']
  _name = 'TbMetaAnalysisMerger'
  f2L = {}
  _schema = Schema(Version(1,0), {
    'Mode' : SimpleItem(defvalue="Profile"),
    'Output' : SimpleItem(defvalue="Kepler-Meta-Histos.root"),
    'Top' : SimpleItem(defvalue=[]),
    'Labels' : SimpleItem(defvalue={})
  })

  def write2D( self, files, hists ):
    prev_path = ""
    for h in hists:
      if h.path != prev_path :
        print "Making histogram for: %s" %h.path
        prev_path = h.path
      out_file = TFile.Open(self.Output,'UPDATE')
      hist = TH2D( h.name, h.title, h.nBinsX, h.minX, h.maxX, h.nBinsY, h.minY, h.maxY )
      for f in files:
        in_file = TFile(f ,'READ')
        obj = in_file.Get(h.path +"/"+ h.name )
        if obj :
          if obj.GetEntries() != 0: obj.Scale(1/obj.GetEntries());
          for l in range(0, obj.GetNbinsX() ):
            hist.SetBinContent( self.f2L[f] - h.minX, l, obj.GetBinContent(l) )
          del obj
        else :
          print "ERROR: %s not found in %s" %(h.path + "/" + h.name,f)
        in_file.Close("R")
        del in_file
      out_file.cd(h.path)      
      hist.Write()
      out_file.Close("R")
      del out_file
 
  def write1D( self, files, graphs) :
    means = []
    sigmas = []
    sorted_f2L = sorted(self.f2L.items(), key=operator.itemgetter(1))
    p = 0
    n = {}
    for f in sorted_f2L:
      print f
      n[f[0]] = p
      p = p + 1
      
    print n
    for g in graphs:
      mean = TGraphErrors()
      mean.SetNameTitle( g.name + "_m", g.title)
      sigma = TGraphErrors()
      sigma.SetNameTitle( g.name + "_s", g.title)
      means.append( mean)
      sigmas.append( sigma )
  
    for f in files:
      in_file = TFile(f,'READ')
      for x in range(0,len(graphs)):
        print "Making histogram for: %s" %(g.path + "/" + g.name)
        g = graphs[x]
        obj = in_file.Get( g.path + "/" + g.name )
        if obj :
         if( obj.GetEntries() != 0 ):
            means[x].SetPoint( n[f] , self.f2L[f] , obj.GetMean() )
            sigmas[x].SetPoint( n[f], self.f2L[f] , obj.GetRMS() )
            means[x].SetPointError( n[f] , 0 , obj.GetRMS()/sqrt(obj.GetEntries()) )
            sigmas[x].SetPointError( n[f], 0 , obj.GetRMS()/sqrt(obj.GetEntries()) )
      in_file.Close("R")
      del in_file
    out_file = TFile.Open(self.Output,'UPDATE')

    for g in range(0,len(graphs)):
      out_file.cd(graphs[g].path)
      means[g].Write()
      sigmas[g].Write()
    out_file.Close()
  
  
  def merge(self, list ):
    
    file0 = TFile() 
    found = False
    files = []
    
    for j in list:
      if j.status == 'completed':
        fname = j.outputdir + "/Kepler-histos.root"
        if os.path.isfile(fname) :
          if found == False :
            file0 = TFile(fname ,'READ')
            found = True
          files.append(fname)
          if j.inputdata.run in self.Labels : 
            self.f2L[fname] = self.Labels[j.inputdata.run]
          else : 
            self.f2L[fname] = j.inputdata.run
      
    all_histograms = []
    print "Searching for histograms..."
    RecursiveSearch( file0, all_histograms , 7 )
    histograms = []
    for x in all_histograms: 
      for t in self.Top:
        if x.find(t) != -1 : 
          if x not in histograms : histograms.append(x)
      
    hists = []
    graphs = []
    print "Building histogram objects..."
    for x in histograms:
      obj = file0.Get( x )
      k = x.rfind('/')
      if self.Mode == "Profile":
        profile = TbProfile(x[k+1:], obj.GetTitle(), x[:k] )
        graphs.append( profile )
      if self.Mode == "2D":
        Runs = job.inputdata.Runs
        hists.append( TbBinning2D( x[k+1:],obj.GetTitle(),x[:k],Runs[0],1,Runs[len(Runs)-1] - Runs[0] +1,
                       obj.GetBinLowEdge(0), obj.GetBinWidth(0), obj.GetNbinsX() ) )
        
    file0.Close()
    del file0
    
    output = TFile.Open(self.Output,'RECREATE')
    output.Close("R")
    del output
    makeDirectories( self.Output, histograms )
    print "Filling %d histograms / graphs" %(len( hists ) + len(graphs ) )
    if self.Mode == "2D": self.write2D( files, hists )
    if self.Mode == "Profile": self.write1D( files, graphs )
    
