import os,sys
import Runs
jobnr    = 2870

#Set blocks to run over and alignment-method
blocks   = ['D2']
method   = 'run'
#


gangadir   = '/afs/cern.ch/work/c/chombach/gangadir/workspace/chombach/LocalXML/%i/' % jobnr
rootout    = '/afs/cern.ch/work/c/chombach/Telescope/ResStudies/' 
for bl in blocks:
    rootout += bl
    rootout +=  '/'
    os.system( 'mkdir -p %s' % rootout )
def findRunNo( so ):
    sof = open( so )
    for ll in sof.readlines():
        l = ll.split()
        if len(l) == 0:
            continue
        if l[0] == 'TbDataSvc':
            ind = l[4].find('Run')
            return l[4][ind+3:ind+7]

runs = Runs.Runs(blocks, method)

if method == 'run':
    runs.setOutputDir( rootout )
    
for dd in os.listdir(gangadir):
    if dd == 'debug':
        continue
    rn = findRunNo(gangadir+dd+'/output/stdout')
    run = runs.findRun( rn )
    
    os.system('cp %s %s' % ( gangadir+dd+'/output/Alignment_out.dat', run.ALIGNOUTFILE) )
    if method == 'run':
        outfile = runs.OUTPUTDIR+'Kepler_%s_%s_%s_%s.root' % (rn, run.DUT, run.BIAS, run.ANGLE)
        os.system('cp %s %s' % ( gangadir+dd+'/output/Kepler-histos.root', outfile) )

