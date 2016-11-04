#define ClusterAna_cxx
#include "ClusterAna.h"
#include <TH2.h>
#include <TStyle.h>
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

void ClusterAna::Loop()
{
//   In a ROOT session, you can do:
//      Root > .L ClusterAna.C
//      Root > ClusterAna t
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
   
   TString m_board2 = m_board;
   m_board2 = m_board2.ReplaceAll("_All","");
   m_board2 = m_board2.ReplaceAll("_Full","");
   m_board2 = m_board2.ReplaceAll("_v7","");
   Long64_t nentries = fChain->GetEntriesFast();
   TString f_out			= m_fileOutdir + plotdir + "/AnalysisOutputCluOnly_" + m_board2 + "_" + m_bias + "_" + m_sector + ".root"; 

   fout = new TFile(f_out,"RECREATE");
   TH1F* h4 = new TH1F("h4","Strip # of cluster with track",512,0.0,512);
   TProfile *hp = new TProfile("hp","Cluster Charge vs TDC time",12,0,12,100,1000);
   TH1F* hcAll = new TH1F("hcAll","Cluster charge",100,0.0,1000.0);
   TH1F *hnoiseChan = new TH1F("hnoiseChan","Noise in connected channels",200,0,200);
   TH1F *hnoiseChan2 = new TH1F("hnoiseChan2","Noise in connected channels",200,-200,200);

   PrepareDUT();
   std::cout << iLo << " " << iHi << std::endl;
   std::cout << tdcLo << " " << tdcHi << std::endl;
   

   //nentries = 20;
   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      
      int nn = 0;
      for(int j=0; j<std::min(clusterNumberPerEvent,10); j++){
        if(clustersPosition[j]>=iLo && clustersPosition[j]<=iHi) nn++;
      }
      if(nn != 1) continue;
      
      
      for(int j=0; j<std::min(clusterNumberPerEvent,10); j++){
        h4->Fill(clustersPosition[j]);
        if(clustersPosition[j] < 0.1) continue;
        //std::cout << polarity*clustersCharge[j] << " " << clustersTDC << " " << kClusterChargeMin << endl;
        if(clustersPosition[j]>=iLo && clustersPosition[j]<=iHi && clustersTDC>1.0 && polarity*clustersCharge[j]>kClusterChargeMin 
           && polarity*clustersCharge[j]<800) hp->Fill(clustersTDC+0.1,polarity*clustersCharge[j]);

        bool beamRegion = clustersPosition[j]>=iLo && clustersPosition[j]<=iHi;
        bool goodTDC = clustersTDC>=tdcLo && clustersTDC<=tdcHi;
        double chr2 = clustersCharge2StripRight[j]*polarity;
        double chl2 = clustersCharge2StripLeft[j]*polarity;                

        if(beamRegion && goodTDC) {
          hcAll->Fill(polarity*clustersCharge[j]);
          hnoiseChan2->Fill(chl2-chr2);
          int ichan = clustersSeedPosition[j];
          hnoiseChan->Fill(noise[ichan]);
        }
        

      }
      // if (Cut(ientry) < 0) continue;
   }
   
   TCanvas *c = new TCanvas("c","Mon Plots",800,600);
   c->Divide(2,2);
   c->cd(1);
   h4->Draw();
   c->cd(2);
   hp->Draw();
   c->cd(3);
   hcAll->Draw();
   c->cd(4);
   hnoiseChan->Draw();
   
   fout->Write();
   
}
