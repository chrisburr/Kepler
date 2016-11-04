from Gaudi.Configuration import *

from Configurables import Kepler
Kepler().InputFiles      = ["eos/lhcb/testbeam/velo/timepix3/July2014/RawData/Run1062/"]
Kepler().PixelConfigFile = "eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run1062/Conditions/PixelMask.dat"
Kepler().EvtMax          = 100
Kepler().WriteTuples     = False
Kepler().Monitoring      = False
Kepler().Alignment       = True
Kepler().AlignmentFile   = "Alignment.dat"

from Configurables import TbTrackPlots
TbTrackPlots().ParametersResidualsXY = ("",-0.1,0.1,2000)

from Configurables import TbAlignment, TbTracking
TbAlignment().PrintConfiguration = True
TbAlignment().FitStrategy        = 1
TbAlignment().ReferencePlane     = 3 
TbAlignment().MaskedPlanes       = TbTracking().MaskedPlanes
TbAlignment().OutputLevel        = INFO
