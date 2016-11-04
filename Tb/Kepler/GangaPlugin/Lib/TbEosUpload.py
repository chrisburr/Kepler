

from Ganga.GPIDev.Schema import *
from Ganga.GPIDev.Base import GangaObject
from Ganga.GPIDev.Schema import *
from Ganga.GPIDev.Adapters.IPostProcessor import IPostProcessor
import os

class TbEosUpload(IPostProcessor):

  _schema = IPostProcessor._schema.inherit_copy()
  _schema.datadict['files'] = SimpleItem(defvalue=[], doc='Files to upload')
  _schema.datadict['prefix'] = SimpleItem(defvalue="",doc='Prefix to uploaded file')
  _category = 'postprocessor'
  _name = 'TbEosUpload'
  _exportmethods = ['execute']


  def execute(self,job,newstatus):
    eos_cp="/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select cp"
    counter=0
    for run in job.splitter.Files.keys():
      j=job.subjobs(counter)
      if j.status == 'completed':
        rootpath = "/eos/lhcb/testbeam/velo/timepix3/%s/RootFiles/Run%d/" %( job.splitter.Month, run )
        if 'Alignment' in self.files:
          source="%sAlignment_out.dat" %( j.outputdir )
          if os.path.isfile( source ): 
            sink="%sConditions/%sAlignment%dmille.dat" %(rootpath, self.prefix, run )
            os.system(eos_cp+" "+source+" "+sink+" "+">/dev/null")
          else : print source + " does not exist"
        if 'Tuple' in self.files:
          source="%sKepler-tuple.root" %( j.outputdir )
          sink="%sOutput/%sKepler-tuple-%d.root" %(rootpath, self.prefix, run )
          os.system(eos_cp+" "+source+" "+sink+" "+">/dev/null")
        if 'Histograms' in self.files:
          source="%sKepler-histos.root" %( j.outputdir )
          sink="%sOutput/%sKepler-histos-%d.root" %(rootpath,self.prefix, run )
          os.system(eos_cp+" "+source+" "+sink+" "+">/dev/null")
        if 'TimingConfig' in self.files:
          source="%sTimingConfig.dat" %( j.outputdir )
          sink = "%sConditions/%sTimingConfig.dat" %(rootpath,self.prefix)
          os.system(eos_cp+" "+source+" "+sink+" "+">/dev/null")
      counter = counter + 1 
    return True
