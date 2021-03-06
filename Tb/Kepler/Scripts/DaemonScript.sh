#!/bin/bash
. /etc/bashrc

shopt -s expand_aliases

EOS_PATH=/eos/lhcb/testbeam/velo/timepix3/Nov2015/
EOS_CMD=/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select
KEPLER=/afs/cern.ch/user/t/tevans/cmtuser/KEPLER/KEPLER_v3r0/Tb/Kepler/Scripts/Kepler
OPT_PATH=/afs/cern.ch/user/t/tevans/cmtuser/KEPLER/KEPLER_v3r0/Tb/Kepler/options/
OutputFolder=$EOS_PATH/RootFiles/Run$1/
hist=$(pwd)/Kepler-histos.root
tree=$(pwd)/Kepler-tuple.root 

if [ -n "$2" ] ; then
  echo "Running alignment of run $1 ($2)"
  AlignmentFile=eos/lhcb/user/t/tevans/Alignment/Nov2015/Default_$2.dat
  $KEPLER $1 -a $AlignmentFile -o $OPT_PATH/align.py -h $hist -t $tree | tee Alignment_Run$1.std.out
  $EOS_CMD cp $(pwd)/Alignment_out.dat $EOS_PATH/RootFiles/Run$1/Conditions/Alignment$1mille.dat
  $EOS_CMD cp $(pwd)/Alignment_Run$1.std.out $EOS_PATH/RootFiles/Run$1/Output/Alignment_Run$1.std.out
fi

$KEPLER $1 -o $OPT_PATH/tuple.py -h $hist -t $tree | tee Batch_Run$1.std.out

$EOS_CMD cp $(pwd)/Batch_Run$1.std.out $OutputFolder/Output/Batch_Run$1.std.out
$EOS_CMD cp $(pwd)/Kepler-histos.root  $OutputFolder/Output/Kepler-histos.root
$EOS_CMD cp $(pwd)/Kepler-tuple.root   $OutputFolder/Output/Kepler-tuple.root

