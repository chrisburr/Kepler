from Gaudi.Configuration import *

from Configurables import Kepler
# Set the path to the directory/files to be processed
Kepler().InputFiles =  ["/afs/cern.ch/work/t/tevans/public/Kepler_data_1MHz_120s_trigger/"]
Kepler().EvtMax = 4000 


def sim():
  telseq = GaudiSequencer("Telescope")
  
  from Configurables import TbTrackFitter
  telseq.Members = [ TbTrackFitter()]
  # scattering estimate (theta0 squared?)
  TbTrackFitter().scat2 = 1.e-9
  #TbTrackFitter().scat2 = 0
  TbTrackFitter().direction = -1
  
  # divergence of true track 
  TbTrackFitter().theta0 = 1.e-4
  #TbTrackFitter().hiterror2 = 9.0e-6
  #TbTrackFitter().theta0 = 0
  
  monseq = GaudiSequencer("Monitoring")
  monseq.Members = []

appendPostConfigAction(sim)
