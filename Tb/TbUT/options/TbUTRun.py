__author__ = 'ja'


import sys
sys.path.append("options/python")
from TbUTClusterizator import TbUTClusterizator

app = TbUTClusterizator()
# set parameters
app.inputData = "/afs/cern.ch/user/c/cbetanco/work/LHCb/Run_Angle_Scan-M3-FanIn-265-15160.dat"
app.isAType = False
app.sensorType = "NType"

app.eventMax = 100000
app.pedestalInputData = "$KEPLERROOT/../TbUT/options/UT/Pedestal-M3-FanIn-266.dat"
app.eventNumberDisplay = 10

app.runClusterization()


