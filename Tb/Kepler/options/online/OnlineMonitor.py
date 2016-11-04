
#-------------- EXTRA LINES TO RUN MONITOR SERVICE ONLINE --------------#
import os
from Gaudi.Configuration import ApplicationMgr

from Configurables import LHCb__EventRunable
ApplicationMgr().Runable= LHCb__EventRunable("Runable",NumErrorToStop=1)

from Configurables import MonitorSvc
ApplicationMgr().ExtSvc += ["MonitorSvc"]
MonitorSvc().ExpandNameInfix=os.environ["UTGID"]+"/"
MonitorSvc().ExpandCounterServices = 1
MonitorSvc().DimUpdateInterval = 1

def run():
  import OnlineEnv as Online
  Online.end_config(False)



#----------------------- USER JOB CONFIGURATION ------------------------#
import example
