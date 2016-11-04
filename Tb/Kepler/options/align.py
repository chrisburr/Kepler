from Gaudi.Configuration import *

from Configurables import Kepler
Kepler().Alignment = True

from Configurables import TbAlignment
from Configurables import TbMillepede, TbAlignmentMinuit2, TbAlignmentMinuit1

Kepler().addAlignment( TbAlignmentMinuit1( MaskedPlanes=[4], DOFs=[1,1,0,0,0,1], MaxChi2=9999999999999999999 , ReferencePlane=3) )
Kepler().addAlignment( TbMillepede(MaskedPlanes=[4],DOFs=[1,1,0,1,1,1],MaxChi2 = 2000,ResCutInit=0.5,ResCut=0.1))
Kepler().addAlignment( TbMillepede(MaskedPlanes=[4], DOFs=[1,1,0,1,1,1], MaxChi2 = 15,ResCutInit=1.0,ResCut=0.2 ) )
Kepler().addAlignment( TbMillepede(Monitoring=True, MaskedPlanes=[4], DOFs=[1,1,1,1,1,1], MaxChi2=15,ResCutInit=1.0,ResCut=0.2 ) )

Kepler().addAlignment( TbAlignmentMinuit2(DOFs=[1,1,0,0,0,0],MaxChi2=1 , DeviceToAlign=4, XWindow=10000000, TimeWindow=15) )
Kepler().addAlignment( TbAlignmentMinuit2(DOFs=[1,1,0,1,1,1],MaxChi2=1 , DeviceToAlign=4, XWindow=1.0, TimeWindow=5) )
Kepler().addAlignment( TbAlignmentMinuit2(DOFs=[1,1,0,1,1,1],MaxChi2=1 , DeviceToAlign=4, XWindow=1.0, TimeWindow=5) )
Kepler().addAlignment( TbAlignmentMinuit2(DOFs=[1,1,1,1,1,1],MaxChi2=1 , DeviceToAlign=4, XWindow=1.0, TimeWindow=5) )
Kepler().addAlignment( TbAlignmentMinuit2(DOFs=[1,1,1,1,1,1],MaxChi2=1 , DeviceToAlign=4, XWindow=1.0, TimeWindow=5, Monitoring=True) )

TbAlignment().PrintConfiguration = True
TbAlignment().NTracks = 8000

from Configurables import TbEventBuilder
TbEventBuilder().MinPlanesWithHits = 8

from Configurables import TbSimpleTracking
Kepler().UseSimpleTracking = True

TbSimpleTracking().MinPlanes = 8
TbSimpleTracking().TimeWindow = 35
TbSimpleTracking().MaskedPlanes = [4]
TbSimpleTracking().MaxDistance = 1.2

from Configurables import TbTriggerMonitor
TbTriggerMonitor().OutputLevel = ERROR
