# Basic configuration file. 
# Execute with: gaudirun.py $KEPLERROOT/options/example.py
from Gaudi.Configuration import *
from Configurables import Kepler
import pickle
import ROOT

# Set the path to the directory/files to be processed
path = 'eos/lhcb/testbeam/velo/timepix3/'
RUN = '9110'
# if int(RUN) < 2000:
#   path += 'July'
# elif int(RUN) < 2815:
#   path += 'Oct'
# elif int(RUN) < 4000:
#   path += 'Nov'
# else: 
#   path += 'Dec'
# path += '2014'

path += 'July2015'

Kepler().InputFiles =  [path + '/RawData/Run' + RUN + '/']
Kepler().PixelConfigFile = ["eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run1236/Conditions/PixelConfig.dat"]
Kepler().AlignmentFile = path + '/RootFiles/Run' + RUN + '/Conditions/Alignment' + RUN + 'mille.dat'
#Kepler().AlignmentFile = 'ResStudies/Alignments/dut/Alignment2509.dat'
Kepler().HistogramFile= 'Kepler_histos_' + RUN + '.root'
# Set the number of events to run over 
Kepler().EvtMax = 11000000
#Kepler().TimingConfigFile = "myTimingConfig.dat"

# Set the configuration of the individual algorithms, e. g.
from Configurables import TbEventBuilder 
#TbEventBuilder().MinPlanesWithHits = 5

from Configurables import TbClustering
TbClustering().PrintConfiguration = True

from Configurables import TbSimpleTracking
TbSimpleTracking().PrintConfiguration = True

from Configurables import TbVisualiserOutput
TbVisualiserOutput().PrintConfiguration = True
TbVisualiserOutput().ViewerEvent = 1049
Kepler().UserAlgorithms = [TbVisualiserOutput()]

DUT_id = pickle.load( open( "DUTnum.p", "rb" ) )
#DUT_id = 3


fResiduals = ROOT.TFile("UnbiasedLocalResolutionsPerPlane.root")
gx = fResiduals.Get("xPerPlane")
gy = fResiduals.Get("yPerPlane")
scaling = 15
print ((gx.GetBinContent(DUT_id+1)**2 + gy.GetBinContent(DUT_id+1)**2)**0.5)
allowance = scaling * ((gx.GetBinContent(DUT_id+1)**2 + gy.GetBinContent(DUT_id+1)**2)**0.5)
print 'DUT_id', DUT_id
print 'allowance:', allowance

def egRun():
    from Configurables import TbEventBuilder, TbTrackPlots, TbCalibration, TbVisualiserOutput
    from Configurables import TbClustering, TbClusterPlots, TbSimpleTracking, TbHitMonitor
    from Configurables import TbClusterAssociator, TbEfficiency, TbDUTMonitor
    
    TbEventBuilder().PrintFreq = 10
    
    TbSimpleTracking().TimeWindow = 6
    TbSimpleTracking().MaxDistance = 0
    TbSimpleTracking().MinPlanes = 7
    TbSimpleTracking().MaskedPlanes = [DUT_id, 4]
    TbSimpleTracking().MaxOpeningAngle = 0.005
    TbSimpleTracking().RecheckTrack = True
    TbSimpleTracking().ChargeCutLow = 0
    TbSimpleTracking().DoOccupancyCut = True
    TbSimpleTracking().MaxClusterSize = 20
    TbSimpleTracking().MaxOccupancy = 7 # not inclusive
    TbTrackPlots().MaskedPlanes = [DUT_id, 4]
        
    TbClusterAssociator().DUTs = [DUT_id]
    TbDUTMonitor().DUTs = [DUT_id]
    TbClusterAssociator().TimeWindow = 10
    TbClusterAssociator().XWindow = allowance
    
    TbEfficiency().CheckHitDUT = True
    TbEfficiency().CheckHitAlivePixel = True
    TbEfficiency().DUT = DUT_id
    TbEfficiency().TakeDeadPixelsFromFile = True
    TbEfficiency().nTotalTracks =500000
    TbEfficiency().MaxChi = 2
    
    
    seq = GaudiSequencer("Telescope")
    seq.Members = [TbEventBuilder(), TbClustering(), TbSimpleTracking(), TbClusterAssociator(), TbEfficiency()]    
    #seq.Members = [TbEventBuilder()]  
    
    seq = GaudiSequencer("Monitoring")
    from Configurables import TbHitMonitor, TbClusterPlots, TbTrackPlots 
    #seq.Members = [TbHitMonitor(), TbClusterPlots(), TbTrackPlots(), TbDUTMonitor(), TbVisualiserOutput()]
    seq.Members = [TbDUTMonitor()]
    
appendPostConfigAction(egRun)