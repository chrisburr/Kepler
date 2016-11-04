#!/bin/bash 

RUNNUMBER=$1

cp Alignment.dat Alignment_old.dat
gaudirun.py example.py minuit_alignment.py \
   --option="from Configurables import Kepler, TbAlignment" \
   --option="Kepler().InputFiles	= [\"eos/lhcb/testbeam/velo/timepix3/July2014/RawData/Run$RUNNUMBER/\"]" \
   --option="Kepler().PixelConfigFile = \"eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run$RUNNUMBER/Conditions/PixelMask.dat\"" \
   --option="TbAlignment().AlignmentTechnique = \"Method1\"" 

cp Alignment_out.dat Alignment.dat
gaudirun.py example.py minuit_alignment.py \
   --option="from Configurables import Kepler, TbAlignment" \
   --option="Kepler().InputFiles	= [\"eos/lhcb/testbeam/velo/timepix3/July2014/RawData/Run$RUNNUMBER/\"]" \
   --option="Kepler().PixelConfigFile = \"eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run$RUNNUMBER/Conditions/PixelMask.dat\"" \
   --option="TbAlignment().AlignmentTechnique = \"Method0\"" 

cp Alignment_out.dat Alignment.dat
gaudirun.py example.py minuit_alignment.py \
   --option="from Configurables import Kepler, TbAlignment" \
   --option="Kepler().InputFiles	= [\"eos/lhcb/testbeam/velo/timepix3/July2014/RawData/Run$RUNNUMBER/\"]" \
   --option="Kepler().PixelConfigFile = \"eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run$RUNNUMBER/Conditions/PixelMask.dat\"" \
   --option="TbAlignment().AlignmentTechnique = \"Method0\"" --option="TbAlignment().FitStrategy = 2"

cp Alignment_out.dat Alignment_$RUNNUMBER.dat
gaudirun.py example.py minuit_alignment.py \
   --option="from Configurables import Kepler" \
   --option="Kepler().InputFiles	= [\"eos/lhcb/testbeam/velo/timepix3/July2014/RawData/Run$RUNNUMBER/\"]" \
   --option="Kepler().PixelConfigFile = \"eos/lhcb/testbeam/velo/timepix3/July2014/RootFiles/Run$RUNNUMBER/Conditions/PixelMask.dat\"" \
   --option="Kepler().Alignment = False" \
   --option="Kepler().Monitoring = True"
mv Kepler-histos.root Kepler-histos_$RUNNUMBER.root

