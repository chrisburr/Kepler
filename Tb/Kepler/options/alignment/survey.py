from Gaudi.Configuration import *

from Configurables import Kepler
# Add TbAlignment to the Telescope sequence.
Kepler().Alignment = True
# Set the number of events to process.
Kepler().EvtMax = 100 

from Configurables import TbEventBuilder
# Skip noise events.
TbEventBuilder().MinPlanesWithHits = 5

from Configurables import TbClusterPlots
# Set the reference plane.
TbClusterPlots().ReferencePlane = 3 
# Widen the range of the difference histograms if needed.
TbClusterPlots().ParametersDifferenceXY = ('', -10., 10., 200)

from Configurables import TbAlignment
TbAlignment().AlignmentTechnique = "survey"

