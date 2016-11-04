# Basic configuration file. 
# Execute with: gaudirun.py $KEPLERROOT/options/example.py
from Gaudi.Configuration import *
from Configurables import Kepler

# Set the path to the directory/files to be processed
path = 'eos/lhcb/testbeam/velo/timepix3/'
RUN = '9187'
if int(RUN) < 2000:
  path += 'July'
elif int(RUN) < 2815:
  path += 'Oct'
elif int(RUN) < 4000:
  path += 'Nov'
else: 
  path += 'Dec'
path += '2014'

Kepler().InputFiles =  [path + '/RawData/Run' + RUN + '/']
Kepler().PixelConfigFile = ["eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run1236/Conditions/PixelConfig.dat"]
Kepler().AlignmentFile = path + '/RootFiles/Run' + RUN + '/Conditions/Alignment' + RUN + 'mille.dat'
#Kepler().AlignmentFile = 'ResStudies/Alignments/dut/Alignment2509.dat'
Kepler().HistogramFile= 'Kepler_histos_' + RUN + '.root'

# Set the number of events to run over 
Kepler().EvtMax = 1000

# Set the configuration of the individual algorithms, e. g.
from Configurables import TbEventBuilder
#TbEventBuilder().MinPlanesWithHits = 5

from Configurables import TbClustering
TbClustering().PrintConfiguration = True

from Configurables import TbTracking
TbTracking().PrintConfiguration = True

