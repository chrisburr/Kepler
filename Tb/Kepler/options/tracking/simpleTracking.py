# Use TbSimpleTracking
def simpleTracking():
  from Configurables import TbSimpleTracking
  GaudiSequencer("Tracking").Members = [TbSimpleTracking()]
appendPostConfigAction(simpleTracking)

