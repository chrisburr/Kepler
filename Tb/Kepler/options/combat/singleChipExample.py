# Basic configuration file. 
# Execute with: gaudirun.py singleChipExample.py

from Gaudi.Configuration import *

from Configurables import Kepler

# Set the path to the directory/files to be processed
Kepler().InputFiles = ["singleChipRun/"]
# Set the alignment file
Kepler().AlignmentFile = "singleAlignment.dat"
# Set the masked pixels file
Kepler().PixelConfigFile = "PixelMask.dat"
# Set the number of events to run over 
Kepler().EvtMax = 4300

# Set the configuration of the individual algorithms
from Configurables import TbClustering
TbClustering().PrintConfiguration = True

# Combat specific.
def combatRun():
     from Configurables import TbEventBuilder, TbClustering
     seq = GaudiSequencer("Telescope")
     seq.Members = [TbEventBuilder(), TbClustering()]
     
     seq = GaudiSequencer("Monitoring")
     from Configurables import TbHitMonitor, TbClusterPlots, TbTrackPlots
     seq.Members = [TbHitMonitor(), TbClusterPlots()]

appendPostConfigAction(combatRun)
