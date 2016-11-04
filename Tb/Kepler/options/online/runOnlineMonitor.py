#!/usr/bin/env python
import os

os.environ["DIM_DNS_NODE"]="pplxint6.physics.ox.ac.uk"

os.environ["UTGID"]="LHCb_MONA_TbMonApp_00"

os.environ["PYTHONPATH"]=os.environ["KEPLERROOT"]+'/options:'+os.environ["PYTHONPATH"]

os.system('$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -msgsvc=MessageSvc -tasktype=LHCb::Class1Task -opt=command="import OnlineMonitor; OnlineMonitor.run()" -main=$GAUDIONLINEROOT/options/Main.opts -auto')
