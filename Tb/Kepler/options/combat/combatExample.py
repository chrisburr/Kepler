# Basic configuration file. 
# Execute with: gaudirun.py $KEPLERROOT/options/combatExample.py
from Gaudi.Configuration import *
from Configurables import Kepler


# Set the path to the directory/files to be processed
#Kepler().InputFiles = ["/afs/cern.ch/user/v/vifranco/cmtuser/KEPLER/KEPLER_HEAD/"]

# Set the alignment file
Kepler().AlignmentFile = "/afs/cern.ch/user/v/vifranco/public/CombatAlignment_2arms.dat"

# Set the masked pixels file
Kepler().PixelConfigFile = "Tb/Kepler/options/EmptyPixelMask.dat"

# Set the number of events to run over 
Kepler().EvtMax = 10000


# Set the configuration of the individual algorithms
from Configurables import TbClustering
TbClustering().PrintConfiguration = True

from Configurables import TbTracking
TbTracking().PrintConfiguration = True


# Combat specific.

#CombatInputFile 0 is for the lower arm and File1 for the higher File2 (if using one arm, use comat File0)
#Keep the alignment file consistent, use the same number of planes everywhere, also change the number of arms.
def combatRun():
     from Configurables import TbCombatBuilder
     TbCombatBuilder().CombatInputFile0 = "/afs/cern.ch/user/v/vifranco/public/Run31100003_SAMPLE.txt"
     TbCombatBuilder().ReadoutFormat = "RelaxD"
#     TbCombatBuilder().CombatInputFile1 = "/afs/cern.ch/user/v/vifranco/cmtuser/KEPLER/KEPLER_HEAD/240614_30V_TH500_100ms_merge.dat"

     seq = GaudiSequencer("Telescope")
     seq.Members = [TbCombatBuilder(), TbClustering(), TbTracking()]

     TbCombatBuilder().PrintFreq = 5000
     TbCombatBuilder().NumberArms = 2
     
     TbTracking().MinNClusters = 6
     TbTracking().SearchRadius = 2
     TbTracking().VolumeAngle = 0.0015
     TbTracking().SearchPlanes = [3,5]
     TbTracking().SearchVolume = "diabolo"
     TbTracking().MaxChi2 = 2000000.
     TbTracking().ViewerEventNum = 100
     TbTracking().ViewerTimeLow = 0
     TbTracking().ViewerTimeUp = 2000000
     #TbTracking().ViewerOutput = True
     TbTracking().CombatRun = True
     
     seq = GaudiSequencer("Monitoring")
     from Configurables import TbHitMonitor, TbClusterPlots, TbTrackPlots
     seq.Members = [TbHitMonitor(), TbClusterPlots(),TbTrackPlots()]
     TbTrackPlots().ParametersResidualsXY = ("", -1.0, 1.0, 200)
     TbTrackPlots().ParametersXY = ("", -2, 16, 200)
     TbClusterPlots().ParametersXY = ("", -2, 16, 200)

appendPostConfigAction(combatRun)
