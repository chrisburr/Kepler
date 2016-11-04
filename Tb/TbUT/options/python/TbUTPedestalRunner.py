__author__ = 'ja'
from Gaudi.Configuration import *
from Configurables import ExceptionSvc

from Configurables import Kepler

from Configurables import TbUT__RawDataReaderAlgorithm as rawDataReader
from Configurables import TbUT__RawDataMonitorAlgorithm as rawDataReaderMoniotor
from Configurables import TbUT__PedestalSubtractorAlgorithm  as pedestalSubtractor

from Timer import timer

class TbUTPedestalRunner:
    def __init__(self):
        pass

    @property
    def inputData(self):
        return self.inputData
    @inputData.setter
    def inputData(self,value):
        self.inputData=value

    @property
    def pedestalOutputData(self):
        return  self.pedestalOutputData
    @pedestalOutputData.setter
    def pedestalOutputData(self,value):
        expectedDirectory ="$KEPLERROOT/../TbUT/options/UT/"
        if not value.startswith(expectedDirectory):
            raise AttributeError("The pedestal file have to be stored in directory: "+expectedDirectory)
        self.pedestalOutputData=value

    @property
    def eventMax(self):
        return self.eventNumber
    @eventMax.setter
    def eventMax(self,value):
        self.eventMax=value

    @property
    def isAType(self):
        return self._isAType
    @isAType.setter
    def isAType(self,value):
        self._isAType=value

    @timer
    def runPedestals(self):
        self._preparePedestalRun()
        config= self._runGaudi
        appendPostConfigAction(config)


    def _preparePedestalRun(self):
         # dummy aligment just to run Kepler
        Kepler().PixelConfigFile = ["eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run1236/Conditions/PixelConfig.dat"]
        Kepler().AlignmentFile = "eos/lhcb/testbeam/velo/timepix3/Oct2014/RootFiles/Run2176/Conditions/Alignment2176mille.dat"
        Kepler().InputFiles =  ['eos/lhcb/testbeam/velo/timepix3/Oct2014/RawData/Run2176/']
        Kepler().EvtMax = self.eventMax
        Kepler().HistogramFile="MambaPedestal.root" # should be set more correctly

    def _runGaudi(self):
        keplerSeq = GaudiSequencer("KeplerSequencer")
        seq_moni = GaudiSequencer("Monitoring")
        seq_moni.Members = []
        seq_out = GaudiSequencer('Output')
        seq_moni.Members = []
        seq_tel = GaudiSequencer("Telescope")
        seq_tel.Members = []

        seq_UT_data_processing=GaudiSequencer("UTPedestal")
        rawDataReader().isAType=self.isAType
        rawDataReader().inputData= self.inputData
        pedestalSubtractor().ChannelMaskInputLocation= "$KEPLERROOT/../TbUT/options/UT/MambaMasks.dat"
        pedestalSubtractor().PedestalOutputFile=self.pedestalOutputData
        pedestalSubtractor().treningEntry=15000


        seq_UT_data_processing.Members =[rawDataReader(), rawDataReaderMoniotor() ,pedestalSubtractor()]
        keplerSeq.Members+=[seq_UT_data_processing]

