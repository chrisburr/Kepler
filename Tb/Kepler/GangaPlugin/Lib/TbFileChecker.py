from Ganga.GPIDev.Base import GangaObject
from Ganga.GPIDev.Adapters.IPostProcessor import PostProcessException, IPostProcessor
from Ganga.GPIDev.Base.Proxy import GPIProxyObject
from Ganga.GPIDev.Schema import ComponentItem, FileItem, Schema, SimpleItem, Version
from Ganga.Utility.Config import makeConfig, ConfigError, getConfig
from Ganga.Utility.Plugin import allPlugins
from Ganga.Utility.logging import getLogger, log_user_exception
import commands
import copy
import os
import string
import re

class TbFileChecker(IPostProcessor):

  _schema = IPostProcessor._schema.inherit_copy()
  _schema.datadict["alignment"] = SimpleItem(False)
  _schema.datadict["resubmit"] = SimpleItem(False) #resubmit items with no output in the errorsummary
  _category = 'postprocessor'
  _name = 'TbFileChecker'
  _exportmethods = ['check','ErrorSummary']

  def check(self,job):

    job_objects = []
    if len( job.subjobs ) != 0:
      for j in job.subjobs:
        job_objects.append(j)
    else: job_objects.append( job )
    result = True
    for j in job_objects:
      job_success = False
      filepath = "%s/stdout" %(j.outputdir)
     #print "Checking %d subjobs " %( len(job_objects ) )
      if j.status != "completed": continue
      if self.alignment:

        if not os.path.isfile( j.outputdir + "Alignment_out.dat" ):
          job_success = False
          j.force_status('failed')
          continue

      if not os.path.isfile( filepath ): continue
      f = reversed( open(filepath,'r').readlines() )
      for line in f:	
        if re.search("ApplicationMgr       INFO Application Manager Finalized successfully",line):
          job_success = True
          break
      if job_success: j.force_status('completed')
      else : j.force_status('failed')
      result = result*job_success
    return result 
 
  def ErrorSummary(self,job):
    job_objects = []
    minuit_exceptions = ["MATRIX","DERIVATIVE","VALUE"] 
    millepede_exceptions = ["Initial","Negative","diagonal"]
    for j in job.subjobs:
      if j.status == 'failed': job_objects.append(j)
    for j in job_objects:
      filepath = "%s/stdout" %(j.outputdir)
      error = "Unknown"
      if os.path.isfile( filepath ):
        for line in open(filepath,'r').readlines():
          if re.search("ERROR",line):
            found = False
            for e in minuit_exceptions: 
              if re.search(e,line): 
                found = True
                break
            if found : continue
            error = line[:-1]
            break
          if re.search("error",line):
            found = False
            for e in millepede_exceptions: 
              if re.search(e,line): 
                found = True
                break
            if found : continue

            error = line[:-1]
            break
          if re.search("Error",line):
            found = False
            for e in millepede_exceptions: 
              if re.search(e,line): 
                found = True
                break
            if found : continue

            error = line[:-1]
            break
      else: 
        error = "No output files"
        if self.resubmit: j.resubmit()
      print "%s (%d.%3d) : %s" %( j.name, job.id, j.id, error ) 
