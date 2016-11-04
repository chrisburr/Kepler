from Gaudi.Configuration import *

from Configurables import Kepler
Kepler().Alignment = True
Kepler().EvtMax    = 30000

from Configurables import TbAlignment
TbAlignment().ReferencsvePlane = "Chip1"
TbAlignment().DuT            = ""
TbAlignment().AlignmentTechnique = "Millepede"
TbAlignment().DoFs = [1,1,0,1,1,1]
TbAlignment().ParametersResidualsXY= ("", -0.8, 0.8, 200)
TbAlignment().MaskedPlanes = []
TbAlignment().PrintConfiguration = True

from Configurables import TbClusterPlots
TbClusterPlots().ParametersDifferenceRot = ("",-.5,0.5,200)
TbClusterPlots().ParametersDifferenceXY = ("",-7,7,200)
TbClusterPlots().ParametersXY = ("",-1.,15,280)
TbClusterPlots().ReferencePlane = 2
TbClusterPlots().FillComparisonPlots = True

from Configurables import TbTrackPlots, TbCombatBuilder, TbClustering, TbTracking
TbTrackPlots().ParametersResidualsXY= ("", -0.8, 0.8, 200)
seq = GaudiSequencer("Telescope")
seq.Members = [TbCombatBuilder(), TbClustering(), TbTracking(), TbAlignment()]

from Configurables import TbTracking
TbTracking().MaskedPlanes = []

def patch():
     from Configurables import TbTrackPlots, TbCombatBuilder, TbClustering, TbTracking
     seq.Members = [TbCombatBuilder(), TbClustering(), TbTracking(), TbAlignment()]

appendPostConfigAction(patch)