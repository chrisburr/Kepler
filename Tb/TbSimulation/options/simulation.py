# Run with:
# gaudirun.py $KEPLERROOT/options/simulation.py

from Gaudi.Configuration import *

from Configurables import Kepler

# Set the number of events to run over 
Kepler().EvtMax = 100

# Change the sequence of code that is run ---------------
def sim():
  seq = GaudiSequencer("Telescope")
 
  # TestMC options.
  from Configurables import TbTestMC
  TbTestMC().doMisAlign = False
  TbTestMC().NTracks = 200
  TbTestMC().RunDuration = 10000
  TbTestMC().NNoiseClusters = 0
  TbTestMC().HitTimeJitter = 20
  TbTestMC().ClusterPosnError = 0.001
  TbTestMC().ClusterADCMPV = 300.0 # Ish.
  TbTestMC().ChargeSharingWidth = 0.033 # Best not to change much.
  TbTestMC().ThresholdCut = 20
  TbTestMC().ForceEfficiency = True
  #TbTestMC().InitAlignment = "Alignment_perfect.dat"
  #TbTestMC().InitAlignment = Alignment_raw.dat

  # TbClustering options.   
  from Configurables import TbClustering  
  TbClustering().TimeWindow = 200

  # TbTracking options.
  from Configurables import TbTracking
  TbTracking().TimeWindow = 300
  TbTracking().SearchRadius = 1
  TbTracking().MinNClusters = 7
  TbTracking().Monitoring = True
  TbTracking().SearchVolume = "cylinder" # {cylinder, diabolo}
  
  # TbTracking speed options.
  TbTracking().nComboCut = 1000 # O(10) for speed.
  TbTracking().SearchVolumeFillAlgorithm = "adap_seq" # {seq, adap_seq}. Adap faster.
  
  seq.Members = [TbTestMC(), TbClustering(), TbTracking()]
  seq = GaudiSequencer("Monitoring")
  from Configurables import TbClusterPlots, TbTrackPlots
  seq.Members = [TbClusterPlots(), TbTrackPlots()]
  
appendPostConfigAction(sim)


