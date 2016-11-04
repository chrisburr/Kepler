from Gaudi.Configuration import *

trackFitToolName = "TbKalmanTrackFit"
from Configurables import TbTracking
TbTracking().TrackFitTool = trackFitToolName
from Configurables import TbTrackPlots
TbTrackPlots().TrackFitTool = trackFitToolName

from Configurables import TbSimpleTracking
#TbSimpleTracking().TrackFitTool = trackFitToolName
from Configurables import TbVertexTracking
#TbVertexTracking().TrackFitTool = trackFitToolName

from Configurables import TbAlignment
# TbAlignment().TrackFitTool = trackFitToolName
