/////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Oct  1 20:13:36 2015 by ROOT version 5.34/10
// from TTree Clusters/TbUT nTuple
//////////////////////////////////////////////////////////

#ifndef ExampleAnalysis_h
#define ExampleAnalysis_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLegend.h>
#include <TBox.h>
#include <TLine.h>

#include "AnalysisBase.h"
#include "CMS.h"

//#include "ExampleAnalysis_Inputs.h"

// Header file for the classes stored in the TTree if any.
#include <vector>
#include <iostream>
#include <algorithm>

// Fixed size dimensions of array or collections stored in the TTree if any.

class ExampleAnalysis  : public AnalysisBase{
public :
   
  CMS            *fCMS;

   ExampleAnalysis(TTree *tree=0);
   virtual ~ExampleAnalysis();
   virtual void     Loop();
  virtual TString getFileBase(TString scan="Bias", TString board="", TString bias="", TString angle="0", TString sector="1");  

  TFile *fout;
};

#endif

#ifdef ExampleAnalysis_cxx
ExampleAnalysis::ExampleAnalysis(TTree *tree){
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
  TString filename;
   if (tree == 0) {
     //
     // lhcb-dev
     //
     filename = m_fileIndir+"Board"+m_board+"/Run_Bias_Scan-"+getFileBase("Bias",m_board, m_bias, "0",m_sector)+"_Tuple_Tracks.root"; 

     // lxplus
     //
     //filename = "/afs/cern.ch/work/s/sblusk/public/TB/2015/BoardA8/Run_Bias_Scan-B8-A-296-13332_Tuple_Tracks.root"; // A8 - s1
     //filename = "/afs/cern.ch/work/s/sblusk/public/TB/2015/BoardA8/Run_Bias_Scan-B8-A-324-13355_Tuple_Tracks.root"; // A8 - s2
     //filename = "/afs/cern.ch/work/s/sblusk/public/TB/2015/BoardA8/Run_Bias_Scan-B8-A-359-13386_Tuple_Tracks.root";  // A8 - s3
     //filename = "/afs/cern.ch/work/s/sblusk/public/TB/2015/BoardA6/Run_Bias_Scan-B6-A-212-8358_Tuple_Tracks.root";   // A6
     //

     TFile *f = new TFile(filename);
     tree = (TTree*)f->Get("Clusters");

     // Get mean noise and width
     hMeanNoise 	= (TH1F*)f->Get("hMeanNoise"); 
     hWidthNoise 	= (TH1F*)f->Get("hWidthNoise"); 

     TString filename2 = filename.ReplaceAll("_Tracks","");
     TFile * f2 = new TFile(filename2);
     if(f2) {
       TTree * tree2 = (TTree*) f2->Get("TbUT/CMS");
       if(tree2) {
         tree->AddFriend(tree2);
         fCMS = new CMS(tree2);
       }else{
         fCMS=0;  
       }
     } else {
       cout << "WARNING: Could not find CMS data file: " << filename2 << endl;
       fCMS =0 ;
     }
   }
   Init(tree);
}

ExampleAnalysis::~ExampleAnalysis()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}
TString ExampleAnalysis::getFileBase(TString scan, TString board, TString bias, TString angle, TString sector) {

  TString tag = "";

  if(board == "A6"){
    if(sector == "1") {
      if ( bias == "300" ) tag = "B6-A-212-8358";
    } else if(sector == "2"){
      if ( bias == "300" ) tag = "B6-A-242-8389";
    } else if(sector == "3"){
      if ( bias == "300" ) tag = "B6-A-293-8425";
    } else if(sector == "4"){
      if ( bias == "300" ) tag = "B6-A-326-8452";
    } else if(sector == "5"){
      if ( bias == "300" ) tag = "B6-A-377-8494";
    } else if(sector == "6"){
      if ( bias == "250" ) tag = "B6-A-409-8524";
    }
  } else if (board == "A4") {
    if(sector == "1") {
      if(bias == "400" ) tag = "B4-A-210-8552";
    } else if(sector == "3"){
      if(bias == "400" ) tag = "B4-A-275-8615";
    }
  } else if (board == "A8") {
    if(sector == "1"){
      if(bias == "500") tag = "B8-A-296-13332";
    }else if (sector == "2"){
      if(bias == "400") tag = "B8-A-324-13355";
    }else if (sector == "3"){
      if(bias == "400") tag = "B8-A-359-13386";
    }
  }

  return tag;
}


#endif // #ifdef ExampleAnalysis_cxx
