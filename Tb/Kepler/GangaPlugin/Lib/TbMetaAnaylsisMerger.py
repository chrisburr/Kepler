from ROOT import *
from RecursiveSearch import RecursiveSearch

 # main programme 
 


all_histograms = []
RecursiveSearch( f, all_histograms , 5 )

output = TFile('Meta-graphs.root','RECREATE')

# clone the directory structure

for x in all_histograms:
  directories = x.split('/')
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

output.Write()

output.Close()

output = TFile('Meta-graphs.root','UPDATE')

for x in all_histograms:
  obj = f.Get( x )

  k = x.rfind('/')
  path = x[:k]
  name = x[k+1:]
  output.cd( path )

  graph_mean = TGraph( )
  graph_mean.SetNameTitle( name + "_mean" , obj.GetTitle() )
  graph_mean.SetPoint( graph_mean.GetN() , 0, obj.GetMean() )
  graph_mean.Write()

  graph = TGraph( )
  graph.SetNameTitle( name + "_sigma" , obj.GetTitle() )
  graph.SetPoint( graph.GetN() , 0, obj.GetRMS() )
  graph.Write()



output.Write()
  