#!/usr/bin/env python

"""
fastAnalysis.py for use during UT testbeam data taking

To use:
1.  Set up TbUT running environment (https://twiki.cern.ch/twiki/bin/view/LHCb/TbUT)
2.  Edit this script to point to your current eos mount point and the data dir for current testbeam, or use the command line options
3.  From Tb/TbUT directory, run scripts/fastAnalysis.py with the following required options:
    -b board    : Letter+number combination, e.g. 'A8'
    -p pednum   : run number of the pedestal run to use (UT run number)
    -r runnum   : run number to analyze (UT run number) -- enter 0 for pedestal analysis only
4.  These are optional arguments:
    -n nevmax   : number of events to analyze
    -f force    : Force overwrite of any output files existing
    -t senstype : sensor type ('PType' or 'NType')
    -e eosmount : eos mount point for data to use (uses as a simple path)
    -i indir    : input path relative to eosmount point to find data
    -o outdir   : Directory to put the monitoring in
5.  Inside outdir it will make monitoring directories, and save the pedestal .dat, root files, and defined plots there
6.  It will not re-run the analysis if the files exist already unless you use -f
7.  After running it will pop open a window to view the new plots, and exit when that window is closed
"""

############################################################
##Modify this to change defaults: 
nevdef=100000
eosmount='/afs/cern.ch/work/m/mrudolph/private/nov_testbeam/eos'
indir='lhcb/testbeam/ut/OfficialData/October2015'
outdir='.'
############################################################






############################################################
##Define all the draw commands for simple monitoring plots from Cluster tuple here
##Noise mean and width are automatically created as the first 2 plots
##The last plot will be the ADC in the fiducial region (that gets defined based on previous draw command postprocessing)
#Potential TODO: create a new module file with the plot configuration to make extension a bit easier

class draw_command():
    """draw command skeleton for a TTree
    
    Need at least command, cut, option, title for histogram to make
    Can also bind postprocessor functions to draw more stuff or
    to return to the list of fiducial cuts for the final plot
    """
    def __init__(self, command, cut, option, title, postprocessor=None):
        self.command = command
        self.cut = cut
        self.option = option
        self.title = title
        self.postprocessor = postprocessor


##Post processing functions for histograms defined in draw commands -- expect TPad, TTree, TH1 parameters
##  Code below could be used for a couple things -- you have the pad so you can process the hist and draw more things,
##    or you can return a cut string that will be used to define the fiducial region later on

def post_nostat( pad, tree, hist ):
    """Generic postprocessor to remove stat box"""
    hist.SetStats(False)

def post_clusterpos( pad, tree, hist ):
    """Process the cluster position histogram and find beam area"""
    post_nostat( pad, tree, hist)

    maxbin = hist.GetMaximumBin()
    maxval = hist.GetBinContent(maxbin)
    thresh = 0.25
    #look to the left and then to the right
    currbin = maxbin
    while( hist.GetBinContent(currbin) > thresh*maxval ):
        currbin -= 1
    left = hist.GetXaxis().GetBinLowEdge(currbin)
    currbin = maxbin
    while( hist.GetBinContent(currbin) > thresh*maxval ):
        currbin += 1
    right = hist.GetXaxis().GetBinUpEdge(currbin)
    
    hist.GetYaxis().SetRangeUser(0,maxval*1.1)
    from ROOT import TLine
    l1 = TLine( left ,0, left,maxval*1.1)
    pad.posl1=l1
    l1.Draw()
    l2 = TLine( right,0,right,maxval*1.1)
    pad.posl2=l2
    l2.Draw()
    pad.Update()
    return "clustersPosition > {} && clustersPosition < {}".format(left,right)

def post_tdc_prof( pad, tree, hist ):
    """Process the charge v. TDC profile and find timing cut"""
    ymin = hist.GetMinimum()
    if ymin < 0:
        ymin *= 1.2
    else:
        ymin *= 0.8
    ymax = hist.GetMaximum()
    if ymax < 0:
        ymax *= 0.8
    else:
        ymax *= 1.2
    hist.GetYaxis().SetRangeUser(ymin,ymax)
    isneg = (ymax < 0) or ( ymin < 0 and abs(ymin) > abs(ymax) )
    if isneg:
        b = hist.GetMinimumBin()
    else:
        b = hist.GetMaximumBin()

    from ROOT import TLine
    l1 = TLine( b - 1.5 ,ymin, b - 1.5,ymax)
    pad.tdcl1=l1
    l1.Draw()
    l2 = TLine( b+1.5,ymin,b+1.5,ymax)
    pad.tdcl2=l2
    l2.Draw()
    return "abs(clustersTDC - {}) <= 1.5".format(b)

############################################################
##The list of draw commands to run
##Make sure for each command if you specify anything about the histogram to draw to, give it a unique name!
## Fiducial adc name is "fidadc", noise are called "meanNoise" and "widthNoise"
drawcommands = [ draw_command("clusterNumberPerEvent","","","Number of clusters;N_{clusters};Events"),
                 draw_command("clustersCharge>>charge(200,-1200,0)","clustersSize > 0","","Cluster charge;Tot. charge [ADC];Clusters"),
                 draw_command("clustersPosition","clustersSize > 0","","Cluster position;Channel;Clusters",postprocessor=post_clusterpos),
                 draw_command("clustersSize","clustersSize > 0", "","Cluster size;N_{strips};Clusters"),
                 draw_command("clustersTDC>>tdc(10,0.5,10.5)","","","TDC;TDC;Events",postprocessor=post_nostat),
                 draw_command("clustersCharge:clustersTDC>>tdcp(10,0.5,10.5)","clustersSize > 0","prof","Charge v TDC;TDC;<charge> [ADC]",postprocessor=post_tdc_prof),
 ]

##Should be possible to hook into an external file with TTree::MakeProxy using these commands as well if needed, though it hasn't been tested.
##If you have other histograms to calculate from available inputs, there are two lists (hlist_pre and hlist_post) in the plotting loop they can be added to;
##  see where the noise histograms are created.  Pre goes before the ttree command and post afterwards
############################################################


import argparse
    
parser = argparse.ArgumentParser(
    description = "Fast analysis of UT testbeam data for quality monitoring",
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,)
parser.add_argument('-b','--board',type=str,required=True,
                    help="Letter+number combination, e.g. 'A8'")
parser.add_argument('-p','--pednum',type=int,required=True,
                    help="Run number of the pedestal run to use (UT run number)")
parser.add_argument('-r','--runnum',type=int,required=True,
                    help="Run number to analyze (UT run number); use 0 to analyze pedestal only")
parser.add_argument('-n','--nevmax',type=int,required=False,default=nevdef,
                    help="Max number of events to analyze")
parser.add_argument('-f','--force',required=False,action='store_true',
                    help="Force overwrite of any output files existing")
parser.add_argument('-t','--senstype',type=str,required=False,default='PType',
                    help="Sensor type ('PType' or 'NType')")
parser.add_argument('-e','--eosmount',type=str,required=False,default=eosmount,
                    help="Eos mount point")
parser.add_argument('-i','--indir',type=str,required=False,default=indir,
                    help='Input data directory path relative to eosmount')
parser.add_argument('-o','--outdir', type=str, required=False,default=outdir,
                    help="Output directory for root files and plots")
args = parser.parse_args()

import sys
import os
import subprocess

#make sure I have places for output
pedestaldir = args.outdir + '/Pedestal-Board{}-{}'.format( args.board, args.pednum )
monitordir = args.outdir + '/Monitoring-Board{}-{}'.format( args.board, args.runnum )
def ensuredir(dir):
    try: 
        os.makedirs(dir)
    except OSError:
        if not os.path.isdir(dir):
            raise
ensuredir(pedestaldir)
if(args.runnum!=0):
    ensuredir(monitordir)

#Run pedestal if it does not already exist
didPedestal = False
if(args.force or not os.path.isfile("{}/Fast-Pedestal-Board{}-{}.dat".format( pedestaldir, args.board, args.pednum ) ) ):
    
    print "================================================================"
    print "==ANALYZE PEDESTAL FOR BOARD {} RUN {}=========================".format( args.board, args.pednum )
    print "================================================================"

    didPedestal = True

    ##Set up the gaudi config for running the pedestal
    pedCode = ("import sys\n"
               "sys.path.append( 'options/python' )\n"
               "from TbUTPedestalRunner import TbUTPedestalRunner\n"
               "app=TbUTPedestalRunner()\n"
               "# set parameter\n"
               "app.inputData= '{}/{}/Board{}/RawData/Pedestal-B{}-{}-{}-.dat'\n"
               "app.isAType={}\n"
               "# have to be more than 4k (~10k)\n"
               "app.eventMax={}\n"
               "#  keep the pedestals files in $KEPLERROOT/../TbUT/options/UT/ directory !!!!!\n"
               "app.pedestalOutputData ='{}/Fast-Pedestal-Board{}-{}.dat'\n"
               "app.runPedestals()\n").format(args.eosmount,args.indir, args.board, args.board[1:], args.board[:1],args.pednum, (args.board[:1]=='A'), args.nevmax, pedestaldir, args.board,args.pednum)


    with open('myTempPedRun.py','w') as ftarget:
        ftarget.write(pedCode)

    ret = subprocess.call(['gaudirun.py','myTempPedRun.py'])

    os.remove('myTempPedRun.py')

    if(ret!=0):
        sys.exit("Bad return code from pedestal run")

#Determine if we have a data run to process
if( args.runnum != 0):

    #find the data file by run number
    dir_to_search = "{}/{}/Board{}/RawData/".format(args.eosmount,args.indir,args.board)
    paths = subprocess.check_output("find {} -iname '*-{}-*.dat'".format(dir_to_search,args.runnum), shell=True).splitlines()

    if( len(paths)==0):
        sys.exit("ERROR: no data file found for run number {}".format(args.runnum))
    elif( len(paths) > 1):
        print "WARNING: more than one file matching run number, using",paths[0]

    inpath = paths[0]
    #These are the output names used by TbUTClusterizator
    outnames = [ os.path.basename(inpath).replace('.dat','.root'), os.path.basename(inpath).replace('.dat','_Tuple.root') ]

    #Skip running if files exist unless forced
    if( args.force or not (os.path.isfile(monitordir+'/'+outnames[0]) and os.path.isfile(monitordir+'/'+outnames[1])) ):

        print "================================================================"
        print "==ANALYZE DATA FOR BOARD {} RUN {}=============================".format( args.board, args.runnum )
        print "================================================================"

        runCode = ("import sys\n"
                   "sys.path.append('options/python')\n"
                   "from TbUTClusterizator import TbUTClusterizator\n"
                   "app = TbUTClusterizator()\n"
                   "# set parameters\n"
                   "app.inputData = '{}'\n"
                   "app.isAType = {}\n"
                   "app.sensorType = '{}'\n"
                   "app.eventMax = {}\n"
                   "app.pedestalInputData = '{}/Fast-Pedestal-Board{}-{}.dat'\n"
                   "app.eventNumberDisplay = 1000\n"
                   "app.runClusterization()\n").format(inpath,(args.board[:1]=='A'),args.senstype,args.nevmax,pedestaldir,args.board,args.pednum)



        with open('myTempRun.py','w') as ftarget:
            ftarget.write(runCode)

        #run twice because of noise input issue...
        ret = subprocess.call(['gaudirun.py','myTempRun.py'])
        ret = subprocess.call(['gaudirun.py','myTempRun.py'])

        #Move them since currently can't control output location from the run
        if( monitordir != '.' ):
            subprocess.call(['mv',outnames[0],monitordir+'/'])
            subprocess.call(['mv',outnames[1],monitordir+'/'])

        os.remove('myTempRun.py')

        if(ret!=0):
            sys.exit("Bad return code from analysis run")




########################################################
##Analysis part
########################################################


print "================================================================"
print "==PLOT DATA FOR BOARD {} RUN {}================================".format( args.board, args.runnum )
print "================================================================"

#Define what extensions to save plots with
def saveall( c, outname ):
    for ext in '.png', '.pdf', '.C':
        c.SaveAs(outname+ext)

#setup ROOT
from ROOT import gROOT, TH1F, TCanvas, TFile, TTree, gStyle, TF1,gDirectory,gPad
gROOT.SetBatch()
gROOT.SetStyle("Plain")

#keep track of plots we made to open at the end
plotlist = []

if( didPedestal ):
    c = TCanvas()
    #analyze the pedestal only
    #dat file is space separated list of pedestals by channel number
    peds = open("{}/Fast-Pedestal-Board{}-{}.dat".format(pedestaldir,args.board,args.pednum), "r")
    try:
        pedvals = [float(p) for p in peds.read().split()]
    except:
        sys.exit("Bad pedestal data format")
    
    nc = len(pedvals)
    hped = TH1F("hped","Pedestal v. channel;Channel;Pedestal [ADC]",nc,-0.5, nc-0.5)
    for i,v in enumerate(pedvals):
        hped.SetBinContent(i+1, v)

    hped.Draw();
    saveall(c,"{}/pedestal".format(pedestaldir))
    plotlist += ["{}/pedestal.png".format(pedestaldir)]

#analyze the run
if( args.runnum != 0 ):
    fhists = TFile("{}/{}".format(monitordir,outnames[0]))
    ftrees = TFile("{}/{}".format(monitordir,outnames[1]))

    t = ftrees.Get("TbUT/Clusters")
    
    #make the rough noise plots -- use the range -120 to 120 ADC and ignore most of the signal contribution
    noise2d = fhists.Get("TbUT/CMSData_vs_channel")
    noise2d.GetYaxis().SetRangeUser(-120,120)
    nc = noise2d.GetNbinsX()

    meanNoise = TH1F("meanNoise","Mean noise;Channel;<noise> [ADC]",nc,-0.5,nc-0.5)
    widthNoise = TH1F("widthNoise","Width noise;Channel;#sigma_{noise} [ADC]",nc,-0.5,nc-0.5)
    meanNoise.SetStats(False)
    widthNoise.SetStats(False)

    for b in range(1, nc+1):
        noise2d.GetXaxis().SetRange(b,b)
        meanNoise.SetBinContent(b, noise2d.GetMean(2))
        meanNoise.SetBinError(b, noise2d.GetMeanError(2))
        widthNoise.SetBinContent(b, noise2d.GetStdDev(2))
        widthNoise.SetBinError(b, noise2d.GetStdDevError(2))

    #hlist_pre is a list of histograms to draw
    hlist_pre = [meanNoise,widthNoise]
    npre = len(hlist_pre)

    #hlist_post, however, is a list of functions that create histograms; define some functions for post-draw histograms first:
    def fiducialADC( t, fidcut ):
        print "Use fiducial cut:", fidcut
        t.Draw("clustersCharge>>fidcharge(200,-1200,0)", fidcut)
        h = t.GetHistogram()
        h.SetTitle("Fiducial cluster charge;Tot. charge [ADC];Clusters")
        return h

    #the output ntuple has empty clusters saved in every event, so always cut those out
    fidcut = "clustersSize > 0"
    hlist_post = [fiducialADC]
    npost = len(hlist_post)

    #create the canvases
    #keep track of how many plots left to do
    ntreedraw = len(drawcommands)
    nplotall = npre + ntreedraw + npost

    #9 plots per canvas max
    ncanvas = nplotall/9
    #if not an exact divisor we need an extra one for the leftovers:
    if( nplotall % 9 != 0 ):
        ncanvas += 1
    #how many columns and rows we want for each number of plots per canvas
    colnums = [1,2,2,2,3,3,3,3,3]
    rownums = [1,1,2,2,2,2,3,3,3]
    
    clist = [TCanvas('c{}'.format(i)) for i in range(ncanvas)]
    plotidx = 0 #count the plots
    for c in clist:
        #how many plots on this canvas?
        #we'll use nplotall as a countdown
        if( nplotall >= 9):
            nplot=9
            nplotall -= 9
        else:
            nplot=nplotall
    
        #setup the canvas based on number of plots to draw
        ncols = colnums[nplot - 1]
        nrows = rownums[nplot - 1]
        c.SetCanvasSize( 600*ncols, 300*nrows )
        c.Divide(ncols,nrows)

        for i in range(nplot):
            #plotidx is overall iterator, i is current canvas iterator
            if (plotidx < npre):
                c.cd(i+1)
                hlist_pre[plotidx].Draw()
            elif(plotidx < npre + ntreedraw):
                c.cd(i+1)
                t.Draw( drawcommands[plotidx-npre].command, drawcommands[plotidx-npre].cut, drawcommands[plotidx-npre].option )
                h = t.GetHistogram()
                gPad.h = h
                h.SetTitle(drawcommands[plotidx-npre].title)
                if(drawcommands[plotidx-npre].postprocessor):
                    newcut = drawcommands[plotidx-npre].postprocessor( gPad, t, h )
                    if type(newcut) == str:
                        fidcut += " && " + newcut
            else:
                c.cd(i+1)
                hlist_post[plotidx - npre - ntreedraw](t,fidcut).Draw()

            plotidx+=1
                
        plotpath = "{}/monitoring-{}".format( monitordir, c.GetName()[1:] )
        saveall(c, plotpath)
        plotlist += ["{}.png".format(plotpath)]

#open up an eye of gnome window to look at the plots; use the next and previous arrows to navigate between them
if( plotlist ):
    subprocess.call(["eog",]+plotlist) 

