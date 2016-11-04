ROOTC=$(shell root-config --cflags)
ROOTL=$(shell root-config --glibs)

all: analyzeCalib

analyzeCalib: analyzeCalib.cpp
	g++ $(ROOTC) -o $@ $^ $(ROOTL) 


clean:
	rm -f analyzeCalib
