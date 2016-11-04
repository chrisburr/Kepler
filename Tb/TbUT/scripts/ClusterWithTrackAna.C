#define ClusterWithTrackAna_cxx
//#include "ClusterWithTrackAna_Inputs.h"
#include "ClusterWithTrackAna.h"
#include <TH2.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>

void addGraphics(TH1 *h, int iCol = 1, TString XTitle="", TString YTitle="")
{
  h->SetXTitle(XTitle);
  h->SetYTitle(YTitle);
  h->SetStats(kFALSE);
  h->SetLineColor(iCol);
  h->SetMarkerColor(iCol);
  h->SetMinimum(0.0);
  h->SetMaximum(1.2*h->GetMaximum());
  h->SetTitleSize(0.1);
  
  //h->SetLineColor(kBlack);
  h->SetMarkerSize(0.7);
  h->SetMarkerStyle(20);
  h->GetXaxis()->SetTitleOffset(1.0);  
  h->GetYaxis()->SetTitleOffset(1.2);
  h->GetXaxis()->SetTitleSize(0.045);  
  h->GetYaxis()->SetTitleSize(0.045);
  h->GetXaxis()->SetLabelSize(0.04);  
  h->GetYaxis()->SetLabelSize(0.04);  
  h->SetNdivisions(505,"X");
  h->SetNdivisions(505,"Y");
  h->SetLineWidth(2);
}

void addGraphics(TH2 *h, int iCol = 1, TString XTitle="", TString YTitle="")
{
  //float bw = h->GetBinWidth(1);
  h->SetXTitle(XTitle);
  h->SetYTitle(YTitle);
  h->SetStats(kFALSE);
  h->SetLineColor(iCol);
  h->SetMarkerColor(iCol);
  h->SetMinimum(0.0);
  h->SetMaximum(1.2*h->GetMaximum());
  h->SetTitleSize(0.1);
  
  //h->SetLineColor(kBlack);
  //h->SetMarkerSize(0.7);
  //h->SetMarkerStyle(20);
  h->GetXaxis()->SetTitleOffset(1.0);  
  h->GetYaxis()->SetTitleOffset(1.2);
  h->GetXaxis()->SetTitleSize(0.045);  
  h->GetYaxis()->SetTitleSize(0.045);
  h->GetXaxis()->SetLabelSize(0.04);  
  h->GetYaxis()->SetLabelSize(0.04);  
  h->SetNdivisions(505,"X");
  h->SetNdivisions(505,"Y");
  h->SetLineWidth(2);
}


void ClusterWithTrackAna::Loop()
{
//   In a ROOT session, you can do:
//      Root > .L ClusterWithTrackAna.C
//      Root > ClusterWithTrackAna t
//      Root > t.GetEntry(12); // Fill t data members with entry number 12
//      Root > t.Show();       // Show values of entry 12
//      Root > t.Show(16);     // Read and show values of entry 16
//      Root > t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   Int_t nentries = fChain->GetEntriesFast();

   TString m_board2 = m_board;
   m_board2 = m_board2.ReplaceAll("_All","");
   m_board2 = m_board2.ReplaceAll("_Full","");
   m_board2 = m_board2.ReplaceAll("_v7","");
   TString f_out = m_fileOutdir + plotdir + "/AnalysisOutput_" + m_board2 + "_" + m_bias + "_" + m_sector + ".root";
   if(m_angle != "0"){
     f_out			= m_fileOutdir + plotdir + "/AnalysisOutput_" + m_board2 + "_" + m_bias + "_" + m_sector + "_" + m_angle + ".root";
   }
   cout << "Will write out file: " << f_out << endl;
   


   fout = new TFile(f_out,"RECREATE");

   TH1F* h0 = new TH1F("h0","#DeltaX between strip hit and track projection (strips)",101,-50.5,50.5);
   TH1F* h1 = new TH1F("h1","#DeltaX",800,-2.0,2.0);
   TH2F* h1vsx = new TH2F("h1vsx","#DeltaX vs X",50,-5,5,100,-0.2,0.2);
   TH1F* h1mpa = new TH1F("h1mpa","#DeltaX",400,-20.0,20.0);
   TH1F* h1mbpa = new TH1F("h1mbpa","#DeltaX",400,-20.0,20.0);
   TH1F* h1fpa = new TH1F("h1fpa","#DeltaX",400,-20.0,20.0);
   TH1F* h1fbpa = new TH1F("h1fbpa","#DeltaX",400,-20.0,20.0);

   TH1F* h1mpa1 = new TH1F("h1mpa1","#DeltaX",800,-20.0,20.0);
   TH1F* h1mpa2 = new TH1F("h1mpa2","#DeltaX",800,-20.0,20.0);
   TH1F* h1mpa3 = new TH1F("h1mpa3","#DeltaX",800,-20.0,20.0);
   TH1F* h1mpa4 = new TH1F("h1mpa4","#DeltaX",800,-20.0,20.0);
   TH1F* h1mpa5 = new TH1F("h1mpa5","#DeltaX",800,-20.0,20.0);

   TH1F* h1mpaL = new TH1F("h1mpaL","Strip# of Missed Hit in Lower PA region",512,0,512);
   TH1F* h1mpaU = new TH1F("h1mpaU","Strip# of Missed Hit in Upper PA region",512,0,512);


   TH1F* h1s = new TH1F("h1s","Seed strip of cluster",512,0.0,512.0);
   TH1F* h1a = new TH1F("h1a","#DeltaX, 1 strip",800,-2.0,2.0);
   TH1F* h1b = new TH1F("h1b","#DeltaX, 2 strip",800,-2.0,2.0);
   TH1F* h1z = new TH1F("h1z","#DeltaX, low Charge",800,-2.0,2.0);
   TH1F* h1w = new TH1F("h1w","#DeltaX",20000,-100.0,100.0);
   TH1F* h1wY = new TH1F("h1wY","#DeltaX",20000,-100.0,100.0);
   TH2F* h2 = new TH2F("h2","X_{DUT} vs X_{trk}",800,-8.0,8.0,800,-8,8);
   TH2F* h3 = new TH2F("h3","Y_{trk} vs X_{trk}, with cluster",640,-8,8.0,640,-8,8);
   TH2F* h3a = new TH2F("h3a","Y_{trk} vs X_{trk} in fiducial",640,-8,8.0,640,-8,8);
   TH2F* h3b = new TH2F("h3b","Y_{trk} vs X_{trk}, with missed cluster",640,-8,8.0,640,-8,8);
   TH2F* h3c = new TH2F("h3c","Y_{trk} vs X_{trk}, with found cluster",640,-8,8.0,640,-8,8);
   TH2F* h3d = new TH2F("h3d","Y_{trk} vs X_{trk}, with found cluster",640,-8,8.0,640,-8,8);
   TH1F* h4 = new TH1F("h4","Detector strip # of cluster with track",512,0.0,512);
   TH1F* h4a = new TH1F("h4a","Electonic Strip # of cluster with track",512,0.0,512);
   TH1F* h4b = new TH1F("h4b","Strip # of cluster with track",1024,0.0,1024);
   TH1F* h4c = new TH1F("h4c","Strip # of cluster with track",1024,0.0,1024);
   TH1F* h5 = new TH1F("h5","#theta_{X}",500,-5.0,5.0);
   TH1F* h6 = new TH1F("h6","#theta_{Y}",500,-5.0,5.0);
   TH1F* h5c = new TH1F("h5c","#theta_{X}",500,-5.0,5.0);
   TH1F* h6c = new TH1F("h6c","#theta_{Y}",500,-5.0,5.0);
   TH1F* h5a = new TH1F("h5a","X position of track",400,-10.0,10.0);
   TH1F* h6a = new TH1F("h6a","Y position of track",400,-10.0,10.0);
   TH1F* h5b = new TH1F("h5b","X position of matched cluster",400,-10.0,10.0);
   TH1F* h6b = new TH1F("h6b","Y position of matched cluster",400,-10.0,10.0);

   TProfile *h8 = new TProfile("h8","#DeltaX vs #theta_{trk}",100,-5,5,-1.0,1.0);
   TProfile *h9 = new TProfile("h9","#DeltaX vs Y_{trk} at DUT",1600,-8,8,-1.0,1.0);
   TProfile *h9a = new TProfile("h9a","#DeltaX vs X_{trk} at DUT",20,-5,5,-1.0,1.0);

   TProfile *h10a = new TProfile("h10a","<ADC> vs strip",512,0,512,0.0,1000.0);
   TProfile *h10b = new TProfile("h10b","<ADC> vs strip",512,0,512,0.0,1000.0);
   TProfile *h10c = new TProfile("h10c","<ADC> vs strip",512,0,512,0.0,1000.0);
   TProfile *h10d = new TProfile("h10d","<ADC> vs Y_{trk}",400,-8,8,0.0,1000.0);
   TProfile *h10e = new TProfile("h10e","<ADC> vs X_{trk}",100,-8,8,0.0,1000.0);
   TH1F* h11n = new TH1F("h11n","Strip # of matched cluster",512,0.0,512);
   TH1F* h11d = new TH1F("h11d","Strip # of track",512,0.0,512);
   h11n->Sumw2();
   h11d->Sumw2();
   
   TH1F* h12 = new TH1F("h12","Y position of matched cluster",400,-10.0,10.0); h12->Sumw2();
   TH1F* h12c = new TH1F("h12c","X position of matched cluster",400,-10.0,10.0);h12c->Sumw2();
   TH1F* h12a = new TH1F("h12a","Y position of track",1600,-10.0,10.0); h12a->Sumw2();
   TH1F* h12b = new TH1F("h12b","Y position of track",1600,-10.0,10.0); h12b->Sumw2();

   TH1F* h12dn = new TH1F("h12dn","X position of track",200,-10.0,10.0); h12dn->Sumw2();
   TH1F* h12en = new TH1F("h12en","X position of track",200,-10.0,10.0);h12en->Sumw2();
   TH1F* h12fn = new TH1F("h12fn","X position of track",200,-10.0,10.0);h12fn->Sumw2();
   TH1F* h12gn = new TH1F("h12gn","X position of track",200,-10.0,10.0);h12gn->Sumw2();
   TH1F* h12dd = new TH1F("h12dd","X position of track",200,-10.0,10.0);h12dd->Sumw2();
   TH1F* h12ed = new TH1F("h12ed","X position of track",200,-10.0,10.0);h12ed->Sumw2();
   TH1F* h12fd = new TH1F("h12fd","X position of track",200,-10.0,10.0);h12fd->Sumw2();
   TH1F* h12gd = new TH1F("h12gd","X position of track",200,-10.0,10.0);h12gd->Sumw2();

   TH1F* h12hn = new TH1F("h12hn","X position of track",50,-0.5,0.5);h12hn->Sumw2();
   TH1F* h12in = new TH1F("h12in","X position of track",50,-0.5,0.5);h12in->Sumw2();
   TH1F* h12jn = new TH1F("h12jn","X position of track",50,-0.5,0.5);h12jn->Sumw2();
   TH1F* h12kn = new TH1F("h12kn","X position of track",50,-0.5,0.5);h12kn->Sumw2();
   TH1F* h12hd = new TH1F("h12hd","X position of track",50,-0.5,0.5);h12hd->Sumw2();
   TH1F* h12id = new TH1F("h12id","X position of track",50,-0.5,0.5);h12id->Sumw2();
   TH1F* h12jd = new TH1F("h12jd","X position of track",50,-0.5,0.5);h12jd->Sumw2();
   TH1F* h12kd = new TH1F("h12kd","X position of track",50,-0.5,0.5);h12kd->Sumw2();

   TProfile *h12m = new TProfile("h12m","<ADC> vs interstrip pos",50,-0.5,0.5,0.0,1000.0);
   TProfile *h12n = new TProfile("h12n","<ClusterSize> vs interstrip pos",50,-0.5,0.5,0.0,1000.0);

   TH1F* h12on = new TH1F("h12on","dist of track to cutout",700,-2.0,5.0);h12on->Sumw2();
   TH1F* h12od = new TH1F("h12od","dist of track to cutout",700,-2.0,5.0);h12od->Sumw2();

   TH1F *h13mpa = new TH1F("h13mpa","#Clusters/Ev",10,-0.5,9.5);
   TH1F *h13mbpa = new TH1F("h13mbpa","#Clusters/Ev",10,-0.5,9.5);
   TH1F *h13fpa = new TH1F("h13fpa","#Clusters/Ev",10,-0.5,9.5);
   TH1F *h13fbpa = new TH1F("h13fbpa","#Clusters/Ev",10,-0.5,9.5);   

   TH1F* h14a = new TH1F("h14a","Nom Strip, no matched cluster",512,0.0,512);
   TH1F* h14b = new TH1F("h14b","Nom Strip, matched cluster",512,0.0,512);

   TH1F *h15a = new TH1F("h15a","Chisq of tracks with matched DUT hit",200,0.0,200.0);
   TH1F *h15b = new TH1F("h15b","TDC time with matched DUT hit",12,0.0,12.0);
   TH1F *h15c = new TH1F("h15c","Time Between Trigger and Track",5,0.0,5.0);
   TH1F *h15d = new TH1F("h15d","TDC time of all clusters",12,0.0,12.0);
   TH1F *h15e = new TH1F("h15e","TDC time of good clusters",12,0.0,12.0);

   TH1F *h16a = new TH1F("h16a","Chisq of tracks without matched DUT hit",200,0.0,200.0);
   TH1F *h16b = new TH1F("h16b","TDC time without matched DUT hit",12,0.0,12.0);
   TH1F *h16c = new TH1F("h16c","Time Between Trigger and Track",5,0.0,5.0);
   h15c->Sumw2();
   h16c->Sumw2();
   
   TH1F* h17 = new TH1F("h17","Q_{L} / (Q_{L}+Q_{R})",50,0.0,1.0);
   TH2F* h17a = new TH2F("h17a","Q_{L} / (Q_{L}+Q_{R}) vs Interstrip Pos",20,-0.5,0.5,120,-0.1,1.1);
   TProfile* h17b = new TProfile("h17b","Q_{R} / (Q_{L}+Q_{R}) vs Interstrip Pos",20,-0.5,0.5,-0.1,1.1);

   TH1F* hcmpa = new TH1F("hcmpa","Cluster charge",100,0.0,1000.0);
   TH1F* hcmbpa = new TH1F("hcmbpa","Cluster charge",100,0.0,1000.0);
   TH1F* hcfpa = new TH1F("hcfpa","Cluster charge",100,0.0,1000.0);
   TH1F* hcfbpa = new TH1F("hcfbpa","Cluster charge",100,0.0,1000.0);

   TH1F* h18a = new TH1F("h18a","Seed/Total charge",100,0.0,2.0);

   TH2F* h18b = new TH2F("h18b","#DeltaX vs Interstrip Pos (1 strip Clu)",100,-0.5,0.5,100,-0.2,0.2);
   TH2F* h18c = new TH2F("h18c","#DeltaX vs Interstrip Pos (2 strip Clu)",100,-0.5,0.5,100,-0.2,0.2);
   TH2F* h18d = new TH2F("h18d","Seed/Total charge vs Interstrip Pos",100,-0.5,0.5,100,0.0,1.1);

   TH1F* hcTrkXY[16];
   for(int i=0; i<4; i++){
     for(int j=0; j<4; j++){
       int iblock = 4*i+j; 
       TString hname = Form("hcTrkXY_%d",iblock+1);     
       TString htitle = Form("Cluster charge, block %d",iblock+1);     
       hcTrkXY[iblock]= new TH1F(hname,htitle,100,0.0,1000.0);
     }
   }
   
   TH1F *hlandau[512];
   for(int i=0; i<nChan; i++){
     hlandau[i] = new TH1F(Form("hlandau_%d",i),"Cluster charge",100,0.0,1000.0);
   }
   


   TH1F* hcAll = new TH1F("hcAll","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrk = new TH1F("hcTrk","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrkCorr = new TH1F("hcTrkCorr","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrkSeed = new TH1F("hcTrkSeed","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrkSeed1 = new TH1F("hcTrkSeed1","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrkSeed2 = new TH1F("hcTrkSeed2","Cluster charge",100,0.0,1000.0);
   //TH1F* hcTrkY1 = new TH1F("hcTrkY1","Cluster charge",100,0.0,1000.0);
   //TH1F* hcTrkY2 = new TH1F("hcTrkY2","Cluster charge",100,0.0,1000.0);
   //TH1F* hcTrkY3 = new TH1F("hcTrkY3","Cluster charge",100,0.0,1000.0);
   //TH1F* hcTrkY4 = new TH1F("hcTrkY4","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrkNoTop = new TH1F("hcTrkNoTop","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrkLowX = new TH1F("hcTrkLowX","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrkHighX = new TH1F("hcTrkHighX","Cluster charge",100,0.0,1000.0);
   TH1F* hcTrka = new TH1F("hcTrka","Cluster charge, limited region",100,0.0,1000.0);
   TH1F* hcTrk1 = new TH1F("hcTrk1","Cluster charge, Size = 1",100,0.0,1000.0);
   TH1F* hcTrk2 = new TH1F("hcTrk2","Cluster charge, Size = 2",100,0.0,1000.0);
   TProfile *h2p = new TProfile("h2p","Cluster Charge vs TDC time",12,0,12,100,1000);


   TH2F* h31a = new TH2F("h31a","Y_{trk} vs X_{trk}, with cluster",64,-8,8.0,64,-8,8);
   TH2F* h31b = new TH2F("h31b","Y_{trk} vs X_{trk}, with cluster, low ADC",64,-8,8.0,64,-8,8);
   TH1F* h32a = new TH1F("h32a","Strip # of cluster with track",512,0.0,512);
   TH1F* h32b = new TH1F("h32b","Strip # of cluster with track & low ADC",512,0.0,512);
   TH1F* h33a = new TH1F("h33a","Y position of matched cluster",200,-10.0,10.0);
   TH1F* h33b = new TH1F("h33b","X position of matched cluster",200,-10.0,10.0);
   TH1F* h34 = new TH1F("h34","Y position of matched cluster & low ADC",200,-10.0,10.0);

   TH1F *hnoise = new TH1F("hnoise","Noise in connected channels",100,-200,200);
   TH1F *hnoiseChan = new TH1F("hnoiseChan","Noise in connected channels",200,0,200);
   TH1F *hnoisePerChannel = new TH1F("hnoisePerChannel","Noise",512,0,512);

   TH1F* h35 = new TH1F("h35","No. clusters / event",50,0.0,50.0);
   
   TH1F *h41[10];
   TH1F *h42[10];
   TH1F *h43[10];
   TH1F *h44[10];
   for(int i=0; i<10; i++){
     h41[i] = new TH1F(Form("h41_%d",i),Form("ADC_{L}-ADC_{R}, Bin %d, Odd Ch",i),80,-400,400);
     h42[i] = new TH1F(Form("h42_%d",i),Form("ADC_{L}-ADC_{R}, Bin %d, Odd Ch",i),80,-400,400);
     h43[i] = new TH1F(Form("h43_%d",i),Form("ADC_{L}-ADC_{R}, Bin %d, Odd Ch",i),160,-400,400);
     h44[i] = new TH1F(Form("h44_%d",i),Form("ADC_{L}-ADC_{R}, Bin %d, Odd Ch",i),160,-400,400);
   }

   for(int i=0; i<nChan; i++){
     hnoisePerChannel->Fill(i+0.5,noise[i]);
   }

   TH1F* h51 = new TH1F("h51","Y_{trk}",5000,-50.0,50.0);   
   TH1F* h52 = new TH1F("h52","Y_{trk}",5000,-50.0,50.0);   


   //------------------------------------------------------------------------------------------------------
   
   //-----------------------------
   // Prepare DUT (Alignment, etc)
   //-----------------------------

   PrepareDUT();

   float biasVal = atof(m_bias);
   cout << "chargeCorrSlopeOdd, chargeCorrSlopeEven = " << chargeCorrSlopeOdd << " " << chargeCorrSlopeEven << " " 
        << m_bias << " " << biasVal << endl;
   

   std::ofstream myfile;
   if(writeEventsWithMissinhHitsToFile){
     myfile.open("MissingDUTHits.dat");
   }

   int iChan = nChan;
   double nomStrip = 0, detStrip = 0;
   int nPrint = 0;
   double dxh[10];
   int njump = 10000;
   if(nentries > 100000) njump = 50000;
   Long64_t nbytes = 0, nb = 0;
   cout << "Begin loop over " << nentries << " events" << endl;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      if(jentry%njump==0) cout << "====> At entry = " << jentry << endl;
      if(n_tp3_tracks != 1) continue;

      //==========================================
      // Example here of acccessing CMS strip data
      //==========================================
      if(fCMS){
        for(int k=0;k<iChan;k++){
          if(fCMS->cmsData[k]!=0) hnoise->Fill(fCMS->cmsData[k]);
        }      
      }

      h35->Fill(clusterNumberPerEvent);
      if(clusterNumberPerEvent>10) continue;
      
      
      // Loop over TPIX tracks in event
      for(int k=0; k<n_tp3_tracks; k++){
        if(dtime > trackTriggerTimeDiffCut) continue;
        double x_trk = vec_trk_tx->at(k)*z_DUT+vec_trk_x->at(k);
        double y_trk = vec_trk_ty->at(k)*z_DUT+vec_trk_y->at(k);
        
        transformTrackToDUTFrame(k, x_trk, y_trk, nomStrip, detStrip);
        if(x_trk>xMin-1.5 && x_trk<xMax+1.5 && y_trk>yMin-1.5 && y_trk<yMax+1.5) h3d->Fill(x_trk,y_trk);
        int idetStrip = detStrip;
        bool foundBadStrip = false;
        for(int ib=0; ib<nbadStrips; ib++){
          if(badStrips[idetStrip] > 0) {foundBadStrip = true; break;}
        }
        if(foundBadStrip) {
          //cout << "Found bad strip - skipping: " << idetStrip << endl;
          continue;
        }
        

        //if(isInCutoutRegion(x_trk, y_trk)) continue;
        double distToCutout = DistToCutoutRegion(x_trk, y_trk);
        bool awayFromCutout = distToCutout > minDistFromHole;

        int closestStrip = nomStrip;
        double fracStrip = nomStrip - closestStrip - 0.5;

        double tx = 1000*vec_trk_tx->at(k);
        double ty = 1000*vec_trk_ty->at(k);

        h5->Fill(tx);
        h6->Fill(ty);

        // Test cuts here, e.g.  to reject regions
        //if(nomStrip<422 || nomStrip>442) continue;
        //if(y_trk>2.4) continue;
        //if(nomStrip<420 || nomStrip>445) continue;
        

        bool goodRegion = true;
        for(int id = 0; id<nDeadRegion; id++){
          if(x_trk>=deadRegionLo[id]  && x_trk<=deadRegionHi[id]) goodRegion = false;  
        }
        if(!goodRegion) continue;

        h5a->Fill(x_trk);
        h6a->Fill(y_trk);
        bool goodTrack = false;
        bool inFiducialX = false;
        bool inFiducialY = false;

        if(x_trk>xMin && x_trk<xMax) inFiducialX = true;          
        if(y_trk>yMin && y_trk<yMax) inFiducialY = true;          
        bool inFiducial = inFiducialX && inFiducialY;
        inFiducial = inFiducial && (x_trk<xLeftHole || x_trk>xRightHole);
        
        if(tx>txMin && tx<txMax && ty>tyMin && ty<tyMax) goodTrack = true;        
        bool goodTime =  (clustersTDC >= tdcLo && clustersTDC <= tdcHi);

        if(goodTrack && goodTime && inFiducial) h12od->Fill(distToCutout);

        if(goodTrack && goodTime && inFiducial && awayFromCutout) {
          h3a->Fill(x_trk,y_trk);
          h12a->Fill(y_trk);
          h12dd->Fill(x_trk);
          if(y_trk>yInt1[0]&&y_trk<yInt1[1]) h12ed->Fill(x_trk);
          if(y_trk>yInt2[0]&&y_trk<yInt2[1]) h12fd->Fill(x_trk);
          if(y_trk>yInt3[0]&&y_trk<yInt3[1]) h12gd->Fill(x_trk);
          h12hd->Fill(fracStrip);
          if(y_trk>yInt1[0]&&y_trk<yInt1[1] && x_trk>-3.5&&x_trk<2.5) h12id->Fill(fracStrip);
          if(y_trk>yInt2[0]&&y_trk<yInt2[1] && x_trk>-3.5&&x_trk<2.5) h12jd->Fill(fracStrip);
          if(y_trk>yInt3[0]&&y_trk<yInt3[1] && x_trk>-3.5&&x_trk<2.5) h12kd->Fill(fracStrip);
        }
        
        bool foundHit = false;
        bool foundHitNoFid = false;
        double x_trk0 = x_trk;
        for(int j=0; j<min(clusterNumberPerEvent,10); j++){
          dxh[j] = -999;
          if(clustersPosition[j] < 0.1) continue;
          if(polarity*clustersCharge[j] < 0.2*kClusterChargeMin) continue;
          double x_dut = getDUTHitPosition(j);
          x_trk = x_trk0;
       
          int iPeak = 1;
          if(clustersSeedPosition[j]%2==0) iPeak = 0;
          
          h1s->Fill(clustersSeedPosition[j]);
          if(clustersSeedPosition[j]<iLo-5 || clustersSeedPosition[j]>iHi+5) continue;

          double dx = x_dut - x_trk;
          dxh[j] = dx;

          if(inFiducial && goodTime && awayFromCutout) h12->Fill(y_trk); 
          if(goodTrack && inFiducial && fabs(dx)<dxWin && awayFromCutout) {
            h2p->Fill(clustersTDC+0.1,polarity*clustersCharge[j]);
            h15d->Fill(clustersTDC);          
            if(goodTime) h15e->Fill(clustersTDC);          
          }
          

          if(goodTrack && inFiducial && goodTime && fabs(dx)<dxWin){
            h3->Fill(x_trk,y_trk);
          }
          
          if(goodTrack && inFiducial && goodTime && fabs(dx)<dxWin) foundHitNoFid = true;

          h51->Fill(y_trk - yGloOff);
          if(fabs(dx)<dxWin) h52->Fill(y_trk - yGloOff);

          if(goodTrack && inFiducial && goodTime && awayFromCutout) {
            hcAll->Fill(polarity*clustersCharge[j]);
            h2->Fill(x_trk, x_dut);
            h1->Fill(dx);
            if(clustersSize[j]==1) h1a->Fill(dx);
            if(clustersSize[j]==2) h1b->Fill(dx);
            h1w->Fill(dx);
            if(y_trk>2.5) h1wY->Fill(dx);
            if(polarity*clustersCharge[j] < 250) h1z->Fill(dx);
            h11d->Fill(detStrip);        
            
            if(fabs(dx)<dxWin) {
              int ichan = clustersSeedPosition[j];
              h4c->Fill(clustersSeedPosition[j]);
              if(ichan>=0 && ichan<=511){
                hlandau[ichan]->Fill(polarity*clustersCharge[j]);
              }
              
              hnoiseChan->Fill(noise[ichan]);
              h18a->Fill(clustersSeedCharge[j]/clustersCharge[j]);
              foundHit = true;
              if(clustersSize[j]==1) h18b->Fill(fracStrip,dx);
              if(clustersSize[j]==2) h18c->Fill(fracStrip,dx);
              h18d->Fill(fracStrip,clustersSeedCharge[j]/clustersCharge[j]);
              
              h12m->Fill(fracStrip,polarity*clustersCharge[j]);
              h12n->Fill(fracStrip,clustersSize[j]);
              h1vsx->Fill(x_trk,dx);
              if(y_trk>yMid&&y_trk<yMax) h10a->Fill(clustersPosition[j],polarity*clustersCharge[j]);
              if(y_trk>yMin&&y_trk<yMid) h10b->Fill(clustersPosition[j],polarity*clustersCharge[j]);
              if(y_trk>yHi2&&y_trk<yMax) h10c->Fill(clustersPosition[j],polarity*clustersCharge[j]);
              h10d->Fill(y_trk,polarity*clustersCharge[j]);
              h10e->Fill(x_trk,polarity*clustersCharge[j]);
              double chleft = polarity*clustersCharge1StripLeft[j];
              double chright = polarity*clustersCharge1StripRight[j];
              double rc = -999;
              if(detStrip <= clustersSeedPosition[j]) {
                if(chleft>0) rc = (chleft/(chleft+clustersSeedCharge[j]));              
              }else{ 
                if(chright>0) rc = (clustersSeedCharge[j]/(chright+clustersSeedCharge[j]));
              }
              if( rc>=0 ) {
                h17->Fill(rc);
                h17a->Fill(fracStrip,rc);
                h17b->Fill(fracStrip,rc);
              }
              h8->Fill(1000*vec_trk_tx->at(k),dx);    
              h9->Fill(y_trk,dx);
              h9a->Fill(x_trk,dx);
              h5b->Fill(x_trk);
              h6b->Fill(y_trk);
              h5c->Fill(tx);
              h6c->Fill(ty);

              h11n->Fill(detStrip);
              hcTrk->Fill(polarity*clustersCharge[j]);
              if(clustersSize[j]==1) hcTrkCorr->Fill(polarity*clustersCharge[j]);
              if(clustersSize[j]==2 && iPeak==1) hcTrkCorr->Fill(polarity*clustersCharge[j]*(1.0-chargeCorrSlopeOdd));
              if(clustersSize[j]==2 && iPeak==0) hcTrkCorr->Fill(polarity*clustersCharge[j]*(1.0-chargeCorrSlopeEven));
              hcTrkSeed->Fill(polarity*clustersSeedCharge[j]);
              if(clustersSize[j]==1) hcTrkSeed1->Fill(polarity*clustersSeedCharge[j]);
              if(clustersSize[j]==2) hcTrkSeed2->Fill(polarity*clustersSeedCharge[j]);
              if(y_trk - yMax < 5.0) hcTrkNoTop->Fill(polarity*clustersCharge[j]);
              if(x_trk < (xMin+xMax)/2.) hcTrkLowX->Fill(polarity*clustersCharge[j]);
              if(x_trk >= (xMin+xMax)/2.) hcTrkHighX->Fill(polarity*clustersCharge[j]);
              int ix = 4*(x_trk - xMin - 0.001) / (xMax-xMin);
              int iy = 4*(y_trk - yMin - 0.001) / (yMax-yMin);
              int ihist = 4*ix + iy;
              hcTrkXY[ihist]->Fill(polarity*clustersCharge[j]);              
              //if(y_trk>yMin && y_trk<=(yMin+0.25*yRange)) hcTrkY1->Fill(polarity*clustersCharge[j]);
              //if(y_trk>(yMin+0.25*yRange) && y_trk<(yMin+0.50*yRange)) hcTrkY2->Fill(polarity*clustersCharge[j]);
              //if(y_trk>(yMin+0.50*yRange) && y_trk<(yMin+0.75*yRange)) hcTrkY3->Fill(polarity*clustersCharge[j]);
              //if(y_trk>(yMin+0.75*yRange) && y_trk<(yMin+1.00*yRange)) hcTrkY4->Fill(polarity*clustersCharge[j]);
              //if(y_trk>yMin&&y_trk<yMid+1.0) hcTrkY2->Fill(polarity*clustersCharge[j]);

              if(clustersSize[j]==1) hcTrk1->Fill(polarity*clustersCharge[j]);
              if(clustersSize[j]==2) hcTrk2->Fill(polarity*clustersCharge[j]);
              if(clustersPosition[j]>170  && clustersPosition[j]<190) hcTrka->Fill(polarity*clustersCharge[j]);
              h31a->Fill(x_trk,y_trk);
              h32a->Fill(clustersPosition[j]);
              h33a->Fill(y_trk);
              h33b->Fill(x_trk);
              if(polarity*clustersCharge[j] < 250) {
                h31b->Fill(x_trk,y_trk);
                h32b->Fill(clustersPosition[j]);
                h34->Fill(y_trk);
              }
              double chr = clustersCharge1StripRight[j]*polarity;
              double chl = clustersCharge1StripLeft[j]*polarity;
              double chr2 = clustersCharge2StripRight[j]*polarity;
              double chl2 = clustersCharge2StripLeft[j]*polarity;
              double pch = polarity*clustersSeedCharge[j];
              int ic = pch/50.;
              if(ic>=0 && ic<10 && clustersSize[j]<=2 ){
                if(iPeak==1) h41[ic]->Fill(chl-chr);
                if(iPeak==0) h42[ic]->Fill(chl-chr);
                if(chr2!=0 and chl2!=0){
                  if(iPeak==1) h43[ic]->Fill(chl2-chr2);
                  if(iPeak==0) h44[ic]->Fill(chl2-chr2);
                }
                
              } 
            }

            double clstrip = getCorrChannel(clustersPosition[j]);
            h4->Fill(clstrip);
            h4b->Fill(clstrip + channelOffset);
            h4a->Fill(clustersPosition[j]);
            h0->Fill(detStrip - clstrip);
          }
        }

        if(inFiducial && goodTrack && goodTime && foundHitNoFid) {
          h12on->Fill(distToCutout);
        }
        
        if(inFiducial && goodTrack && goodTime && awayFromCutout) {
          h16c->Fill(dtime);
          if(foundHit) {
            h3c->Fill(x_trk,y_trk);
            for(int j=0; j<min(clusterNumberPerEvent,10); j++){
              hcfpa->Fill(polarity*clustersCharge[j]);
              if(polarity*clustersCharge[j]>120) h1fpa->Fill(dxh[j]);
            }        
            h15c->Fill(dtime);
            h15b->Fill(clustersTDC+0.1);
            h15a->Fill(vec_trk_chi2ndf->at(k));
            h12b->Fill(y_trk);
            h12dn->Fill(x_trk); 
            if(y_trk>yInt1[0]&&y_trk<yInt1[1]) h12en->Fill(x_trk);
            if(y_trk>yInt2[0]&&y_trk<yInt2[1]) h12fn->Fill(x_trk);
            if(y_trk>yInt3[0]&&y_trk<yInt3[1]) h12gn->Fill(x_trk);
            h12hn->Fill(fracStrip);
            if(y_trk>yInt1[0]&&y_trk<yInt1[1] && x_trk>-3.5&&x_trk<2.5) h12in->Fill(fracStrip);
            if(y_trk>yInt2[0]&&y_trk<yInt2[1] && x_trk>-3.5&&x_trk<2.5) h12jn->Fill(fracStrip);
            if(y_trk>yInt3[0]&&y_trk<yInt3[1] && x_trk>-3.5&&x_trk<2.5) h12kn->Fill(fracStrip);
            if(y_trk>yInt1[0] && y_trk < yInt2[1] && x_trk>-3.5&&x_trk<-2.0) {
              h14b->Fill(detStrip);
              h13fpa->Fill(clusterNumberPerEvent);
            }else if(y_trk>yInt3[0] && y_trk < yInt3[1]) {
              h13fbpa->Fill(clusterNumberPerEvent);
              for(int j=0; j<min(clusterNumberPerEvent,10); j++){
                h1fbpa->Fill(dxh[j]);
                hcfbpa->Fill(polarity*clustersCharge[j]);
              }
            }                
          }else{     
            h3b->Fill(x_trk,y_trk);          
            h13mpa->Fill(clusterNumberPerEvent);
            h16a->Fill(vec_trk_chi2ndf->at(k));
            h16b->Fill(clustersTDC+0.1);
            for(int j=0; j<min(clusterNumberPerEvent,10); j++){
              hcmpa->Fill(polarity*clustersCharge[j]);
              if(polarity*clustersCharge[j]>120) h1mpa->Fill(dxh[j]);
            }            
            nPrint++;
            if(y_trk>yInt1[0] && y_trk < yInt2[1]) {
              h14a->Fill(detStrip);
              if(writeEventsWithMissinhHitsToFile) 
                myfile << jentry << " " << detStrip << " " << x_trk << " " << y_trk << endl;              
              //if(nPrint < 100) cout << "Missed hit, event, #clu, nomStrip =  " << jentry << " " 
              //                      << clusterNumberPerEvent << " " << nomStrip << endl;
              for(int j=0; j<min(clusterNumberPerEvent,10); j++){
                if(x_trk>-3.5&&x_trk<-2.0) h1mpa1->Fill(dxh[j]);
                if(x_trk>-2.0&&x_trk<-1.0) h1mpa2->Fill(dxh[j]);
                if(x_trk>-1.0&&x_trk<0.0) h1mpa3->Fill(dxh[j]);
                if(x_trk>0.0&&x_trk<1.0)  h1mpa4->Fill(dxh[j]);
                if(x_trk>1.5&&x_trk<3.5)  h1mpa5->Fill(dxh[j]);
                if(y_trk<yInt1[1]) h1mpaL->Fill(clustersPosition[j]);                         
                if(y_trk>=yInt1[1]) h1mpaU->Fill(clustersPosition[j]);
              }
            }else if(y_trk>yInt3[0] && y_trk < yInt3[1]) {
              h13mbpa->Fill(clusterNumberPerEvent);
              for(int j=0; j<min(clusterNumberPerEvent,10); j++){              
                h1mbpa->Fill(dxh[j]);
                hcmbpa->Fill(polarity*clustersCharge[j]);
              }    
            }
          }
        } 
      }
   }
   if(writeEventsWithMissinhHitsToFile) myfile.close();
   
   //return;
   

   int i1 = h1->FindBin(-0.3);
   int i2 = h1->FindBin(0.3);
   cout << "Number of track - DUT hit matchs: " << h1->Integral(i1,i2) << endl;
   cout << "Ry = " << Ry << endl;
   
   
   TString r1 = Form("%6.3f < Y < %6.3f mm",yInt1[0], yInt1[1]);
   TString r2 = Form("%6.3f < Y < %6.3f mm ",yInt2[0], yInt2[1]);
   TString r3 = Form("%6.3f < Y < %6.3f mm",yInt3[0], yInt3[1]);   


   TF1 *funchole = new TF1("funchole","[0]+[1]*x+[2]*x*x",-8.0,8.0);
   funchole->SetParameters(holeQuadPar[0],holeQuadPar[1],holeQuadPar[2]);
   funchole->SetLineColor(4);
   funchole->SetLineWidth(2);
   cout << "Hole Parameters: " << holeQuadPar[0] << " " << holeQuadPar[1] << " " << holeQuadPar[2] << endl;
   
   gStyle->SetOptStat(1000000001);
   gStyle->SetOptFit(0011);
   gStyle->SetStatH(0.05);
   gStyle->SetStatW(0.2);

   addGraphics(h3b, 1, "X_{trk} [mm]", "Y_{trk} [mm] ");
   h3b->GetXaxis()->SetRangeUser(-5,5);
   h3b->GetYaxis()->SetRangeUser(-5,5);

   TCanvas *c = new TCanvas("c","Residuals",1500,1000);
   c->Divide(4,3);
   c->cd(1)->SetLeftMargin(0.13);
   addGraphics(h2, 1, "X_{trk} [mm]", "X_{DUT} [mm]");
   double dM = yMax;
   if(xMax>dM) dM = xMax;
   h2->GetXaxis()->SetRangeUser(-1.0*dM-1,dM+1);
   h2->GetYaxis()->SetRangeUser(-1.0*dM-1,dM+1);
   h2->SetMinimum(1);
   h2->Draw();

   c->cd(2)->SetLeftMargin(0.13);;
   addGraphics(h1, 1, "#DeltaX [mm]", "");
   addGraphics(h1z, 2, "#DeltaX [mm]", "");
   addGraphics(h1a, 3, "#DeltaX [mm]", "");
   addGraphics(h1b, 6, "#DeltaX [mm]", "");
   h1->GetXaxis()->SetRangeUser(-0.3,0.3);
   if(m_board.Contains("A1")) h1->GetXaxis()->SetRangeUser(-1.5,1.5);
   h1->SetMaximum(1.2*h1->GetMaximum());
   h1->Draw();   
   //h1z->Draw("same");
   h1a->Draw("same");
   h1b->Draw("same");
   TLine *l1 = new TLine(-stripPitch/2.,0,-stripPitch/2.,0.5*h1->GetMaximum());
   TLine *l2 = new TLine(stripPitch/2,0,stripPitch/2,0.5*h1->GetMaximum());
   TLine *l1a = new TLine(-dxWin,0,-dxWin,0.5*h1->GetMaximum());
   TLine *l2a = new TLine(dxWin,0,dxWin,0.5*h1->GetMaximum());
   l1->SetLineColor(kRed); l2->SetLineColor(kRed);
   l1a->SetLineColor(kBlue); l2a->SetLineColor(kBlue); l1a->SetLineWidth(2); l2a->SetLineWidth(2);
   l1->Draw(); l2->Draw();
   //l1a->Draw(); l2a->Draw();

   TLegend* legend3 = new TLegend(0.15,0.70,0.40,0.89);
   legend3->SetFillStyle(0);
   legend3->SetBorderSize(0);
   legend3->SetFillStyle(0);
   legend3->SetTextSize(0.045);
   
   legend3->AddEntry(h1,"All","L"); 
   //legend3->AddEntry(h1z,"ADC < 250","L"); 
   legend3->AddEntry(h1a,"1-strip","L"); 
   legend3->AddEntry(h1b,"2-strip","L"); 
   legend3->Draw();
   
   c->cd(3)->SetLeftMargin(0.13);;
   addGraphics(h8, 1, "#theta_{X}^{trk} [mrad]", "#DeltaX [mm]");
   h8->GetXaxis()->SetRangeUser(txMin-0.1,txMax+0.1);   
   h8->GetYaxis()->SetRangeUser(-0.1,0.1);   
   h8->Draw();
   c->cd(4)->SetLeftMargin(0.13);;
   addGraphics(h9, 1, "Y^{trk} [mm]", "#DeltaX [mm]");
   h9->GetXaxis()->SetRangeUser(yMin-1,yMax+1);   
   h9->GetYaxis()->SetRangeUser(-0.1,0.1);   
   h9->Draw();
   c->cd(5)->SetLeftMargin(0.13);;
   addGraphics(h9a, 1, "X^{trk} [mm]", "#DeltaX [mm]");
   h9a->GetXaxis()->SetRangeUser(-4,4);   
   h9a->GetYaxis()->SetRangeUser(-0.1,0.1);   
   h9a->Draw();

   c->cd(6)->SetLeftMargin(0.13);;
   addGraphics(h3, 1, "X_{trk} [mm]", "Y_{trk} [mm]");
   h3->GetXaxis()->SetRangeUser(xMin-1,xMax+1);   
   h3->GetYaxis()->SetRangeUser(yMin-1,yMax+1);   
   h3->Draw();
   if(holeQuadPar[0]!=0) funchole->Draw("same");
   if(fabs(xLeftHole)<900 && fabs(xRightHole)<900){
     TBox *b1 = new TBox(xMin,yMin,xLeftHole,yMax);
     TBox *b2 = new TBox(xRightHole,yMin,xMax,yMax);
     b1->SetLineColor(kBlue); b1->SetLineWidth(2);b1->SetFillStyle(0);   
     b1->Draw();
     b2->SetLineColor(kBlue); b2->SetLineWidth(2);b2->SetFillStyle(0);   
     b2->Draw();
   }else{   
     TBox *b = new TBox(xMin,yMin,xMax,yMax);
     b->SetLineColor(kBlue); b->SetLineWidth(2);b->SetFillStyle(0);   
     b->Draw();
   }
   

   c->cd(7)->SetLeftMargin(0.13);;
   addGraphics(h5, 1, "#theta_{X}^{trk} [mrad]", "");
   h5->GetXaxis()->SetRangeUser(txMin-0.1,txMax+0.1);   
   h5->Draw();
   h5c->SetLineColor(2); h5c->Draw("same");
   TLine *l1b = new TLine(txMin,0,txMin,h5->GetMaximum());
   TLine *l2b = new TLine(txMax,0,txMax,h5->GetMaximum());
   l1b->SetLineColor(kBlue); l2b->SetLineColor(kBlue); l1b->SetLineWidth(2); l2b->SetLineWidth(2);
   l1b->Draw(); l2b->Draw();

   c->cd(8)->SetLeftMargin(0.13);
   addGraphics(h6, 1, "#theta_{Y}^{trk} [mrad]", "");
   h6->GetXaxis()->SetRangeUser(tyMin-0.1,tyMax+0.1);   
   h6->Draw();
   h6c->SetLineColor(2); h6c->Draw("same");
   TLine *l1c = new TLine(tyMin,0,tyMin,h6->GetMaximum());
   TLine *l2c = new TLine(tyMax,0,tyMax,h6->GetMaximum());
   l1c->SetLineColor(kBlue); l2c->SetLineColor(kBlue); l1c->SetLineWidth(2); l2c->SetLineWidth(2);   
   l1c->Draw(); l2c->Draw();
   
   c->cd(9)->SetLeftMargin(0.13);;
   addGraphics(h4, 1, "Strip # with cluster", "");
   addGraphics(h4a, 2, "Strip # with cluster", "");
   h4->GetXaxis()->SetRangeUser(max(iLo-60.0,1.0),min(iHi+60.0,512.0));
   h4a->GetXaxis()->SetRangeUser(max(iLo-60.0,1.0),min(iHi+60.0,512.0));
   double vmax = h4a->GetMaximum();
   //for(int ii=iLo; ii<=iHi;ii++){
   //  if(h4a->GetBinContent(ii)>vmax) vmax = h4a->GetBinContent(ii);
   //}
   h4->SetMaximum(1.25*vmax);
   TLine *l1d = new TLine(iLo,0,iLo,0.75*h4a->GetMaximum());
   TLine *l2d = new TLine(iHi,0,iHi,0.75*h4a->GetMaximum());
   l1d->SetLineColor(kBlue); l2d->SetLineColor(kBlue); l1d->SetLineWidth(2); l2d->SetLineWidth(2);   
   h4->Draw();
   h4a->Draw("same");
   h4c->SetLineColor(4);
   h4c->Draw("same");
   l1d->Draw(); l2d->Draw();
   
   TLegend* legend4 = new TLegend(0.15,0.70,0.94,0.89);
   legend4->SetFillStyle(0);
   legend4->SetBorderSize(0);
   legend4->SetFillStyle(0);
   legend4->SetTextSize(0.045);
   
   legend4->AddEntry(h4,"Sensor strip ch #","L"); 
   legend4->AddEntry(h4a,"Electr. ch #","L");

   legend4->Draw();


   c->cd(10)->SetLeftMargin(0.13);;
   addGraphics(hcAll, 1, "Cluster charge [ADC]", "");
   addGraphics(hcTrk1, 3, "Cluster charge [ADC]", "");
   addGraphics(hcTrk2, 6, "Cluster charge [ADC]", "");
   hcAll->SetMaximum(1.25*hcAll->GetMaximum());
   hcAll->Draw();
   hcTrk->SetLineColor(kBlue); hcTrk->SetLineWidth(2);
   hcTrk->Draw("same"); 
   hcTrk1->Draw("same"); 
   hcTrk2->Draw("same"); 
  
   TLegend* legend2 = new TLegend(0.15,0.70,0.94,0.89);
   legend2->SetFillStyle(0);
   legend2->SetBorderSize(0);
   legend2->SetFillStyle(0);
   legend2->SetTextSize(0.045);
   
   legend2->AddEntry(hcAll,"All clusters, trk in Fid","L"); 
   legend2->AddEntry(hcTrk,"Clusters, |#DeltaX|<200 #mum, trk in Fid.","L");
   legend2->AddEntry(hcTrk1,"1-strip Clusters","L");
   legend2->AddEntry(hcTrk2,"2-strip Clusters","L");

   legend2->Draw();

   

   c->cd(11)->SetLeftMargin(0.13);;
   addGraphics(h10a, 1, "Strip #", "<ADC>");
   addGraphics(h10b, 2, "Strip #", "<ADC>");
   addGraphics(h10c, 4, "Strip #", "<ADC>");
   int ixl = iLo;
   int ixh = iHi;
   int yh = hcTrk->GetMean()-50;
   h10a->GetXaxis()->SetRangeUser(ixl-2,ixh+2);   
   h10a->GetYaxis()->SetRangeUser(yh-200,yh+300);
   /*
   h10a->Draw();
   h10b->SetLineColor(2);h10b->SetMarkerColor(2);
   h10b->Draw("same");
   h10c->SetLineColor(4);h10c->SetMarkerColor(4);
   h10c->Draw("same");

   TLegend* legend1 = new TLegend(0.25,0.75,0.94,0.89);
   legend1->SetFillStyle(0);
   legend1->SetBorderSize(0);
   legend1->SetFillStyle(0);
   legend1->SetTextSize(0.045);
   
   legend1->AddEntry(h10a,"Top half in Y","LEP"); 
   legend1->AddEntry(h10b,"Bottom half in Y","LEP"); 
   legend1->AddEntry(h10c,"Top 2 mm in Y","LEP"); 
   legend1->Draw();
   */
   addGraphics(h35, 1, "#DUT clusters", "Entries");   
   h35->GetYaxis()->SetTitleOffset(1.2);
   h35->Draw();

   c->cd(12)->SetLeftMargin(0.13);;
   addGraphics(h2p, 1, "TDC time / 2.5 ns", "<ADC>");
   h2p->Draw();
   TLine *l1e = new TLine(tdcLo,0,tdcLo,h2p->GetMaximum());
   TLine *l2e = new TLine(tdcHi,0,tdcHi,h2p->GetMaximum());
   l1e->SetLineColor(kBlue); l2e->SetLineColor(kBlue); l1e->SetLineWidth(2); l2e->SetLineWidth(2);   
   l1e->Draw(); l2e->Draw();

   if(printPlots) c->Print("Plots/plot_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");

   //return;
   

   /*
   TCanvas *c4 = new TCanvas("c4","Plot 4",800,600);
   c4->Divide(2,2);
   c4->cd(1)->SetLeftMargin(0.13);
   addGraphics(h12en, 1, "X_{trk} [mm]", "");
   addGraphics(h12fn, 1, "X_{trk} [mm]", "");

   TH1F *hepa1 = (TH1F*)h12en->Clone("hepa1");
   TH1F *hepa2 = (TH1F*)h12fn->Clone("hepa2");
   TH1F *henpa = (TH1F*)h12gn->Clone("henpa");
   hepa1->Divide(h12en,h12ed,1.0,1.0,"B");
   hepa2->Divide(h12fn,h12fd,1.0,1.0,"B");
   henpa->Divide(h12gn,h12gd,1.0,1.0,"B");
   addGraphics(hepa1, 1, "X_{trk} [mm]", "#Good DUT hit / # Track ");
   addGraphics(hepa2, 1, "X_{trk} [mm]", "#Good DUT hit / # Track ");
   h3b->GetYaxis()->SetRangeUser(-5,5);
   h3b->Draw();

   c4->cd(2)->SetLeftMargin(0.13);
   hepa1->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   hepa1->SetMinimum(0.0);
   hepa1->SetMaximum(1.2);
   hepa1->SetLineWidth(1);
   hepa1->SetTitle(r1);
   hepa1->Draw("hist");
   c4->cd(3)->SetLeftMargin(0.13);
   hepa2->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   hepa2->SetMinimum(0.0);
   hepa2->SetMaximum(1.2);
   hepa2->SetLineWidth(1);
   hepa2->SetTitle(r2);
   hepa2->Draw("hist");

   c4->cd(4)->SetLeftMargin(0.13);
   addGraphics(henpa, 1, "X_{trk} [mm]", "#Good DUT hit / # Track ");
   henpa->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   henpa->SetMinimum(0.0);
   henpa->SetMaximum(1.2);
   henpa->SetLineWidth(1);
   henpa->SetTitle(r3);
   henpa->Draw("hist");   


   //TH1F *hepas2 = (TH1F*)h12in->Clone("hepas2");
   //TH1F *hepas3 = (TH1F*)h12jn->Clone("hepas3");
   //TH1F *hepas4 = (TH1F*)h12kn->Clone("hepas4");
   TCanvas *c5 = new TCanvas("c5","Plot 5",800,600);
   c5->Divide(2,2);
   c5->cd(1)->SetLeftMargin(0.13);
   addGraphics(h12hn, 1, "Rel. Strip Pos", "");
   addGraphics(h12in, 1, "Rel. Strip Pos", "");
   addGraphics(h12jn, 1, "Rel. Strip Pos", "");
   addGraphics(h12kn, 1, "Rel. Strip Pos", "");

   hepas2->Divide(h12in,h12id,1.0,1.0,"B");
   hepas3->Divide(h12jn,h12jd,1.0,1.0,"B");
   hepas4->Divide(h12kn,h12kd,1.0,1.0,"B");
   addGraphics(hepas1, 1, "Rel. Strip Pos.", "#Good DUT hit / # Track ");
   addGraphics(hepas2, 1, "Rel. Strip Pos.", "#Good DUT hit / # Track ");
   addGraphics(hepas3, 1, "Rel. Strip Pos.", "#Good DUT hit / # Track ");
   addGraphics(hepas4, 1, "Rel. Strip Pos.", "#Good DUT hit / # Track ");
   hepas1->Draw("hist");
   c5->cd(2)->SetLeftMargin(0.13);
   hepas2->SetTitle(r1);
   hepas2->SetMinimum(0.8);
   hepas2->SetMaximum(1.2);
   hepas2->SetLineWidth(1);
   hepas2->Draw("hist");
   c5->cd(3)->SetLeftMargin(0.13);
   hepas3->SetTitle(r2);
   hepas3->SetMinimum(0.8);
   hepas3->SetMaximum(1.2);
   hepas3->SetLineWidth(1);
   hepas3->Draw("hist");
   c5->cd(4)->SetLeftMargin(0.13);
   hepas4->SetTitle(r3);
   hepas4->SetMinimum(0.8);
   hepas4->SetMaximum(1.2);
   hepas4->SetLineWidth(1);
   hepas4->Draw("hist");

   if(printPlots) c5->Print("Plots/plot5_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");
   */
   //return;
   

   TCanvas *c1 = new TCanvas("c1","Plot 1",1600,800);
   c1->Divide(3,2);

   c1->cd(1)->SetLeftMargin(0.13);
   addGraphics(h10d, 2, "Y_{trk} [mm]", "<ADC>");
   h10d->GetXaxis()->SetRangeUser(yMin-0.5,yMax+0.5);
   h10d->Draw();

   //c1->cd(2)->SetLeftMargin(0.13);
   //h3b->Draw();   

   c1->cd(2)->SetLeftMargin(0.13);
   addGraphics(h12b, 2, "Y_{trk} [mm]", "");
   addGraphics(h12a, 1, "Y_{trk} [mm]", "");
   h12a->SetMaximum(1.2*h12a->GetMaximum());
   h12a->GetXaxis()->SetRangeUser(yMin-0.5,yMax+0.5);
   h12a->Draw("hist");
   h12b->Draw("hist,same");

   TLegend* legend0 = new TLegend(0.15,0.75,0.94,0.89);
   legend0->SetFillStyle(0);
   legend0->SetBorderSize(0);
   legend0->SetFillStyle(0);
   legend0->SetTextSize(0.045);   
   legend0->AddEntry(h12a,"Track Y pos","LEP"); 
   legend0->AddEntry(h12b,"Track Y pos with good cluster","LEP");
   legend0->Draw();

   c1->cd(3)->SetLeftMargin(0.13);
   TH1F *he = (TH1F*)h12b->Clone("he");
   he->Divide(h12b,h12a,1.0,1.0,"B");
   float bw = 1000*h12b->GetBinWidth(1);
   TString yt = Form("(#Good DUT hit / # Track) / %3.0f #mum",bw);
   addGraphics(he, 1, "Y_{trk} [mm]", yt);
   he->SetTitle("DUT Efficiency vs Y_{trk}");
   he->GetXaxis()->SetRangeUser(yMin-0.5,yMax+0.5);
   he->SetMinimum(0.8);
   he->SetMaximum(1.1);
   he->Draw("e");

   c1->cd(4)->SetLeftMargin(0.13);
   addGraphics(h10e, 2, "X_{trk} [mm]", "<ADC>");
   h10e->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   h10e->Draw();

   c1->cd(5)->SetLeftMargin(0.13);
   addGraphics(h12dd, 2, "X_{trk} [mm]", "");
   addGraphics(h12dn, 1, "X_{trk} [mm]", "");
   h12dd->SetMaximum(1.2*h12dd->GetMaximum());
   h12dd->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   h12dd->Draw("hist");
   h12dn->Draw("hist,same");

   c1->cd(6)->SetLeftMargin(0.13);
   TH1F *he2 = (TH1F*)h12dn->Clone("he");
   he2->Divide(h12dn,h12dd,1.0,1.0,"B");
   bw = 1000*h12dd->GetBinWidth(1);
   yt = Form("(#Good DUT hit / # Track) / %3.0f #mum",bw);
   addGraphics(he2, 1, "X_{trk} [mm]", yt);
   he2->SetTitle("DUT Efficiency vs X_{trk}");
   he2->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   he2->SetMinimum(0.8);
   he2->SetMaximum(1.1);
   he2->Draw("e");
   if(printPlots) c1->Print("Plots/plot1_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");

   TCanvas *c8 = new TCanvas("c8","Plot 8",800,400);
   c8->Divide(2,1);
   TH1F *he4 = (TH1F*)h15c->Clone("Eff_trigger-DUT_time");
   he4->Divide(h15c,h16c,1.0,1.0,"B");
   yt = "#Good DUT hit / # Track";
   addGraphics(h16c, 1, "DUT time - Track Time (ns)", "Entries");
   addGraphics(he4, 1, "DUT time - Track Time (ns)", "Efficiency");
   c8->cd(1)->SetLeftMargin(0.15);;
   c8->cd(1)->SetRightMargin(0.05);;
   h16c->Draw();
   c8->cd(2)->SetLeftMargin(0.13);;
   c8->cd(2)->SetRightMargin(0.05);;
   he4->Draw();
   if(printPlots) c8->Print("Plots/plot8_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");   

   if(holeSector){
     TCanvas *c7 = new TCanvas("c7","Plot 7",800,800);
     c7->SetLeftMargin(0.13);
     TH1F *he3 = (TH1F*)h12on->Clone("Erf()_fit");
     he3->Divide(h12on,h12od,1.0,1.0,"B");
     bw = 1000*h12od->GetBinWidth(1);
     yt = Form("(#Good DUT hit / # Track) / %3.0f #mum",bw);
     addGraphics(he3, 1, "Dist to cutout [mm]", yt);
     he3->SetStats(kTRUE);
     he3->SetTitle("DUT Efficiency vs Dist to Cutout");
     he3->GetXaxis()->SetRangeUser(-0.2,2.0);
     he3->GetYaxis()->SetRangeUser(0.0,1.3);
     he3->Draw("e");
     TLatex *myLatex = new TLatex();
     myLatex->SetTextFont(42); myLatex->SetTextColor(1); 
     myLatex->SetTextAlign(12); myLatex->SetNDC(kTRUE); myLatex->SetTextSize(0.047);
     TString text = "Sector "+m_sector;
     c7->cd();
     myLatex->DrawLatex(0.18,0.8,text);
     
     TF1 *f4a = new TF1("f4a","0.5*[0]*(1+TMath::Erf((x-[1])/[2]))",-0.1,2.0);
     f4a->SetParameters(0.5,0.0,0.04);
     f4a->SetParNames("Const","Mean","Sigma");
     he3->Fit("f4a","R");
     TPad *inset = new TPad("inset","y vx x",0.44,0.1,0.89,0.55);
     inset->SetLeftMargin(0.13);
     inset->SetRightMargin(0.05);
     h3->SetTitle("");
     inset->Draw();
     inset->cd();
     h3->Draw();
     h3b->SetMarkerSize(0.2);h3b->SetMarkerStyle(20);h3b->SetMarkerColor(kRed); 
     h3b->Draw("same");
     if(holeQuadPar[0]!=0) funchole->Draw("same");   
     
     if(printPlots) c7->Print("Plots/plot7_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");   
   }
   
   /*
   TCanvas *c2 = new TCanvas("c2","Plot 2",1600,600);
   c2->Divide(5,2);
   addGraphics(h1mpa, 2, "X_{clu}-X_{trk} [mm]", ""); h1mpa->SetTitle("2.8<Y_{trk}<4.1 mm, All X, No Match Clu");
   addGraphics(h1mbpa, 4, "X_{clu}-X_{trk} [mm]", ""); h1mbpa->SetTitle("1.0<Y_{trk}<2.3 mm, All X, No Match Clu");
   addGraphics(h1fpa, 1, "X_{clu}-X_{trk} [mm]", ""); h1fpa->SetTitle("2.8<Y_{trk}<4.1 mm, All X, Match Clu");
   addGraphics(h1fbpa, 1, "X_{clu}-X_{trk} [mm]", ""); h1fbpa->SetTitle("1.0<Y_{trk}<2.3 mm, All X, Match Clu");

   addGraphics(h1mpa1, 2, "X_{clu}-X_{trk} [mm]", ""); h1mpa1->SetTitle("2.8<Y_{trk}<4.1 mm, -3.5<X_{trk}<-2.0 mm, No Match Clu");
   addGraphics(h1mpa2, 2, "X_{clu}-X_{trk} [mm]", ""); h1mpa2->SetTitle("2.8<Y_{trk}<4.1 mm, -2.0<X_{trk}<-1.0 mm, No Match Clu");
   addGraphics(h1mpa3, 2, "X_{clu}-X_{trk} [mm]", ""); h1mpa3->SetTitle("2.8<Y_{trk}<4.1 mm, -1.0<X_{trk}<0.0 mm, No Match Clu");
   addGraphics(h1mpa4, 2, "X_{clu}-X_{trk} [mm]", ""); h1mpa4->SetTitle("2.8<Y_{trk}<4.1 mm,  0.0<X_{trk}<1.0 mm, No Match Clu");
   addGraphics(h1mpa5, 2, "X_{clu}-X_{trk} [mm]", ""); h1mpa5->SetTitle("2.8<Y_{trk}<4.1 mm,  1.5<X_{trk}<3.5 mm, No Match Clu");
   h1mpa->SetLineWidth(1); 
   h1mpa1->SetLineWidth(1); 
   h1mpa2->SetLineWidth(1); 
   h1mpa3->SetLineWidth(1); 
   h1mpa4->SetLineWidth(1); 
   h1mpa5->SetLineWidth(1); 
   h1fpa->SetLineWidth(1); 
   h1fbpa->SetLineWidth(1); 

   
   h1fpa->GetXaxis()->SetRangeUser(-3,9); 
   h1fbpa->GetXaxis()->SetRangeUser(-3,9); 
   h1mbpa->GetXaxis()->SetRangeUser(-3,9); 
   h1mpa->GetXaxis()->SetRangeUser(-3,9); 
   h1mpa1->GetXaxis()->SetRangeUser(-3,9); 
   h1mpa2->GetXaxis()->SetRangeUser(-3,9); 
   h1mpa3->GetXaxis()->SetRangeUser(-3,9); 
   h1mpa4->GetXaxis()->SetRangeUser(-3,9); 
   h1mpa5->GetXaxis()->SetRangeUser(-3,9); 

   c2->cd(1);
   h3b->Draw();   
   double xl[5] = {-3.5, -2.0, -1.0, 0.0, 1.5};
   double xh[5] = {-2.0, -1.0,  0.0, 1.0, 3.5};
   
   
   for(int i=0;i<5;i++){
     TBox *b = new TBox(xl[i],yInt1[0],xh[i],yInt2[1]);
     b->SetLineColor(2); b->SetFillStyle(0);b->SetLineWidth(2);
     b->Draw();
   }
   TBox *b = new TBox(xl[0],yInt3[0],xh[4],yInt3[1]);
   b->SetLineColor(4);b->SetFillStyle(0);b->SetLineWidth(2);
   b->Draw();
   


   c2->cd(2);
   h1mpa->Draw();   
   c2->cd(3);
   h1fpa->GetYaxis()->SetRangeUser(0,30);
   h1fpa->Draw();   
   c2->cd(4);
   h1mbpa->Draw();   
   c2->cd(5);
   h1fbpa->GetYaxis()->SetRangeUser(0,30);
   h1fbpa->Draw();   
   c2->cd(6);   
   h1mpa1->Draw();   
   c2->cd(7);
   h1mpa2->Draw();   
   c2->cd(8);
   h1mpa3->Draw();   
   c2->cd(9);
   h1mpa4->Draw();   
   c2->cd(10);
   h1mpa5->Draw();
   c2->Print("Plots/plot2_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");
   */

   /*
   TCanvas *c3 = new TCanvas("c3","Plot 3",800,600);
   c3->Divide(1,2);
   c3->cd(1);
   h1mpaU->GetXaxis()->SetRangeUser(iLo-10,iHi+10);
   h1mpaU->Draw();
   c3->cd(2);
   h1mpaL->GetXaxis()->SetRangeUser(iLo-10,iHi+10);
   h1mpaL->Draw();   
   c3->Print("Plots/plot3_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");
   */

   TH1F *hepas1 = (TH1F*)h12hn->Clone("hepas1");
   hepas1->Divide(h12hn,h12hd,1.0,1.0,"B");
   hepas1->SetTitle("Full Beam Spot");
   addGraphics(hepas1, 1, "Rel. Strip Pos.", "#Good DUT hit / # Track ");
   hepas1->SetMinimum(0.8);
   hepas1->SetMaximum(1.2);
   hepas1->SetLineWidth(1);
   addGraphics(h12m,1,"Interstrip Pos","<ADC>");
   addGraphics(h12n,1,"Interstrip Pos","<Cluster Size>");
   TCanvas *c6 = new TCanvas("c6","Plot 6",1200,400);
   c6->Divide(3,1);
   c6->cd(1);
   hepas1->SetMaximum(1.2);
   hepas1->SetMinimum(0.5);
   hepas1->Draw();
   c6->cd(2);
   h12m->SetMinimum(0);
   h12m->SetMaximum(1.2*h12m->GetMaximum());
   h12m->Draw();
   c6->cd(3);
   h12n->SetMinimum(0.5);
   h12n->SetMaximum(2.0);
   h12n->Draw();
   
  if(printPlots)  c6->Print("Plots/plot6_"+m_board+"_s"+m_sector+"_vb"+m_bias+".png");


   fout->Write();
   

}
