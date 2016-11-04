#include "TFile.h"
#include "TH2.h"
#include "TH1.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "iostream"
#include <stdio.h>
#include "Riostream.h"
#include <math.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "TRandom.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TLegend.h"
//script opens the kepler-histograms sent by submitRes.py to the grid, specifically taking the unbiased residual for the DUT (plane 4) for both X and Y. 
//Fits a guassian to each of the residuals and stores the mean and rms. 
//Plots them as a function of Bias or Angle, where the bias and angle values are stored in runList.txt 
//stores them as a .png and .root file format (in LHCb style)
//run as root -b if you dont want canvas printed to screen. 
//MAY need to change path in ~L62 to your own!!!!!
//Emma Buchanan May 2015 (emma.buchanan@cern.ch) 

void res_analysis(){
  //-------Change Parameters Here------//
  const int job = 9; //job number
  const int run =5; //number of subjobs in this job 
  const int beg= 13; //from runList.txt what is the range of the "Block" begining->end
  const int end= 17; 
  int choice =0; //Is this an Bias or angle scan?  (0 for Bias, 1 for angle)
  const int iplane =4; //number of the plane you wish to analyse (DUT =4)

  //saving the final plot as a .png, and/or root file 
  char plot_bias[200]; //the name of the plot, will be printed to a .png at the end of this script.
  sprintf(plot_bias, "%s", "sensor_bias_scan.png");
  char plot_angle[200];
  sprintf(plot_angle, "%s", "sensor_angle_scan.png");
  char root_plotb[100]; //this .root file will contain the final plot/s
  sprintf(root_plotb, "%s", "sensor_bias_scan.root");
  char root_plota[100]; //this .root file will contain the final plot/s
  sprintf(root_plota, "%s", "sensor_angle_scan.root");
  //----------end of parameters--------//


  //Reading kepler histograms and fitting gaussian to DUT
  float DUT_meanX[run];  float DUT_rmsX[run]; //fit parameters for X
  float DUT_meanY[run];  float DUT_rmsY[run]; //fit parameters for Y
  TF1 *gauss = new TF1("gauss","gaus");
  TH1F *XDUT[run];
  TH1F *YDUT[run];
  TCanvas *Xcanvas[run];
  TCanvas *Ycanvas[run];
 
  //opening the DUT (plane 4) from the kepler-histos and fitting a gaussian to each.  
  char filename[100];
  char locationX[100];  char locationY[100];
  TFile *openFile[100]; 
  for (int subjob =0; subjob<run; subjob++){
      sprintf(filename, "%s%d%s%d%s","/afs/cern.ch/user/e/ebuchana/gangadir/workspace/ebuchana/LocalXML/",job,"/",subjob,"/output/Kepler-histos.root "); //the location of each root file for each run 
      sprintf(locationX, "%s%d", "Tb/TbTrackPlots/BiasedResiduals/GlobalX/Plane",iplane); //location of the biased residual of the DUT
      sprintf(locationY, "%s%d", "Tb/TbTrackPlots/BiasedResiduals/GlobalY/Plane",iplane); //location of the biased residual of the DUT
      cout << filename << endl;
      openFile[subjob] = new TFile(filename); //opening the root files

      //------------------ For X ------------------//
      cout << locationX << endl;
      Xcanvas[subjob] = new TCanvas(); //creating a canvas for each histogram 
      Xcanvas[subjob]->SetTitle("X Biased Residal for DUT"); //setting canvas title 
      XDUT[subjob] = (TH1F*)openFile[subjob]->Get(locationX); //getting each of the histograms from the kepler root files
      XDUT[subjob]->Fit("gauss", "R"); //fitting guassian
      DUT_meanX[subjob]=XDUT[subjob]->GetFunction("gauss")->GetParameter(1); //getting the mean of the postion 
      DUT_rmsX[subjob]=XDUT[subjob]->GetFunction("gauss")->GetParameter(2); //this should be the RMS 
      cout << "DUT mean X\t" << DUT_meanX[subjob] <<endl;
      cout << "DUT rms X\t" << DUT_rmsX[subjob] <<endl;      

      //------------------ For Y ------------------//
      cout << locationY << endl;
      Ycanvas[subjob] = new TCanvas(); //creating a canvas for each histogram
      Ycanvas[subjob]->SetTitle("Y Biased Residal for DUT"); //setting canvas title      
      YDUT[subjob] = (TH1F*)openFile[subjob]->Get(locationY); //getting each of the histograms from the kepler root files      
      YDUT[subjob]->Fit("gauss", "R"); //fitting guassian      
      DUT_meanY[subjob]=YDUT[subjob]->GetFunction("gauss")->GetParameter(1); //getting the mean of the postion      
      DUT_rmsY[subjob]=YDUT[subjob]->GetFunction("gauss")->GetParameter(2); //this should be the RMS      
      cout << "DUT mean Y\t" << DUT_meanY[subjob] <<endl;
      cout << "DUT rms Y\t" << DUT_rmsY[subjob] <<endl;

  }

  //Printing to the DUT histograms and gaussian fits to a new root file 
  char rootFile[100];
  sprintf(rootFile, "%s", "root_res_analysis.root");
  cout << "creating\t" << rootFile << endl;
  TFile *myfile =  new TFile(rootFile,"RECREATE"); // creating rootfile
  for (int subjob =0; subjob<run; subjob++){
    XDUT[subjob]->Write(); //writing the plots to file 
    YDUT[subjob]->Write(); //writing the plots to file 
  }
  
  //Reading in the parameters from runList.txt
  char runList[100];
  char str1[10],str2[10],str3[10],str4[10],str5[10],str6[10];
  float a,b,c,f; char d[100], e[100]; 
  float full_Angle[100], full_Bias[100], length[100];
  float Angle[run], Bias[run], rmsX[run];
  int r =0;
  sprintf(runList, "%s", "/afs/cern.ch/user/e/ebuchana/cmtuser/KEPLER/KEPLER_HEAD/Tb/Kepler/options/ResStudies/runList.txt");
 
  FILE * runFile = fopen (runList,"read");
  cout << "reading in runList.txt " << endl;
  fscanf(runFile, "%s \t %s \t %s \t %s \t %s \t %s", str1,str2,str3,str4,str5,str6);
  
  while (!feof(runFile)){
    fscanf(runFile,"%f \t %f \t %f \t %s \t %s \t %f", &a, &b, &c, d, e, &f); 
    full_Angle[r]=b;// all angles in runList.txt
    full_Bias[r]=c; // all bias voltages in runList.txt
    length[r]=f; // number of line in runList.txt
    r++;
  }
  
  //initalisng arrays (may not be needed)
  for (int range =beg-1; range <end; range ++){
    Angle[range]=0;
    Bias[range]=0;
  }
 
  //saving the range of bias and angles that you need from the full set of parameters  
  int in =0;
  for (int range =beg-1; range <end; range ++){
    Angle[in]=full_Angle[range];
    Bias[in]=full_Bias[range];
    in++;
  }

   
  //Plots after this line will be in the LHCb style format.
  gROOT->  ProcessLine(".x lhcbStyle.C");
  
  
  if (choice==0){ //if Bias scan
    //--------- for X ----------//
    TCanvas *c1 = new TCanvas("c1", "Bias vs Xrms (DUT)");
    TGraph *g1 = new TGraph(run, Bias, DUT_rmsX); 
    g1->Draw("ap");
    g1->SetTitle("Bias vs. DUT rms; Bias Voltage; DUT X spatial resolution "); 
    c1->Print(plot_bias);
    c1->Print(root_plotb);
    //--------- for Y ----------//
    TCanvas *c2 = new TCanvas("c2", "Bias vs Yrms (DUT)");
    TGraph *g2 = new TGraph(run, Bias, DUT_rmsY);
    g2->Draw("ap");
    g2->SetTitle("Bias vs. DUT rms; Bias Voltage; DUT Y spatial resolution ");
      // c2->Print(plot_bias);
      //  c2->Print(root_plotb);
    
  }
  
  else{ //if angle scan
    
    TCanvas *c3 = new TCanvas("c3", "Angle vs Xrms (DUT)");
    TGraph *g3 = new TGraph(run, Angle, DUT_rmsX); 
    g3->Draw("ap");
    c3->Print(plot_angle);
    c3->Print(root_plota);
    
    TCanvas *c4 = new TCanvas("c4", "Angle vs Yrms (DUT)");
    TGraph *g4 = new TGraph(run, Angle, DUT_rmsY);
    g4->Draw("ap");
    // c4->Print(plot_angle);
    // c4->Print(root_plota);
    
  }
  
  
}



