///////////////////////////////////////////////////
//
// Macro: plotADCs.C
// Author: S. Blusk
// Date: Oct 19, 2015
//
// Purpose: To loop over events and look at the CMS ADC distribution vs channel number
// Can also be run on selected events, liste in a file: "MissingDUTHits.dat", which can
// be obtained by running ClusterAnaWithTrack
// with the flag 
//      writeEventsWithMissinhHitsToFile = true
//
// Note: The cursor needs to be on the event display window to advance to the next event.
// Hit ENTER to go to next event.
//
///////////////////////////////////////////////////

bool plotSelectedEvents = true;
bool showEventDisplay = true;

bool fillStatDistribution = true;  // Fill summary histogram

int lowCh = 127; // Low Channel number of ADC distribution
int hiCh = 256;  // High Channel number of ADC distribution

int evList[20000];
double nomStrip[20000];
double xtrk[20000];
double ytrk[20000];
// Various arrays of parameters common to this macro
double tdcTime = 0;
double adcs[512] = {0};
double finalSignal[512] = {0};

int NCluster = 0;
float ClusterCharge[100] = {0};
int ClusterSize[100] ={0};
float ClusterPosY[100] ={0};
float seedChannel[100] ={0};

int iOffSet = 0;               // Offset into tree, to get to specific events
int printEventDisplays = 0;   // Number of event displays to print to file
float polarity = -1.0;


/////////////////////////////////////////////////////////////////////////////////

void plotADCs(){
  gStyle->SetNdivisions(505,"Y");
  gStyle->SetNdivisions(505,"X");
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);

  //TString sigFile = getFileToOpen(runNum);
  TString sigFile = "/data2/sblusk/TB/July2015/AnaFiles/BoardA6/Run_Bias_Scan-B6-A-212-8358_Tuple.root";

  
  // Open signal file
  TFile *fs = new TFile(sigFile);  
  TTree* cmstree = (TTree*)fs->Get("TbUT/CMS");
  TTree* fChain = (TTree*)fs->Get("TbUT/Clusters");
  int nevents = cmstree->GetEntriesFast();

  // Strip level data
  Double_t        cmsData[512];
  // List of branches
  TBranch        *b_cmsData;   //!
  cmstree->SetBranchAddress("cmsData", cmsData, &b_cmsData);

  // Clusters
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
  
  fChain->SetBranchAddress("clusterNumberPerEvent", &clusterNumberPerEvent, &b_clusterNumberPerEvent);
  fChain->SetBranchAddress("clustersTDC", &clustersTDC, &b_clustersTDC);
  fChain->SetBranchAddress("timestamps", &timestamps, &b_timestamps);
  fChain->SetBranchAddress("clustersPosition", clustersPosition, &b_clustersPosition);
  fChain->SetBranchAddress("clustersSeedPosition", clustersSeedPosition, &b_clustersSeedPosition);
  fChain->SetBranchAddress("clustersCharge", clustersCharge, &b_clustersCharge);
  fChain->SetBranchAddress("clustersSize", clustersSize, &b_clustersSize);
  fChain->SetBranchAddress("clustersSeedCharge", clustersSeedCharge, &b_clustersSeedCharge);
  fChain->SetBranchAddress("clustersCharge2StripLeft", clustersCharge2StripLeft, &b_clustersCharge2StripLeft);
  fChain->SetBranchAddress("clustersCharge1StripLeft", clustersCharge1StripLeft, &b_clustersCharge1StripLeft);
  fChain->SetBranchAddress("clustersCharge1StripRight", clustersCharge1StripRight, &b_clustersCharge1StripRight);
  fChain->SetBranchAddress("clustersCharge2StripRight", clustersCharge2StripRight, &b_clustersCharge2StripRight);
  
  TH1F *h = new TH1F("h","ADC, CM + Step Subtracted",512,0.0,512);  
  TH1F *hm1 = new TH1F("hm1","ADC of nomStrip",110,-100.0,1000);  
  TH1F *hm = new TH1F("hm","ADC of nomStrip#pm1",110,-100.0,1000);
  h->SetTitle("");
  h->SetNdivisions(510,"X");

  TCanvas *theCanvas;
  int nPrint = 0;
  
  char input;
  int nEvents;
  
  int jentry;
  double strip, xp, yp;

  std::ifstream infile;
  if(plotSelectedEvents){
    // Read in event list
    TString filename = "MissingDUTHits.dat";
    cout << "------------------------------------------------------------" << endl;
    cout << " You've selected to view events from file: " << filename << endl;
    cout << "------------------------------------------------------------" << endl;    
    infile.open(filename);
    if(!infile) { // file couldn't be opened
      cerr << "Error: file could not be opened" << endl;
      exit(1);
    }
    int il = 0;   
    while ( !infile.eof() ) { // keep reading until end-of-file
      infile >> jentry >> strip >> xp >> yp;
     if(il>9999) break;
     evList[il] = jentry;
     nomStrip[il] = strip;
     xtrk[il] = xp;
     ytrk[il] = yp;
     il++;     
   }
   infile.close();
   nEvents = nevents - 1000;
  }else{
    cout << "How many events do you want to look at ?" << endl;
    cin >> nEvents;
    cout << "--------------------------------------"<<endl;
  }

  //------------------------  
  // Begin Loop over events
  //------------------------
  il = 0;
  for(int i=iOffSet; i<iOffSet+nEvents; i++){
    //if(i%1==0) cout << "Processing Event " << i << endl;
    if(plotSelectedEvents){
      if(i != evList[il]) continue;
      //if(showEventDisplay){
      //  cout << "-------------------------------------------------------------------------------"<<endl;
      //  cout << "Found selected event: " << i << ", NomStrip of Missing Hit = " << nomStrip[il] << endl;
      //  cout << "-------------------------------------------------------------------------------"<<endl;
      //} 
    }

    cmstree->GetEntry(i);
    fChain->GetEntry(i);
    int k=0;

    //--------------
    // Get CMS data
    //--------------
    for(int j=0; j<512;j++){
      adcs[j] = cmsData[j];
    }

    //------------------
    // Get Cluster data
    //------------------
    NCluster = clusterNumberPerEvent;
    if(NCluster>=10) NCluster=10;
    if(showEventDisplay) cout << "# Clusters = " << NCluster << endl;
    for(int j=0; j<NCluster; j++){
      ClusterSize[j] = clustersSize[j];
      ClusterPosY[j] = 1.0*clustersPosition[j];
      ClusterCharge[j] = clustersCharge[j];
      tdcTime = clustersTDC;
    }


    //-------------------------
    // Get signal distributions
    //-------------------------
    getSignal();      

    int nstrip = 0;
    double vstrip1 = 0, vstrip3 = 0;
    if(plotSelectedEvents){
      nstrip = nomStrip[il]+0.5;
      vstrip1 = finalSignal[nstrip];
      if(nstrip>1) vstrip3 = vstrip1 + finalSignal[nstrip-1];
      if(nstrip<511) vstrip3 = vstrip3 + finalSignal[nstrip+1];
      vstrip1 = -1.0*vstrip1;
      vstrip3 = -1.0*vstrip3;
      
    }
    
    //------------------------------------------------------------
    // Single Event displays of pedestal & noise subtracted data -
    //------------------------------------------------------------
    if(showEventDisplay){
      theCanvas = plotSingleEvent(i, h, nomStrip[il]);  
      cout << "NomStrip = " << nstrip << " .... Charge in nomStrip and nomStrip +- 1: " << vstrip1 << "  " << vstrip3 << endl;
      printf("\n <<< ENTER / RETURN to continue >>> \n");
      theCanvas->WaitPrimitive();
      if(nPrint < printEventDisplays){
        theCanvas->Print(Form("./Plots/EventDisplayCorr_%d.png",i));
        theCanvas->Print(Form("./Plots/EventDisplayCorr_%d.pdf",i)); 
        nPrint++;
      }
    }
    
    //----------------
    // Fill histograms
    //----------------
    if(fillStatDistribution && plotSelectedEvents){
      hm1->Fill(-1.0*vstrip1);
      hm->Fill(-1.0*vstrip3);
    }
    
    if(plotSelectedEvents) il++;    
  }

  
  // Plot statistical distributions
  TCanvas *c = new TCanvas("c","Plots",500,800);
  c->Divide(1,2);
  c->cd(1);  
  hm1->Draw();
  c->cd(2);  
  hm->Draw();
}

void getSignal(){
  // -----------------
  // -  Final result -
  //    - Here, one could make any "corrections", or flip polarity, etc
  // -----------------
  for(int k=0;k<256;k++){finalSignal[k] = adcs[k];}  
  return;
}


TCanvas* plotSingleEvent(int i, TH1F *h, double xstrip)
{
  //-----------------
  // Final result
  //-----------------
  double low = 1000;
  double hi = -1000;

  h->Reset();  
  for(int k=0;k<512;k++) {
    h->Fill(k,finalSignal[k]);
    if(finalSignal[k]<low) low = finalSignal[k];
    if(finalSignal[k]>hi) hi = finalSignal[k];
  }
  
  h->SetMinimum(low-100);
  h->SetMaximum(hi+300);
  h->GetXaxis()->SetRangeUser(lowCh,hiCh);

  TCanvas *c = new TCanvas("c","",500,0,800,800);
  h->Draw();
  TLine *l = new TLine(lowCh, 0, hiCh, 0);
  TLine *l2 = new TLine(xstrip, low-100, xstrip, hi+150);
  l2->SetLineColor(5);   l2->SetLineWidth(4); l2->Draw();
  l->SetLineColor(3); l->SetLineStyle(2); l->SetLineWidth(2); l->Draw();  
  h->Draw("same");
  

  TLatex *myLatex = new TLatex();
  myLatex->SetTextFont(42); myLatex->SetTextColor(1); 
  myLatex->SetTextAlign(12); myLatex->SetNDC(kTRUE); myLatex->SetTextSize(0.065);
  myLatex->DrawLatex(0.14,0.85,Form("Event %d",i));
  TString tdcs = Form("TDC time = %4.1f ns",tdcTime);
  myLatex->SetTextSize(0.045);myLatex->DrawLatex(0.18,0.79,tdcs);  


  float yval = 0.79;
  myLatex->SetTextSize(0.035);
  yval = yval - 0.05;
  for(int k=0;k<NCluster;k++){
    TString clString = Form("Size=%d,  Q=%3.0f,  Pos=%4.1f",ClusterSize[k],ClusterCharge[k],ClusterPosY[k]);
    myLatex->DrawLatex(0.14,yval,clString);  
    yval = yval - 0.05;
  }

  return c;
}


