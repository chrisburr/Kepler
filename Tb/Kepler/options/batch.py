from Gaudi.Configuration import *

from Configurables import Kepler

from Configurables import TbClusterAssociator
TbClusterAssociator().DUTs = [4]
TbClusterAssociator().XWindow = 0.1
TbClusterAssociator().TimeWindow = 35

from Configurables import TbEventBuilder
TbEventBuilder().PrintHeader = True
TbEventBuilder().MinPlanesWithHits = 8
TbEventBuilder().PrintFreq = 50
TbEventBuilder().Monitoring = True
TbEventBuilder().ForceCaching = False

from Configurables import TbClustering
TbClustering().SearchDist = 2

from Configurables import TbSimpleTracking
Kepler().UseSimpleTracking = True
TbSimpleTracking().MinPlanes = 8
TbSimpleTracking().MaskedPlanes=[4]


from Configurables import TbTrackPlots
TbTrackPlots().ParametersSlope = ("", -0.01,0.01,1000)
TbTrackPlots().ParametersResidualsXY = ("",-0.055,0.055,500)
TbTrackPlots().ParametersXY=("",0,15,500)

from Configurables import TbTriggerMonitor
TbTriggerMonitor().OutputLevel = ERROR
