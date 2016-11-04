from ROOT import *

def RecursiveSearch( directory , histograms, maxDepth, path="", depth=0 ):
  if type( directory ) is not TDirectoryFile :
    if type( directory) is not TFile :
      if type( directory ) is TH1D :
        histograms.append( path[:-1]  )
      return
        
  if depth > maxDepth : return
  
  for i in range(0,directory.GetListOfKeys().GetEntries()):
    obj = directory.GetListOfKeys().At(i)
    RecursiveSearch( directory.Get( obj.GetName()) , histograms,maxDepth, path + obj.GetName() + "/" , depth+1)
    
  return

def makeDirectories( filename, objects):
  for x in objects:
    output = TFile.Open(filename,'UPDATE')
    directories = x.split('/')
    output.cd() 
    layer = output
    path = ""
    for ind in range( 0, len(directories) -1 ) :
      tmpKey = layer.FindKey( directories[ind] )
      if not tmpKey  :
        foo = layer.mkdir( directories[ind], directories[ind] )
        layer.cd()
        foo.Write()
        layer.Write()
        layer = foo
      else : layer = tmpKey.ReadObj()
    output.Close("R")
    del output 
