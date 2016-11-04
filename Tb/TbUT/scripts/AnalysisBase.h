//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Oct  1 20:13:36 2015 by ROOT version 5.34/10
// from TTree Clusters/TbUT nTuple
// found on file: /data2/pmmannin/BoardA6/Run_Bias_Scan-B6-A-212-8358_Tuple_tracks.root
//////////////////////////////////////////////////////////

#ifndef AnalysisBase_h
#define AnalysisBase_h

#include <TROOT.h>
#include <iostream>
#include <TChain.h>
#include <TTree.h>
#include <TMath.h>
#include <TFile.h>
#include <TH2.h>
#include <TF1.h>
#include <TProfile.h>
#include <TString.h>
#include <TGraph.h>
#include <TGraphErrors.h>
//#include "myInputs.h"
#include "AnalysisBase_Inputs.h"

// Header file for the classes stored in the TTree if any.
#include <vector>



// Fixed size dimensions of array or collections stored in the TTree if any.

class AnalysisBase {
 public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain
  
  // Declaration of leaf types

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
  Double_t        dtime;
   
  Int_t           n_tp3_tracks;
  vector<double>  *vec_trk_x;
  vector<double>  *vec_trk_y;
  vector<double>  *vec_trk_tx;
  vector<double>  *vec_trk_ty;
  vector<double>  *vec_trk_chi2ndf;

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
  TBranch        *b_n_tp3_tracks;   //!
  TBranch        *b_vec_trk_x;   //!
  TBranch        *b_vec_trk_y;   //!
  TBranch        *b_vec_trk_tx;   //!
  TBranch        *b_vec_trk_ty;   //!
  TBranch        *b_vec_trk_chi2ndf;   //!
  TBranch        *b_dtime;   //!

  double stripPitch;
  double z_DUT;
  double Rz;
  double Ry;
  double dxWin;
  double xGloOff;
  double yGloOff;
  double xOff;
  

  float iLo, iHi;
  float tdcLo, tdcHi;
  float yMin,yMax;
  float xMin,xMax;
  float yMid, yHi2;
  float tyMin,tyMax;
  float txMin,txMax;
  Int_t skipChannel[512];
  double xLeftHole;
  double xRightHole;
  
  Int_t lowEdge, hiEdge;
  Double_t stripGap[4];
  Double_t deadRegionLo[20];
  Double_t deadRegionHi[20];
  Int_t nDeadRegion;  

  double yInt1[2];
  double yInt2[2];
  double yInt3[2];

  float polarity;

  TH1F *hMeanNoise; 
  TH1F *hWidthNoise; 

  double holeQuadPar[3];
  bool removeTracksInHole;
  double minDistFromHole;
  bool holeSector;
  bool correctForZRotation;

  double chargeCorrSlopeOdd;
  double chargeCorrSlopeEven;
  
  double noise[512];
  int badStrips[512];
  int nbadStrips;

  double channelOffset;  

  AnalysisBase(TTree *tree=0);
  virtual ~AnalysisBase();
  //virtual Int_t    Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop()  = 0; //make it pure virtual!
  virtual Bool_t   Notify();
  virtual void     Show(Long64_t entry = -1);
  virtual void     getRange(TH1 *h, float &lo, float& hi, float thresh = 0.25, int nSkipMax=5);
  virtual void getTDCBins(TProfile* h, float& lo, float& hi);
  virtual void getBeamLocation(TH1F *h, float &lo, float& hi);
  virtual double getXOffset(TH1F *h1w);
  virtual void getBeamLoc();
  virtual void getTDC();
  virtual void findBeamRegionAndAlign(int iPass = 1);
  virtual Double_t getCorrChannel(double ch);
  virtual void findChipBoundary();
  virtual void setCrossTalkCorr();
  

  virtual void findDeadRegions();
  virtual void transformTrackToDUTFrame(int k, double& x_trk, double& y_trk, double& nomStrip, double& detStrip);
  virtual void PrepareDUT();
  virtual Double_t getDUTHitPosition(int j);
  virtual void findCutoutRegion(TH2F* h);
  virtual bool isInCutoutRegion(double xtrk, double ytrk);
  virtual double DistToCutoutRegion(double xtrk, double ytrk);
  virtual void correctForStripGaps();

};

#endif
