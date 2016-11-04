from Gaudi.Configuration import *

from Configurables import Kepler
# Add TbAlignment to the Telescope sequence.
Kepler().Alignment = True

#chombach modifications



from Configurables import TbEventBuilder
# Skip noise events.
TbEventBuilder().MinPlanesWithHits = 5


# List of devices under test
duts = [4] 

from Configurables import TbTracking
# Exclude DUTs from the pattern recognition.
TbTracking().MaskedPlanes = duts
# Require clusters on all telescope planes.
TbTracking().MinNClusters = 8 

from Configurables import TbAlignment
TbAlignment().AlignmentTechnique = "Millepede"
# Set the number of tracks to process.
TbAlignment().NTracks = 10000
# Set the reference plane (fixed position).
TbAlignment().ReferencePlane = 3
# Set the degrees of freedom (x, y, z, rx, ry, rz).
TbAlignment().DoFs = [0, 0, 1, 0, 0, 0]
TbAlignment().ParametersResidualsXY = ("", -0.2, 0.2, 100)
TbAlignment().MaskedPlanes = duts 
TbAlignment().PrintConfiguration = True
TbAlignment().ResCutInit = 1.3
TbAlignment().ResCut = 0.06
TbAlignment().NIterations = 6
