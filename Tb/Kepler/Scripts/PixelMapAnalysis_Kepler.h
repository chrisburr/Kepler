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
#include <string>

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

......................................................................
 */ 

using namespace std;
 
void PixelMapAnalysis(TH2D* hist, unsigned int plane, ostream& os = std::cout ){
 
  for (int col=0;col<256;col++){
    for (int row=0;row<256;row++)
    {
       double count =     hist->GetBinContent(col+1,row+1);   

       vector<double> neighbours;
        for( int i = 0 ; i < 3; i++)
        {
          for(int j=0; j < 3; j++)
          {
            if( i == 1 && j == 1) continue;
            neighbours.push_back( hist->GetBinContent( col +i, row +j ));
          }
        }
	double total(0.);
      if (col>0&&col<255&&row>0&&row<255) {
	for (uint i=2;i<neighbours.size()-2;i++) total=total+neighbours[i];

	if (count >10*total)
	  os << plane << " "<< col << " "<< row << endl;
      }
      else  {
	for (uint i=0;i<neighbours.size();i++)
	  total=total+neighbours[i];

	if (count >10*total)
	  os << plane << " "<< col << " "<< row << endl;
      }
    }
  }
}
