__author__ = 'ja'


from Gaudi.Configuration import *
from Configurables import Kepler

from Configurables import TbUT__RawDataReaderAlgorithm as rawDataReader
from Configurables import TbUT__RawDataMonitorAlgorithm as rawDataMonitor
from Configurables import TbUT__PedestalSubtractorAlgorithm  as pedestalSubtractor
from Configurables import TbUT__PedestalSubtractorDataMonitorAlgorithm as pedestalMonitor
from Configurables import TbUT__CommonModeSubtractorAlgorithm  as CMS
from Configurables import TbUT__CommonModeSubtractorDataMonitorAlgorithm  as CMSMonitor
from Configurables import TbUT__ClusterCreatorAlgorithm as ClusterCreator
from Configurables import TbUT__ClusterCreatorDataMonitorAlgorithm as ClusterCreatorMonitor
from Configurables import TbUT__NTupleCreator as nTupleCreator

from Timer import timer

class TbUTClusterizator:
    def __init__(self):
        self.isAType=True

    @property
    def eventNumberDisplay(self):
        return self.eventNumberDisplay
    @eventNumberDisplay.setter
    def eventNumberDisplay(self,value):
        self.eventNumberDisplay=value

    @property
    def pedestalInputData(self):
        return  self.pedestalInputData
    @pedestalInputData.setter
    def pedestalInputData(self,value):
        self.pedestalInputData=value
    @property
    def eventMax(self):
        return self.eventNumber
    @eventMax.setter
    def eventMax(self,value):
        self.eventMax=value

    @property
    def isAType(self):
        return self.isAType
    @isAType.setter
    def isAType(self, value):
        self.isAType=value

    @property
    def sensorType(self):
        return self.sensorType
    @sensorType.setter
    def sensorType(self, value):
        self.sensorType=value

    @property
    def inputData(self):
        return self.inputData
    @inputData.setter
    def inputData(self,value):
        self.inputData=value

    @timer
    def runClusterization(self):
        self._prepareStandaloneClusterization()
        config=self._runClusterization
        appendPostConfigAction(config)



    def _prepareStandaloneClusterization(self):
        # dummy aligment just to run Kepler
        Kepler().PixelConfigFile = ["eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run1236/Conditions/PixelConfig.dat"]
        Kepler().AlignmentFile = "eos/lhcb/testbeam/velo/timepix3/Oct2014/RootFiles/Run2176/Conditions/Alignment2176mille.dat"
        Kepler().InputFiles =  ['eos/lhcb/testbeam/velo/timepix3/Oct2014/RawData/Run2176/']
        Kepler().EvtMax = self.eventMax
        outputName=self.inputData[:-4]+".root"
        outputName=outputName[outputName.rfind("/")+1:]
        Kepler().HistogramFile=outputName
        outputTupleName=self.inputData[:-4]+"_Tuple.root"
        outputTupleName=outputTupleName[outputTupleName.rfind("/")+1:]
        Kepler().TupleFile=outputTupleName


    def _runClusterization(self):
        keplerSeq = GaudiSequencer("KeplerSequencer")
        seq_moni = GaudiSequencer("Monitoring")
        seq_moni.Members = []
        seq_out = GaudiSequencer('Output')
        seq_moni.Members = []
        seq_tel = GaudiSequencer("Telescope")
        seq_tel.Members = []

        seq_UT_data_processing=GaudiSequencer("UT")

        rawDataReader().inputData=self.inputData
        rawDataReader().isAType=self.isAType
        rawDataMonitor().displayEventNumber=self.eventNumberDisplay
        pedestalSubtractor().ChannelMaskInputLocation= "$KEPLERROOT/../TbUT/options/UT/MambaMasks.dat"
        pedestalSubtractor().PedestalInputFile=self.pedestalInputData
        pedestalSubtractor().FollowingOption='file'
        pedestalSubtractor().treningEntry=1
        pedestalMonitor().displayEventNumber=self.eventNumberDisplay
        CMS().ChannelMaskInputLocation= "$KEPLERROOT/../TbUT/options/UT/MambaMasks.dat"
        CMS().NoiseOutputFile="$KEPLERROOT/../TbUT/options/UT/noise_Mamba.dat"
        CMSMonitor().displayEventNumber=self.eventNumberDisplay
        ClusterCreator().NoiseInputFile="$KEPLERROOT/../TbUT/options/UT/noise_Mamba.dat"
        ClusterCreator().LowThreshold=2.5
        ClusterCreator().HighThreshold=3
        ClusterCreator().sensorType=self.sensorType
        ClusterCreatorMonitor().displayEventNumber=self.eventNumberDisplay
        ClusterCreatorMonitor().sensorType=self.sensorType
        nTupleCreator().StoreEventNumber=900000
        nTupleCreator().WriteRaw=False
        nTupleCreator().WriteHeader=True
        nTupleCreator().WritePedestal=False
        nTupleCreator().WriteCMS=False
        seq_UT_data_processing.Members = [rawDataReader(), rawDataMonitor() ,pedestalSubtractor(),pedestalMonitor(), CMS(),
                                          CMSMonitor(), ClusterCreator(),ClusterCreatorMonitor(),nTupleCreator()]
        keplerSeq.Members+=[seq_UT_data_processing]
