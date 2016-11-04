__author__ = 'ja'
import sys
sys.path.append( "options/python" )

from TbUTPedestalRunner import TbUTPedestalRunner
app=TbUTPedestalRunner()
# set parameter
app.inputData='/afs/cern.ch/user/c/cbetanco/work/LHCb/Pedestal-M3-FanIn-266.dat'
app.isAType=False
# have to be more than 4k (~10k)
app.eventMax=100000
#  keep the pedestals files in $KEPLERROOT/../TbUT/options/UT/ directory !!!!!
app.pedestalOutputData ="$KEPLERROOT/../TbUT/options/UT/Pedestal-M3-FanIn-266.dat"

app.runPedestals()

