## Note that the special string AppName will be replaced upon initialisation
## in all cases with the relavent app name (DaVinci, Gauss etc...)
import os, tempfile, pprint, sys
from GangaGaudi.Lib.Applications.Gaudi import Gaudi
from GangaGaudi.Lib.Applications.GaudiUtils import fillPackedSandbox
from GangaLHCb.Lib.Applications.AppsBaseUtils import available_apps, guess_version, available_packs
from GangaLHCb.Lib.Applications.AppsBaseUtils import backend_handlers, activeSummaryItems
from Ganga.GPIDev.Lib.File.FileBuffer import FileBuffer
from Ganga.GPIDev.Base.Proxy import GPIProxyObjectFactory
from Ganga.GPIDev.Schema import *
from Ganga.Utility.Shell import Shell
from GangaLHCb.Lib.Applications.PythonOptionsParser import PythonOptionsParser
from Ganga.GPIDev.Adapters.StandardJobConfig import StandardJobConfig
from Ganga.Utility.Config import getConfig
from Ganga.Utility.files import expandfilename
from Ganga.Utility.execute import execute
import Ganga.Utility.logging
import GangaLHCb.Lib.Applications.CMTscript
import pickle
import subprocess
from GangaLHCb.Lib.Applications import XMLPostProcessor
from Ganga.Core.exceptions import ApplicationConfigurationError

logger = Ganga.Utility.logging.getLogger()

class Kepler(Gaudi):
    _name = 'Kepler'
    _category = 'applications'
    #__doc__ = GaudiDocString('AppName')
    _schema = Gaudi._schema.inherit_copy()
    docstr = 'The package the application belongs to (e.g. "Sim", "Phys")'
    _schema.datadict['package'] = SimpleItem(defvalue=None,
                                             typelist=['str','type(None)'],
                                             doc=docstr)
    docstr = 'The package where your top level requirements file is read '  \
             'from. Can be written either as a path '  \
             '\"Tutorial/Analysis/v6r0\" or in a CMT style notation '  \
             '\"Analysis v6r0 Tutorial\"'
    _schema.datadict['masterpackage'] = SimpleItem(defvalue=None,
                                                   typelist=['str','type(None)'],
                                                   doc=docstr)
    _schema.datadict['platform'] = SimpleItem( defvalue="x86_64-slc6-gcc48-opt" )
    docstr = 'Extra options to be passed onto the SetupProject command '\
             'used for configuring the environment. As an example '\
             'setting it to \'--dev\' will give access to the DEV area. '\
             'For full documentation of the available options see '\
             'https://twiki.cern.ch/twiki/bin/view/LHCb/SetupProject'
    _schema.datadict['setupProjectOptions'] = SimpleItem(defvalue='',
                                                         typelist=['str','type(None)'],
                                                         doc=docstr)    
    _exportmethods = Gaudi._exportmethods[:]
    _exportmethods += ['readInputData']
    
    def _get_default_version(self, gaudi_app):
        return guess_version(gaudi_app)
    
               
    def _auto__init__(self):
        self.appname='Kepler'
      #  super(self.appname, self)._auto__init__()

    def postprocess(self):
        XMLPostProcessor.postprocess(self,logger)

    def readInputData(self,optsfiles,extraopts=False):
        def dummyfile():
            temp_fd,temp_filename=tempfile.mkstemp(text=True,suffix='.py')
            os.write(temp_fd,"Dummy file to keep the Optionsparser happy")
            os.close(temp_fd)
            return temp_filename

        if type(optsfiles)!=type([]): optsfiles=[optsfiles]
        
        if len(optsfiles)==0: optsfiles.append(dummyfile())
        
        if extraopts: extraopts=self.extraopts
        
        else: extraopts=""
            
       # parser = check_inputs(optsfiles, extraopts, self.env) 
        try:
            parser = PythonOptionsParser(optsfiles,extraopts,self.getenv(False))
        except Exception, e:
            msg = 'Unable to parse the job options. Please check options ' \
                  'files and extraopts.'
            raise ApplicationConfigurationError(None,msg)

        return GPIProxyObjectFactory(parser.get_input_data())

    def getpack(self, options=''):
        """Performs a getpack on the package given within the environment
           of the application. The unix exit code is returned
        """
        command = 'getpack ' + options + '\n'
        if options == '':
            command = 'getpack -i'
        return CMTscript.CMTscript(self,command)
        
    def make(self, argument=None):
        """Performs a CMT make on the application. The unix exit code is 
           returned. Any arguments given are passed onto CMT as in
           dv.make('clean').
        """
        config = Ganga.Utility.Config.getConfig('GAUDI')
        command = config['make_cmd']
        if argument:
            command+=' '+argument
        return CMTscript.CMTscript(self,command)

    def cmt(self, command):
        """Execute a cmt command in the cmt user area pointed to by the
        application. Will execute the command "cmt <command>" after the
        proper configuration. Do not include the word "cmt" yourself. The 
        unix exit code is returned."""
        command = '###CMT### ' + command
        return CMTscript.CMTscript(self,command)

    def _getshell(self):
        opts = ''
        if self.setupProjectOptions: opts = self.setupProjectOptions

        fd = tempfile.NamedTemporaryFile()
        script = '#!/bin/sh\n'
        if self.user_release_area:
            script += 'User_release_area=%s; export User_release_area\n' % \
                      expandfilename(self.user_release_area)
        if self.platform:    
            script += '. `which LbLogin.sh` -c %s\n' % self.platform
        useflag = ''
        cmd = '. SetupProject.sh %s %s %s %s' % (useflag, opts, self.appname, self.version) 
        script += '%s \n' % cmd
        fd.write(script)
        fd.flush()
        logger.debug(script)

        self.shell = Shell(setup=fd.name)
        if (not self.shell): raise ApplicationConfigurationError(None,'Shell not created.')
        
        logger.debug(pprint.pformat(self.shell.env))
        
        fd.close()
        app_ok = False
        ver_ok = False
        for var in self.shell.env:
            if var.find(self.appname) >= 0: app_ok = True
            if self.shell.env[var].find(self.version) >= 0: ver_ok = True
        if not app_ok or not ver_ok:
            msg = 'Command "%s" failed to properly setup environment.' % cmd
            logger.error(msg)
            raise ApplicationConfigurationError(None,msg)

        import copy
        self.env = copy.deepcopy( self.shell.env )

        return self.shell.env


    def _get_parser(self):
        optsfiles = [fileitem.name for fileitem in self.optsfile]
        # add on XML summary

        extraopts = ''
        if self.extraopts:
            extraopts += self.extraopts
            
        try:
            parser = PythonOptionsParser(optsfiles,extraopts,self.getenv(False))
        except ApplicationConfigurationError, e:
            # fix this when preparing not attached to job
            
            msg2=''
            try:
                debug_dir = self.getJobObject().getDebugWorkspace().getPath()
                msg2+='You can also view this from within ganga '\
                       'by doing job.peek(\'../debug/gaudirun.<whatever>\').'
            except:
                debug_dir = tempfile.mkdtemp()
                    
            messages = e.message.split('###SPLIT###')
            if len(messages) is 2:
                stdout = open(debug_dir + '/gaudirun.stdout','w')
                stderr = open(debug_dir + '/gaudirun.stderr','w')
                stdout.write(messages[0])
                stderr.write(messages[1])
                stdout.close()
                stderr.close()
                msg = 'Unable to parse job options! Please check options ' \
                      'files and extraopts. The output and error streams from gaudirun.py can be ' \
                      'found in %s and %s respectively . ' % (stdout.name, stderr.name)
            else:
                f = open(debug_dir + '/gaudirun.out','w')
                f.write(e.message)
                f.close()
                msg = 'Unable to parse job options! Please check options ' \
                      'files and extraopts. The output from gaudirun.py can be ' \
                      'found in %s . ' % f.name
            msg+=msg2
            # logger.error(msg)
            raise ApplicationConfigurationError(None,msg)
        return parser


    def _parse_options(self):
        try:
            parser = self._get_parser()
        except ApplicationConfigurationError, e:
            raise e

        share_dir = os.path.join(expandfilename(getConfig('Configuration')['gangadir']),
                                 'shared',
                                 getConfig('Configuration')['user'],
                                 self.is_prepared.name)
        fillPackedSandbox([FileBuffer('options.pkl',parser.opts_pkl_str)],
                          os.path.join(share_dir,
                                       'inputsandbox',
                                       '_input_sandbox_%s.tar' % self.is_prepared.name))
        inputdata = parser.get_input_data()
        if len(inputdata.files) > 0:
            logger.warning('Found inputdataset defined in optsfile, '\
                           'this will get pickled up and stored in the '\
                           'prepared state. Any change to the options/data will '\
                           'therefore require an unprepare first.')
            logger.warning('NOTE: the prefered way of working '\
                           'is to define inputdata in the job.inputdata field. ')
            logger.warning('Data defined in job.inputdata will superseed optsfile data!')
            logger.warning('Inputdata can be transfered from optsfiles to the job.inputdata field '\
                           'using job.inputdata = job.application.readInputData(optsfiles)')
            share_path = os.path.join(share_dir,'inputdata')
            if not os.path.isdir(share_path): os.makedirs(share_path)
            f=open(os.path.join(share_path,'options_data.pkl'),'w+b')
            pickle.dump(inputdata, f)
            f.close()

        share_path = os.path.join(share_dir,'output')
        if not os.path.isdir(share_path): os.makedirs(share_path)
        f=open(os.path.join(share_path,'options_parser.pkl'),'w+b')
        pickle.dump(parser, f)
        f.close()

from Ganga.GPIDev.Adapters.ApplicationRuntimeHandlers import allHandlers
for (backend, handler) in backend_handlers().iteritems():
    allHandlers.add('Kepler', backend, handler)
