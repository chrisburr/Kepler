ROOTC=$(shell root-config --cflags)
ROOTL=$(shell root-config --glibs)

all: combDUTwithTrack

combDUTwithTrack: combDUTwithTrack.cpp
	g++ $(ROOTC) -o $@ $^ $(ROOTL) 


clean:
	rm -f combDUTwithTrack
