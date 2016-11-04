#!/bin/bash 

gaudirun.py example.py survey.py --option="from Configurables import Kepler" --option="Kepler().AlignmentFile=\"Alignment_old.dat\""
cp Alignment_out.dat Alignment.dat
gaudirun.py example.py millepede.py --option="from Configurables import TbAlignment" --option="TbAlignment().DoFs = [1,1,0,0,0,1]"
cp Alignment_out.dat Alignment.dat
gaudirun.py example.py millepede.py
cp Alignment_out.dat Alignment.dat 
gaudirun.py example.py alignment.py --option="from Configurables import Kepler" --option="Kepler().Alignment = False"
