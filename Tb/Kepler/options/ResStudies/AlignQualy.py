from ROOT import TFile, TCanvas
from Runs import Runs

import os, sys
block  = ['D1','D2']
indir  = '/afs/cern.ch/work/c/chombach/Telescope/ResStudies/'
outdir = '/afs/cern.ch/user/c/chombach/www/public/Telescope/AlignQualy/'

os.system('mkdir -p '+outdir)

outfile = outdir+'AlignQualy'
for bl in block:
    outfile += '_'+bl
outfile += '.pdf'
runs = Runs(block, 'run')
runs.setOutputDir(indir)

c = TCanvas('c','c',1200,600)
c.Print(outfile+'(','pdf')

def makePlots(tf, title):

    c.Divide(0,0)
    fn = 'Tb/TbTrackPlots/'
    chi2 = tf.Get(fn+'Chi2PerDof')
    chi2.SetTitle(title)
    chi2.Draw()
    c.Print(outfile,'pdf')

    prob = tf.Get(fn+'Probability')
    prob.SetTitle(title)
    prob.Draw()
    c.Print(outfile,'pdf')

    c.Clear()
    c.Divide(5,2)
    for i in range(9):
        c.cd(i+1)
        res_x = tf.Get(fn+'BiasedResiduals/GlobalX/Plane%i' % i )
        res_x.SetTitle(title+' GlobalX Plane%i' % i)
        res_x.Fit('gaus')
        res_x.Draw()
    c.Print(outfile,'pdf')
    c.Clear()

    c.Divide(5,2)
    for i in range(9):
        c.cd(i+1)
        res_y = tf.Get(fn+'BiasedResiduals/GlobalY/Plane%i' % i )
        res_y.SetTitle(title+' GlobalY Plane%i' % i)
        res_y.Fit('gaus')
        res_y.Draw()
    c.Print(outfile,'pdf')
    c.Clear()

    c.Divide(5,2)
    for i in range(9):
        c.cd(i+1)
        res_xvsx = tf.Get(fn+'BiasedResiduals/GlobalResXvsLocalX/Plane%i' % i )
        res_xvsx.SetTitle(title+' GlobalXResvsX Plane%i' % i)
        res_xvsx.Draw()
    c.Print(outfile,'pdf')
    c.Clear()

    c.Divide(5,2)
    for i in range(9):
        c.cd(i+1)
        res_yvsy = tf.Get(fn+'BiasedResiduals/GlobalResYvsLocalY/Plane%i' % i )
        res_yvsy.SetTitle(title+' GlobalYResvsY Plane%i' % i)
        res_yvsy.Draw()
    c.Print(outfile,'pdf')
    c.Clear()
for block in runs.RUNS:
    for run in runs.RUNS[block]:
        rn  = run.RUN
        ang = run.ANGLE
        bia = run.BIAS
        dut = run.DUT
        block = run.BLOCK
        title = '%s_%s_%s_%s' % (rn, dut, bia, ang)
        fn = runs.OUTPUTDIR+block+'/Kepler_%s.root' % title
        tf = TFile(fn)
        makePlots(tf, title)
        tf.Close()
c.Print(outfile+')','pdf')


