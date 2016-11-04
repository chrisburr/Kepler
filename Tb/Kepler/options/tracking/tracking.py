# This file provides an example how to configure the algorithm TbTracking

from Configurables import TbTracking
TbTracking().MinNClusters = 7
TbTracking().SearchRadius = 1
TbTracking().VolumeAngle = 0.015
TbTracking().SearchVolume = "diabolo"
TbTracking().TimeWindow = 150
TbTracking().nComboCut = 300 # O(100) for speed.
TbTracking().SearchVolumeFillAlgorithm = "adap_seq" # {seq, adap_seq}. Adap faster.
TbTracking().Monitoring = True
TbTracking().SearchPlanes = [4,3,5]
# TbTracking().MaskedPlanes = []

