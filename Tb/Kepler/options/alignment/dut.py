from Gaudi.Configuration import *

from Configurables import Kepler
Kepler().Alignment = True

from Configurables import TbAlignment
TbAlignment().AlignmentTechnique = "3"
TbAlignment().DeviceToAlign = 4
# TbAlignment().DoFs = [True, True, True, True, True, True]
 
