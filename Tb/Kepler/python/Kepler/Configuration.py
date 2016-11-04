
"""
High level configuration tools for Kepler 
"""
__version__ = ""
__author__  = ""

from Gaudi.Configuration import *
import GaudiKernel.ProcessJobOptions
from Configurables import LHCbConfigurableUser, LHCbApp

class Kepler(LHCbConfigurableUser):

  __slots__ = {
    # Input 
    "AlignmentFile"      :   "",            # Name of input alignment file
    "PixelConfigFile"    :   [],            # Names of pixel configuration files
    "TimingConfigFile"   :   "",            # Name of file that configures time offsets
    "EtaConfigFiles"      :   [],            # Names of files with eta correction parameters
    "InputFiles"         :   [],            # Names of input files or directories
    "EvtMax"             :   -1,            # Max. number of events to process
    # Output
    "HistogramFile"      :   "",            # Name of output histogram file
    "TupleFile"          :   "",            # Name of output tuple file
    "WriteTuples"        :   False,         # Flag to write out tuple file or not
    # Options
    "Tracking"           :   True,          # Flag to run tracking or not
    "UseSimpleTracking"  :   True,          # Flag to use Heinrich's Track finding method
    "Alignment"          :   False,         # Flag to run alignment or not
    "Monitoring"         :   True,          # Flag to run the monitoring sequence
    "Sim"                :   False, 
    "UT"                 :   False,         # Flag to run the UT reconstruction sequence
    "UserAlgorithms"     :   []             # List of user algorithms not typically run
  }

  _propertyDocDct = {
    'AlignmentFile'   : """ Name of input alignment file """,
    'PixelConfigFile' : """ Names of input pixel configuration files """,
    'TimingConfigFile': """ Name of input timing configuration file """,
    'EtaConfigFiles'   : """ Names of files with eta correction parameters """,
    'InputFiles'      : """ Names of input raw data files or directories """,
    'EvtMax'          : """ Maximum number of events to process """,
    'HistogramFile'   : """ Name of output histogram file """,
    'TupleFile'       : """ Name of output tuple file """,
    'WriteTuples'     : """ Flag to write out tuple files or not """,
    'Tracking'        : """ Flag to run tracking or not """,
    'Alignment'       : """ Flag to activate alignment or not """,
    'Monitoring'      : """ Flag to run the monitoring sequence or not """,
    'Sim'             : """ Flag to simulate tracks rather than read in """,
    'UT'              : """ Flag to run the UT sequence or not """,
    'UserAlgorithms'  : """ List of user algorithms """
  }

  __used_configurables__ = [LHCbApp,LHCbConfigurableUser]

  def configureServices(self):
    # Pass the input file names to the data service.
    from Configurables import TbDataSvc
    TbDataSvc().Input = self.getProp("InputFiles")
    TbDataSvc().AlignmentFile = self.getProp("AlignmentFile")
    TbDataSvc().PixelConfigFile = self.getProp("PixelConfigFile")
    TbDataSvc().TimingConfigFile = self.getProp("TimingConfigFile")
    TbDataSvc().EtaConfigFiles = self.getProp("EtaConfigFiles")
    ApplicationMgr().ExtSvc += [TbDataSvc()]

    # Add the geometry service which keeps track of alignment,
    # the timing service, which tracks event boundaries, 
    # and the pixel service, which stores masked pixels, 
    # clock phases and per column timing offsets.
    from Configurables import TbGeometrySvc, TbTimingSvc, TbPixelSvc
    ApplicationMgr().ExtSvc += [TbGeometrySvc(), TbTimingSvc(), TbPixelSvc()]
    
    # Use TimingAuditor for timing, suppress printout from SequencerTimerTool
    from Configurables import AuditorSvc, SequencerTimerTool
    ApplicationMgr().ExtSvc += ['ToolSvc', 'AuditorSvc']
    ApplicationMgr().AuditAlgorithms = True
    AuditorSvc().Auditors += ['TimingAuditor']
    if not SequencerTimerTool().isPropertySet("OutputLevel"):
      SequencerTimerTool().OutputLevel = 4

  def configureSequence(self):
    """
    Set up the top level sequence and its phases
    """
    keplerSeq = GaudiSequencer("KeplerSequencer")
    ApplicationMgr().TopAlg = [keplerSeq]
    telescopeSeq = GaudiSequencer("Telescope")
    self.configureTelescope(telescopeSeq)
    keplerSeq.Members = [telescopeSeq]
    if self.getProp("UT") == True:
      utSeq = GaudiSequencer("UT")
      self.configureUT(utSeq)
      keplerSeq.Members += [utSeq]
    if self.getProp("Monitoring") == True:
      monitoringSeq = GaudiSequencer("Monitoring")
      self.configureMonitoring(monitoringSeq)
      keplerSeq.Members += [monitoringSeq]
    outputSeq = GaudiSequencer("Output")
    self.configureOutput(outputSeq)
    keplerSeq.Members += [outputSeq]
    UserAlgorithms = self.getProp("UserAlgorithms")
    if (len(UserAlgorithms) != 0):
      userSeq = GaudiSequencer("UserSequence")
      userSeq.Members = UserAlgorithms
      keplerSeq.Members += [userSeq]   
  def configureTelescope(self, seq):
    if self.getProp("Sim") == False:
      from Configurables import TbEventBuilder
      seq.Members += [TbEventBuilder()]
    else :
      from Configurables import TbTestMC
      seq.Members += [TbTestMC()]
    from Configurables import TbClustering
    seq.Members += [TbClustering()]
    if self.getProp("Tracking") == True:
      from Configurables import TbSimpleTracking, TbTracking
      trackingSeq = GaudiSequencer("Tracking")
      if self.getProp("UseSimpleTracking") == True :
        trackingSeq.Members = [TbSimpleTracking()]
      else :
        trackingSeq.Members = [TbTracking()]
      seq.Members += [trackingSeq]
      from Configurables import TbTriggerAssociator
      seq.Members += [TbTriggerAssociator()]
      from Configurables import TbClusterAssociator
      seq.Members += [TbClusterAssociator()]
      if self.getProp("Alignment") == True:
        from Configurables import TbAlignment
        seq.Members += [TbAlignment()]

  def configureMonitoring(self, seq):
    from Configurables import TbHitMonitor, TbClusterPlots, TbTrackPlots
    if self.getProp("Tracking") == True :
      seq.Members += [TbHitMonitor(), TbClusterPlots(), TbTrackPlots()]
      from Configurables import TbTriggerMonitor
      seq.Members += [TbTriggerMonitor()]
      from Configurables import TbDUTMonitor
      seq.Members += [TbDUTMonitor()]
      from Configurables import TbTrackingEfficiency
      seq.Members += [TbTrackingEfficiency()]
    else :
      seq.Members += [TbHitMonitor(), TbClusterPlots()]

  def configureUT(self, seq):
    # UT algorithms
    seq.Members = []

  def configureInput(self):
    # No events are read as input
    ApplicationMgr().EvtSel = 'NONE'
    # Delegate handling of max. number of events to ApplicationMgr
    self.setOtherProps(ApplicationMgr(), ["EvtMax"])
    # Transient store setup
    EventDataSvc().ForceLeaves = True
    EventDataSvc().RootCLID    =    1
    # Suppress warning message from EventLoopMgr
    from Configurables import MessageSvc
    MessageSvc().setError += ['EventLoopMgr']

  def configureOutput(self, seq):
    # ROOT persistency for histograms
    ApplicationMgr().HistogramPersistency = "ROOT"
    # Set histogram file name.
    histoFile = "Kepler-histos.root"
    if (self.isPropertySet("HistogramFile") and self.getProp("HistogramFile") != ""):
      histoFile = self.getProp("HistogramFile")
    HistogramPersistencySvc().OutputFile = histoFile
    # Set tuple file name.
    tupleFile = "Kepler-tuple.root"
    if (self.isPropertySet('TupleFile') and self.getProp("TupleFile") != ""):
      tupleFile = self.getProp("TupleFile")
    ApplicationMgr().ExtSvc += [NTupleSvc()]
    tupleStr = "FILE1 DATAFILE='%s' TYP='ROOT' OPT='NEW'" % tupleFile
    NTupleSvc().Output += [tupleStr]
    from Configurables import MessageSvc
    MessageSvc().setWarning += ['RFileCnv', 'RCWNTupleCnv']
    # If requested add TbTupleWriter to the output sequence 
    if self.getProp("WriteTuples") == True:
      from Configurables import TbTupleWriter
      seq.Members += [TbTupleWriter()]

  def evtMax(self):
    return LHCbApp().evtMax()

  def __apply_configuration__(self):
    GaudiKernel.ProcessJobOptions.PrintOn()
    self.configureServices()
    self.configureSequence()
    self.configureInput()
    GaudiKernel.ProcessJobOptions.PrintOn()
    log.info(self)
    GaudiKernel.ProcessJobOptions.PrintOff()

  def addAlignment(self, a): 
    from Configurables import TbAlignment
    counter = len(TbAlignment().AlignmentTechniques)
    name = "s%d" % (counter)
    algname = type(a).__name__
    print "Added tool: %s (%d)" % (algname, counter)
    TbAlignment().addTool(a, name = name)
    TbAlignment().AlignmentTechniques.append(algname)



