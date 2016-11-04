########################################################################
#  script to submit resolution study jobs to grid
#  Define runs in runList.txt and combine those in a common block
#  Define z-position configuration of block in ZPos.txt
#  Define which block to run over and analysis step you want to perform
#      -survey   :  Survey alignment
#      -mille    :  Millepede alignmnet
#      -dut      :  DUT run
#      -run      :  Dry run using DUT alignment
#
#  for questions:   chris.hombach@gmail.com
########################################################################
import pickle
from sys      import path
from os.path  import abspath

PATH_TO_OPTS= abspath('$HOME/cmtuser/KEPLER/KEPLER_HEAD/Tb/Kepler/options/ResStudies/')
path.append( PATH_TO_OPTS )
import Runs

#Set blocks to run over and alignment-method
blocks   = ['D1','D2']
method   = 'run'  #survey, mille, dut, run
######

optsdir  = '$HOME/cmtuser/KEPLER/KEPLER_HEAD/Tb/Kepler/options/ResStudies/'

opts     = '%srestudies.py' % optsdir
kepler   = Kepler( optsfile=[ opts ] , version = 'HEAD' )
BACKEND  = Local()#LSF(queue='1nh')#Dirac()
#BACKEND  = Dirac()

runs     = Runs.Runs(blocks, method)
pickled_runs = 'Runs.pkl'
pickle.dump( runs, open( pickled_runs , 'wb' ) )


   

for bl in blocks:
    alfs     = {}
    tbruns   = []    
    for rn in runs.RUNS[bl]:
        r  = rn.RUN
        a  = rn.ANGLE
        b  = rn.BIAS
        d  = rn.DUT
        af = rn.ALIGNFILE 
        tbruns.append( int(r) )
        if method == 'survey':
            rn.createAlignFile()
        alfs[int(r)] = af

    if tbruns[0] < 2000:
        m = 'July2014'
    elif tbruns[0] < 3000:
        m = 'Oct2014'
    elif tbruns[0] < 4000:
        m = 'Nov2014'
    else:
        m = 'Dec2014'
    tbds     = TbDataset( m, tbruns )
    tbds.AlignmentFiles = alfs


    SPLITTER = tbds
    j        = Job(application = kepler)
    j.backend     = BACKEND
    j.splitter    = SPLITTER
    j.outputfiles = ['*.dat']

    j.submit()
