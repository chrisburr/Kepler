//--------------------------------------------------------------------------------------------------------------------------------------------------------
//
// This code will append tracking information to a ROOT tree that contains cluster information from a UT DUT 
//
// To compile
//   > SetupProject LHCb
//   > make
//
// Example of how to run interactively:
// 
// ./combDUTwithTrack -i /data2/pmmannin/BoardA4_redo_v2/Run_Bias_Scan-B1-A-227-8711_Tuple.root -t /data2/sblusk/TB/July2015/TelescopeData/BoardA4/Kepler-tuple.root -o /data2/sblusk/test2.root
//
//
// The Telescope data is on eos at:~/eos/lhcb/testbeam/ut/TelescopeData/July2015/RootFiles/RunXXXX/Output/Kepler-tuple.root
// where XXXX = run number from TPIX DAQ system (Kepler run #)
//
// The DUT data needs to be processed through TbUT and put on eos in a standard area.
// 
// The easiest, although ot safest way, to get the data directly accessible from eos is to use:
// eosmount ~/eos
//--------------------------------------------------------------------------------------------------------------------------------------------------------


#include <iostream>
#include <vector>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TCanvas.h"

#ifdef __MAKECINT__
#pragma link C++ class vector<float>+;
#endif

using namespace std;



int main(int __argc, char *__argv[]){

  gROOT->ProcessLine("#include <vector>");
  const char *calib_filename  = "calib.root";
  const char *out_filename    = "outputfile.root";

  int c;
  extern char* optarg;
  extern int optind;

  int channel; 
  int type; 
  cout << endl;
 

  cout << "number of arguments: " << __argc << endl;  
  for(int i=0; i < __argc; i++) {

    cout << __argv[i] << endl; 
  }
  /*____________________________Parse Command Line___________________________*/
  while((c = getopt(__argc,__argv,"i:o:n:t:p")) != -1){
    switch(c){
      case 'h': // help option
        cout << "Example: ./analyzeCalib -i calib.root -o outputfile.root -n channel -t type" << endl; 
        return 0;
        break;
      case 'i':
        calib_filename = optarg;
        cout << "---> Calibration ROOT filename: " << calib_filename << endl;
        break;
      case 'o':
        out_filename = optarg;
        cout << "--->  Output filename: " << out_filename << endl;
        break;
      case 'n':
        channel = atoi(optarg); 
        cout << "Channel: " << channel << endl; 
        break; 
      case 't':
        type = atoi(optarg); 
        cout << "Type: " << type << endl; 
        break; 
      default: // unknown option flag
        printf("Error!!!! Unknown option -%c\n",c);
        cout << "Example: analyzeCalib -i calib.root -o outputfile.root -n channel -t type" << endl; 
      return 0;
    }
  }

  cout << endl;

   Int_t           clusterNumberPerEvent;
   UInt_t          clustersTDC;
   ULong64_t       timestamps;
   Double_t        clustersPosition[10];
   Int_t           clustersSeedPosition[10];
   Double_t        clustersCharge[10];
   Int_t           clustersSize[10];
   Double_t        clustersSeedCharge[10];
   Double_t        clustersCharge2StripLeft[10];
   Double_t        clustersCharge1StripLeft[10];
   Double_t        clustersCharge1StripRight[10];
   Double_t        clustersCharge2StripRight[10];

   // List of branches
   TBranch        *b_clusterNumberPerEvent;   //!
   TBranch        *b_clustersTDC;   //!
   TBranch        *b_timestamps;   //!
   TBranch        *b_clustersPosition;   //!
   TBranch        *b_clustersSeedPosition;   //!
   TBranch        *b_clustersCharge;   //!
   TBranch        *b_clustersSize;   //!
   TBranch        *b_clustersSeedCharge;   //!
   TBranch        *b_clustersCharge2StripLeft;   //!
   TBranch        *b_clustersCharge1StripLeft;   //!
   TBranch        *b_clustersCharge1StripRight;   //!
   TBranch        *b_clustersCharge2StripRight;   //!

 
  //dut_filename = "/data2/pmmannin/BoardA4_redo_v2/Run_Bias_Scan-B1-A-227-8711_Tuple.root";
  TFile *f_calib 		= new TFile(calib_filename,"READONLY");
	TTree* t_dut 		= (TTree*)f_calib->Get("TbUT/Clusters");

  t_dut->SetBranchAddress("clusterNumberPerEvent", &clusterNumberPerEvent, &b_clusterNumberPerEvent);
  t_dut->SetBranchAddress("clustersTDC", &clustersTDC, &b_clustersTDC);
  t_dut->SetBranchAddress("timestamps", &timestamps, &b_timestamps);
  t_dut->SetBranchAddress("clustersPosition", clustersPosition, &b_clustersPosition);
  t_dut->SetBranchAddress("clustersSeedPosition", clustersSeedPosition, &b_clustersSeedPosition);
  t_dut->SetBranchAddress("clustersCharge", clustersCharge, &b_clustersCharge);
  t_dut->SetBranchAddress("clustersSize", clustersSize, &b_clustersSize);
  t_dut->SetBranchAddress("clustersSeedCharge", clustersSeedCharge, &b_clustersSeedCharge);
  t_dut->SetBranchAddress("clustersCharge2StripLeft", clustersCharge2StripLeft, &b_clustersCharge2StripLeft);
  t_dut->SetBranchAddress("clustersCharge1StripLeft", clustersCharge1StripLeft, &b_clustersCharge1StripLeft);
  t_dut->SetBranchAddress("clustersCharge1StripRight", clustersCharge1StripRight, &b_clustersCharge1StripRight);
  t_dut->SetBranchAddress("clustersCharge2StripRight", clustersCharge2StripRight, &b_clustersCharge2StripRight);

  int numEvents = t_dut->GetEntriesFast();
  cout << " ------------------------------------------------------" << endl;
  cout << " | Number of events found = " << numEvents << endl;
  cout << " ------------------------------------------------------" << endl;

  vector<int> maxStrips;

  int nfound=0;  
  //out_filename = "/data2/sblusk/test.root";
  TFile *f_out = new TFile(out_filename,"recreate");

  //TFile *fileout = new TFile("temp.root","RECREATE");
  TH1F *h_chg       = new TH1F("h_chg","Cluster Charge",200,-2000,2000);
  TH1F *h_chgLeft   = new TH1F("h_chgLeft","Charge Charge (ch-1)",200,-2000,2000);
  TH1F *h_chgLeft2   = new TH1F("h_chgLeft2","Charge Charge (ch-2)",200,-2000,2000);
  TH1F *h_chgRight   = new TH1F("h_chgRight","Charge Charge (ch+1)",200,-2000,2000);
  TH1F *h_chgRight2  = new TH1F("h_chgRight2","Charge Charge (ch+2)",200,-2000,2000);
  TH1F *h_chgDiff   = new TH1F("h_chgDiff","Charge_{left}-Charge_{right}",200,-2000,2000);
  TH1F *h_chgDiff2   = new TH1F("h_chgDiff2","Charge_{s-2}-Charge_{s+2}",200,-2000,2000);
  TH1F *h_seed      = new TH1F("h_seed","Seed Strip",513,-0.5,512.5); 
  TH1F *h_pos       = new TH1F("h_pos","Strip",513,-0.5,512.5);

 int strip; 
 int left; 
 int right;  

  for(int i=0;i < numEvents; i++){
    int nb = t_dut->GetEntry(i);
    if(nb <= 0) break;
    for(int j=0; j < min(clusterNumberPerEvent,10);j++) {
      h_pos->Fill(clustersPosition[j]); 
    } 
  }


  /*
  for(int i=1; i < 511; i++) {
    if(h_pos->GetBinContent(i) > 2500) {
      maxStrips.push_back((int)h_pos->GetBinCenter(i));
      cout << (int)h_pos->GetBinCenter(i) << endl;
    }
  }

  for(int i=0; i < maxStrips.size(); i++) {
    cout << "Max Strip " << i << ": " << maxStrips.at(i) << endl;
  }
  */


  if(type == 0) strip = channel+128;
  else if(type == 1) strip = channel; 
  else { cout << "Please enter a valid type: 0 for A, 1 for D, 2 for Micron Mini, 3 for Ham Mini" << endl; return 0; }

  left  = strip-1; 
  right = strip+1; 

  for(int i=0;i < numEvents; i++){
    int nb = t_dut->GetEntry(i);
    if(nb <= 0) break;
    double chgLeft  = 0.0; 
    double chgRight = 0.0; 
    for(int j=0; j < min(clusterNumberPerEvent,10);j++) {
      if(fabs( clustersPosition[j]-strip) < 0.5 ) {
        h_chg->Fill(clustersCharge[j]);
        h_seed->Fill(clustersSeedPosition[j]); 
        h_chgLeft->Fill(clustersCharge1StripLeft[j]);
        h_chgRight->Fill(clustersCharge1StripRight[j]);
        h_chgLeft2->Fill(clustersCharge2StripLeft[j]);
        h_chgRight2->Fill(clustersCharge2StripRight[j]);
        h_chgDiff->Fill(clustersCharge1StripLeft[j]-clustersCharge1StripRight[j]); 
        h_chgDiff2->Fill(clustersCharge2StripLeft[j]-clustersCharge2StripRight[j]); 
      }
    }
  }

  TCanvas *c1 = new TCanvas("c","c",1200,800);

  c1->Divide(3,3);
  c1->cd(1)->SetLeftMargin(0.13);
  h_chg->SetTitle(TString(Form("Seed Channel %d (strip %d)",channel,strip)));
  h_chg->GetXaxis()->SetTitle("Cluster Charge");
  h_chg->Draw();

  c1->cd(2)->SetLeftMargin(0.13);
  h_chgLeft->SetTitle(TString(Form("Left Channel: %d (strip %d)",channel-1,left)));
  h_chgLeft->GetXaxis()->SetTitle("Cluster Charge, N-1");
  h_chgLeft->Draw();
  
  c1->cd(3)->SetLeftMargin(0.13);
  h_chgRight->SetTitle(TString(Form("Right Channel: %d (strip %d)",channel+1,right)));
  h_chgRight->GetXaxis()->SetTitle("Cluster Charge, N+1");
  h_chgRight->Draw();

  c1->cd(4)->SetLeftMargin(0.13);
  h_chgLeft2->SetTitle(TString(Form("2^{nd} Left Channel: %d (strip %d)",channel-2,left-1)));
  h_chgLeft2->GetXaxis()->SetTitle("Cluster Charge, N-2");
  h_chgLeft2->Draw();
  
  c1->cd(5)->SetLeftMargin(0.13);
  h_chgRight2->SetTitle(TString(Form("2^{nd} Right Channel: %d (strip %d)",channel+2,right+1)));
  h_chgRight2->GetXaxis()->SetTitle("Cluster Charge, N+2");
  h_chgRight2->Draw();


  c1->cd(6)->SetLeftMargin(0.13);
  h_chgDiff->GetXaxis()->SetTitle("Cluster Charge Difference (s-1,s+1)");
  h_chgDiff->Draw();
  
  c1->cd(7)->SetLeftMargin(0.13);
  h_chgDiff2->GetXaxis()->SetTitle("Cluster Charge Difference (s-2,s+2)");
  h_chgDiff2->Draw();
  
  c1->cd(8)->SetLeftMargin(0.13);
  h_seed->GetXaxis()->SetTitle("Seed Cluster Position");
  h_seed->GetXaxis()->SetRangeUser(h_seed->GetMaximumBin()-10,h_seed->GetMaximumBin()+10); 
  h_seed->Draw();

  


  c1->SaveAs(TString(Form("Calibration_Type%d_Channel%d.png",type,channel)));

  // Write and close ROOT file.
  f_out->Write();
  f_out->Close();

  return 0;
}
