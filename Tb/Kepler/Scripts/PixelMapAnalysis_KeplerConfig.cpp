#include "TFile.h"
#include "TH2D.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "TROOT.h"
#include "TKey.h"
#include "TIterator.h"

/*
......................................................................
Author: Hella Snoek (hella.snoek@cern.ch)
Date:   22.07.2014

Simple ROOT analysis code to analyse the pixel hit maps for 
 - DEAD: no hits
 - HOT:  too many hits
 - LOW:  too few hits
pixel hits. 

A pixel cell is compared to its 8 neighbouring (exception for pixel on
the edge below) pixel cells. The 2 highest and 2 lowest hit counts
are removed before making an average of the surrounding pixel cells.
The minimum required count in a cell for the analysis is 20. 
A HOT cell is 40 times higher than the average. 
A LOW cell is 4  std deviations (sqrt) lower.

For the cells on the edge of the sensor the highest and lowest hits
are not removed for the average. 

Three text files are produced. With the format
col row

Run this code as:
root -b -q PixelMapAnalysis.cpp+\(\"inputfilename\",\"outputname\"\)
or
root -b -q PixelMapAnalysis.cpp+\(\"inputfilename\",\"outputname\",true\)
for the verbose version.

or call it with a hitmap:
.L PixelMapAnalysis.cpp+
PixelMapAnalysis(yourTH2D,"outputname",true);


......................................................................
 */
 .* 

#include <iostream>
#include <fstream>

using namespace std;
 
void PixelMapAnalysis(TH2D* hist, unsigned int plane, ostream& os = std::cout ){
 
  for (int col=0;col<256;col++){
    for (int row=0;row<256;row++){
      double count =     hist->GetBinContent(col+1,row+1);   

      vector<double> neighbours{
      	hist->GetBinContent(col,row),   
 	  hist->GetBinContent(col+1,row),   
 	  hist->GetBinContent(col+2,row),
 	  hist->GetBinContent(col,row+1),   
 	  hist->GetBinContent(col+2,row+1),
 	  hist->GetBinContent(col,row+2),
 	  hist->GetBinContent(col+1,row+2),
 	  hist->GetBinContent(col+2,row+2)
 	  }  ;


      if (col>0&&col<255&&row>0&&row<255) {
	std::sort( neighbours.begin(),neighbours.end());
	neighbours.erase(neighbours.begin(),neighbours.begin()+2);
	neighbours.erase(neighbours.end()-2,neighbours.end());
	double total(0.);
	for (uint i=0;i<neighbours.size();i++) {
	  //cout << neighbours[i] << " " ;
	  total=total+neighbours[i];
	  //	  cout << total<< " ";
	}

	if (count >10*total) {
	  os << plane << setw(3) << " "<< setw(3) << col << " "<< row << endl;
	}
      }
      
      else  {
	std::sort( neighbours.begin(),neighbours.end());
	double total(0);
	int active(neighbours.size());
	for (uint i=0;i<neighbours.size();i++) {
	  active=active-1;
	  total=total+neighbours[i];
	}
	if (count >10*total) {
	  os << setw(3) << col << " " << row <<  endl;
	  continue;
	}
      }
    }
  }
}


void PixelMapAnalysis_KeplerConfig(const char* filename){
  
  cout << "opening filename" << endl;
  TFile file(filename,"read");
  file.GetDirectory("Tb/TbHitMonitor/HitMap")->GetListOfKeys()->Print();

  ofstream hotF(Form("%s_hot.dat",filename));
  TIter nextkey(file.GetDirectory("Tb/TbHitMonitor/HitMap")->GetListOfKeys());

  TKey *key;
  
  while((key= (TKey*) nextkey())) {
    TH2D *hist = (TH2D*) key->ReadObj();
    std::cout << "-------Now processing: " << key->GetName() << "-----------" <<endl;
    std::string name = key->GetName();
    int plane(0);
    sscanf( &name.back(), "%d", &plane );
    PixelMapAnalysis(hist, plane, hotF);
  }
  hotF.close();
}
