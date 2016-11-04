from ROOT import *

#gROOT.ProcessLine(".x lhcbStyle.C")
gStyle.SetPalette(1)

infile = TFile("Kepler-histos.root")
infile.cd("Tb/TbClusterPlots")

nPlanes = 8
efficiencies = []
for i in range(nPlanes):
  histoname = "Plane" + repr(i)
  hTotal = gDirectory.Get("Positions/" + histoname)
  hPassed = gDirectory.Get("PositionsAssociated/" + histoname)
  eff = TEfficiency(hPassed, hTotal)
  eff.SetDirectory(0)
  efficiencies.append(eff)
infile.Close()

gStyle.SetPaintTextFormat("3.2g")
c = TCanvas("c", "c", 1200, 600)
c.Divide(4, 2, 0, 0)
for i in range(nPlanes):
  c.cd(i + 1)
  efficiencies[i].Draw("col")
  #efficiencies[i].Draw("textsame")
  efficiencies[i].Paint("col")
  efficiencies[i].GetPaintedHistogram().GetXaxis().SetTitle("#font[12]{x} [mm]")
  efficiencies[i].GetPaintedHistogram().GetYaxis().SetTitle("#font[12]{y} [mm]")
  efficiencies[i].GetPaintedHistogram().SetMaximum(1.0)
  c.Update()
 
