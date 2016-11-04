
from Ganga.GPIDev.Base import GangaObject
from Ganga.GPIDev.Base.Proxy import GPIProxyObjectFactory
from Ganga.GPIDev.Schema import *

def ListFiles( path ,extension) :
  import subprocess
  proc = subprocess.Popen(["/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select","ls",path],stdout=subprocess.PIPE)
  out=""
  err=""
  files = []
  (out,err) = proc.communicate()
  files = out.split('\n')
  output = []
  for f in files:
    if f.find(extension) != -1:
      output.append( path + f)
  return output
  
class TbQuery( GangaObject ) :
  path = ""
  _schema = Schema(Version(1,0), {
        'Month' : SimpleItem(defvalue='',doc='Testbeam period to look for data'),
        'Run' : SimpleItem(defvalue=0,doc='Run to look at')
        } )
  _name = "TbQuery"
  _exportmethods = ['getOptions','getFiles']
  _category = 'query'
  _data = ''
          
  def getFiles(self):
    path = "eos/lhcb/testbeam/velo/timepix3/%s/RawData/Run%d/" %( self.Month, self.Run )
    return ListFiles(path,".dat")
    
  def getConfiguration(self):
    path = "eos/lhcb/testbeam/velo/timepix3/%s/RootFiles/Run%d/Conditions/" %(self.Month,self.Run)
    return ListFiles(path,".dat")
  _exportmethods = ['getConfiguration','getFiles']
    