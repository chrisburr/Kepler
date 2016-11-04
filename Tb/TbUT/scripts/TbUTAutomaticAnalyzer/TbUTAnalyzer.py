__author__ = 'ja'
from xml.dom.minidom import parse
import xml.dom.minidom
import os
from tempfile import mkstemp
from shutil import move
from os import remove, close

class TbUTAnalyzer():

    def __init__(self):
        self.dirToAnalyse="BoardA4"
        self.pathToDataInEos="/afs/cern.ch/user/a/adendek/eos/lhcb/testbeam/ut/OfficialData/July2015/"
        self.scanType='BiasScan'  # one of 'AngleScan' 'BiasScan'
        self.TbUTPath=os.path.expandvars("$KEPLERROOT")+"/../TbUT/"

    def _performAnalysis(self, inputFilename):
        print( "work on: " +inputFilename)
        self.__changeOptionFile(inputFilename)
        os.chdir(self.TbUTPath)
        os.system("gaudirun.py options/TbUTRun.py")
        os.system("gaudirun.py options/TbUTRun.py")
        self._moveOutputRootFilesIntoDirectory(inputFilename)
        ###
        # add your stuffs here!
        ###


    def runAnalysis(self):
        self._createOutputDir()
        xmlRoot = xml.dom.minidom.parse('July2015Database.xml')
        for directory in xmlRoot.getElementsByTagName('directory'):
            if (directory.getAttribute("name")) in self.dirToAnalyse:
                biasScan=directory.getElementsByTagName(self.scanType)
                for run in biasScan[0].getElementsByTagName("file"):
                    dataPath=self.pathToDataInEos+ self.dirToAnalyse+"/RawData/"+run.firstChild.nodeValue
                    self._performAnalysis(dataPath)


    def _createOutputDir(self):
        self.outDirName=self.TbUTPath+"study_"+self.dirToAnalyse+"_"+self.scanType
        if not os.path.exists(self.outDirName):
            os.mkdir(self.outDirName)

    def __changeOptionFile(self,inputFileName):
        optionFilePath=self.TbUTPath+"options/TbUTRun.py"
        fh, abs_path = mkstemp()
        with open(abs_path,'w') as new_file:
            with open(optionFilePath) as old_file:
                for line in old_file:
                    if not  "app.inputData" in line:
                        new_file.write(line)
                    else:
                        new_file.write("app.inputData='"+inputFileName+"'\n")
        close(fh)
        #Remove original file
        remove(optionFilePath)
        #Move new file
        move(abs_path, optionFilePath)

    def _moveOutputRootFilesIntoDirectory(self, inputFilename):
        outputName=inputFilename[:-4]+".root"
        outputName=outputName[outputName.rfind("/")+1:]
        move(outputName,self.outDirName+"/"+outputName)

        outputTupleName=inputFilename[:-4]+"_Tuple.root"
        outputTupleName=outputTupleName[outputTupleName.rfind("/")+1:]
        move(outputTupleName,self.outDirName+"/"+outputTupleName)


if __name__=='__main__':
    analyzer = TbUTAnalyzer()
    analyzer.runAnalysis()
