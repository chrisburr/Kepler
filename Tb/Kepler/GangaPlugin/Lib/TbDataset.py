from Ganga.GPIDev.Adapters.ISplitter import ISplitter
from Ganga.GPIDev.Schema import *
from TbQuery import TbQuery
from collections import defaultdict
from Ganga.GPIDev.Base.Proxy import addProxy, stripProxy

class TbDataset(ISplitter):
  _name = "TbDataset"
  _schema = Schema(Version(1,0), {
  'filesPerJob' : SimpleItem(defvalue=1),
  'maxFiles' : SimpleItem(defvalue=1),
  'ignoremissing': SimpleItem(defvalue=False),
  'Files' : SimpleItem(defvalue={}),
  'AlignmentFiles' : SimpleItem(defvalue={}),
  'PixelConfigFiles' : SimpleItem(defvalue={}),
  'TimingConfigFiles' : SimpleItem(defvalue={}),
  'Month' : SimpleItem(defvalue=""),
  'run' : SimpleItem(defvalue=0),
  'prefix' : SimpleItem(defvalue=""),
  'AutoConfig' : SimpleItem(defvalue=True) })
  _exportmethods = [ 'split','optionsString' ]   
    
  def split(self,job):
    from Ganga.GPIDev.Lib.Job import Job
   
    subjobs = []
    for run in self.Files.keys():
      j = addProxy(self.createSubjob(job))
#      j.splitter = None
#      j.merger = None
      jp = stripProxy(j)
      jp._splitter_data = self.optionsString(run)
      subjobs.append(jp)
      
    print "Submitting jobs for %d runs" %( len(subjobs))
    return subjobs
  
  def __construct__(self,args):
    if len( args ) == 0 : return 
    self.Month = args[0]
    for r in args[1]:
      query = TbQuery()
      query.Month = self.Month
      query.Run = r
      print "Looking for %s/Run%d" %(self.Month, r )
      files = query.getFiles()
      if len(files) != 0 : self.Files[r] = []
      for f in files:
        self.Files[r].append(f)
      
      if self.AutoConfig:
        config_files = query.getConfiguration()
        alignment_file = ""
        for f in config_files:
          if f.find("Alignment") != -1:
            if alignment_file != "": alignment_file = f
            elif f.find("mille") != -1: alignment_file = f
           # elif f.find(self.prefix) != -1 : alignment_file = f
          elif f.find("PixelConfig") != -1:
            if r not in self.PixelConfigFiles.keys():
              self.PixelConfigFiles[r] = []
            self.PixelConfigFiles[r].append(f)
          elif f.find("TimingConfig") != -1: self.TimingConfigFiles[r] = f
        if alignment_file != "" : self.AlignmentFiles[r] = alignment_file
      
      
  def optionsString(self, run):
  
    files_for_this_run = self.Files[run]
    if len( files_for_this_run ) == 0 :
      return ""
    output = "from Configurables import TbDataSvc \n"
    output += "TbDataSvc().Input = ['" + files_for_this_run[0] +"'"
  
    for f in range(1, len(files_for_this_run)):
        output += ",'"+files_for_this_run[f]+"'"
    output += "]"
    # now add the configuration files ... 
    if self.AutoConfig:
      pixel_config = []
      if run in self.PixelConfigFiles.keys() : 
        pixel_config = self.PixelConfigFiles[run]
        output+= " \nTbDataSvc().PixelConfigFile += ['" + pixel_config[0] +"'"
        for f in range(1,len(pixel_config)):
          output += ",'"+pixel_config[f]+"'"
        output += "]"
      timing_config = ""
      if run in self.TimingConfigFiles.keys() :
        timing_config = self.TimingConfigFiles[run]
        output += " \nTbDataSvc().TimingConfigFile = '%s'" %(timing_config)
        
      alignment = ""
      if run in self.AlignmentFiles.keys() : 
        alignment = self.AlignmentFiles[run]
        output += " \nTbDataSvc().AlignmentFile = '%s'" %(alignment)
      
    return output
  
