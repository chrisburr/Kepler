# Configuration file for residual studies. 

from os.path import join, abspath
from sys     import path

import pickle

from Gaudi.Configuration import *
from Configurables import Kepler

cdir = "eos/lhcb/testbeam/velo/timepix3/LabData/TP/SurrogateParameterFiles/"
cfile = {"W0009_J04"                    : [ cdir+"S20_Coarse64_26022015_SpidrTime_surrog_fitpars_perpix_NNsmoothingON.dat" ],
         "W0009_H08,W0009_D09,W0009_E09": [ cdir+"T2_200V_TestPulse_SpidrTime_130315_CHIP0_surrog_fitpars_perpix_NNsmoothingON.dat",
                                            cdir+"T2_200V_TestPulse_SpidrTime_130315_CHIP1_surrog_fitpars_perpix_NNsmoothingON.dat",
                                            cdir+"T2_200V_TestPulse_SpidrTime_130315_CHIP2_surrog_fitpars_perpix_NNsmoothingON.dat"]
         }

PATH_TO_OPTS= abspath('/afs/cern.ch/user/c/chombach/cmtuser/KEPLER/KEPLER_HEAD/Tb/Kepler/options/ResStudies/')
path.append( PATH_TO_OPTS )
from Runs import Runs

pickled_runs = PATH_TO_OPTS+'/Runs.pkl'
runs = pickle.load( open( pickled_runs ) )

block = runs.BLOCKS[0]
rdut = runs.RUNS[block][0].DUT
#runs = Runs('A','run')

if runs.TYPE == 'survey':
    Kepler().Alignment = True
    Kepler().EvtMax = 100

    from Configurables import TbClusterPlots
    # Set the reference plane.
    TbClusterPlots().ReferencePlane = 3

    # Widen the range of the difference histograms if needed.
    TbClusterPlots().ParametersDifferenceXY = ('', -30., 30., 600)
    from Configurables import TbAlignment
    TbAlignment().AlignmentTechnique = "survey"

    from Configurables import TbEventBuilder
    TbEventBuilder().MinPlanesWithHits = 5

elif runs.TYPE == 'mille':
    Kepler().Alignment = True
    Kepler().EvtMax = 100
    
    # List of devices under test
    duts = [4]
    from Configurables import TbTracking
    # Exclude DUTs from the pattern recognition.
    TbTracking().MaskedPlanes = duts
    # Require clusters on all telescope planes.
    TbTracking().MinNClusters = 8
    from Configurables import TbClusterPlots
    # Set the reference plane.
    TbClusterPlots().ReferencePlane = 3
    TbClusterPlots().ParametersDifferenceXY = ('', -10., 10., 200)
    from Configurables import TbAlignment
    
    # Set the number of tracks to process.
    TbAlignment().NTracks = 10000
    # Set the reference plane (fixed position).
    TbAlignment().ReferencePlane = 3
    # Set the degrees of freedom (x, y, z, rx, ry, rz).
    TbAlignment().DoFs = [1, 1, 0, 1, 1, 1]
    TbAlignment().ParametersResidualsXY = ("", -0.2, 0.2, 100)
    TbAlignment().MaskedPlanes = duts
    TbAlignment().PrintConfiguration = True
    TbAlignment().ResCutInit = 1.3
    TbAlignment().ResCut = 0.06
    TbAlignment().NIterations = 6
    
    TbAlignment().AlignmentTechnique = "Millepede"

    from Configurables import TbMillepede
    TbMillepede().OutputLevel = 2
    from Configurables import TbEventBuilder
    TbEventBuilder().MinPlanesWithHits = 5

elif runs.TYPE == 'dut':
    Kepler().Alignment = True
    Kepler().EvtMax = 100

    from Configurables import TbAlignment
    TbAlignment().AlignmentTechnique = "Millepede"
    TbAlignment().MilleDUT = True
    TbAlignment().DeviceToAlign = 4
    TbAlignment().DoFs = [1, 1, 0, 1, 1, 1]
    # Set the number of tracks to process.
    TbAlignment().NTracks = 10000

    from Configurables import TbEventBuilder
    TbEventBuilder().MinPlanesWithHits = 5
elif runs.TYPE == 'run':
    Kepler().Alignment = False
    Kepler().EvtMax = 1000

from Configurables import TbClustering
TbClustering().PrintConfiguration = True

from Configurables import TbTracking
TbTracking().PrintConfiguration = True

Kepler().PixelConfigFile = cfile[ rdut ]
