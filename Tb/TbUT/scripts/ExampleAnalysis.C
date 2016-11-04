//
//    Example Analysis code
//    Stripped down version to loop over tracks and clusters with alignment, etc.
//    Date: Oct 27, 2015
// 
#define ExampleAnalysis_cxx
#include "ExampleAnalysis.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>


void ExampleAnalysis::Loop(){

  if (fChain == 0) return;

   Int_t nentries = fChain->GetEntriesFast();

   //--------------------------------------
   // Create output file to save histograms
   //--------------------------------------
   fout = new TFile(f_out,"RECREATE");

   // Just add 2 histograms, as examples
   TH1F* h1w = new TH1F("h1w","#DeltaX",20000,-100.0,100.0);
   TH1F *hnoise = new TH1F("hnoise","Noise in connected channels",100,-200,200);
   TH1F* h12dd = new TH1F("h12dd","X position of track",200,-10.0,10.0);h12dd->Sumw2();
   TH1F* h12dn = new TH1F("h12dn","X position of track",200,-10.0,10.0);h12dn->Sumw2();
   //------------------------------------------------------------------------------------------------------

   //-----------------------------------------------
   // Example for writing some information to a file
   //-----------------------------------------------
   std::ofstream myfile;
   if(writeEventsWithMissinhHitsToFile){
     myfile.open("MissingDUTHits.dat");
   }

   // Prepare DUT - Alignment, etc
   PrepareDUT();
   

   // ----------------------
   // Begin Analysis Loop
   // ----------------------
   int iChan = nChan;
   double nomStrip = 0;
   int njump = 10000;
   if(nentries > 100000) njump = 50000;
   Long64_t nbytes = 0, nb = 0;
   cout << "Begin loop over " << nentries << " events" << endl;

   // Begin loop over events
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      if(jentry%njump==0) cout << "====> At entry = " << jentry << endl;
      if(n_tp3_tracks != 1) continue; // Require 1 and only 1 TPIX track

      //==========================================
      // Example here of acccessing CMS strip data
      //==========================================
      if(fCMS){
        for(int k=0;k<iChan;k++){
          if(fCMS->cmsData[k]!=0) hnoise->Fill(fCMS->cmsData[k]);
        }      
      }
      
      // Loop over TPIX tracks in event
      for(int k=0; k<n_tp3_tracks; k++){
        if(dtime > trackTriggerTimeDiffCut) continue;
        double x_trk = vec_trk_tx->at(k)*z_DUT+vec_trk_x->at(k);
        double y_trk = vec_trk_ty->at(k)*z_DUT+vec_trk_y->at(k);
        double tx = 1000*vec_trk_tx->at(k);
        double ty = 1000*vec_trk_ty->at(k);

        transformTrackToDUTFrame(k, x_trk, y_trk, nomStrip);

        // If you want to remove tracks in region of hole, can change 'removeTracksInHoleDef = false' to not remove them.
        if(isInCutoutRegion(x_trk, y_trk)) continue;
       
        bool goodRegion = true;
        for(int id = 0; id<nDeadRegion; id++){
          if(x_trk>=deadRegionLo[id]  && x_trk<=deadRegionHi[id]) goodRegion = false;  
        }
        if(!goodRegion) continue;

        bool goodTrack = false;
        bool inFiducialX = false;
        bool inFiducialY = false;
        if(x_trk>xMin && x_trk<xMax) inFiducialX = true;          
        if(y_trk>yMin && y_trk<yMax) inFiducialY = true;          
        bool inFiducial = inFiducialX && inFiducialY;
        inFiducial = inFiducial && (x_trk<xLeftHole || x_trk>xRightHole);
        
        if(tx>txMin && tx<txMax && ty>tyMin && ty<tyMax) goodTrack = true;        
        bool goodTime =  (clustersTDC >= tdcLo && clustersTDC <= tdcHi);

        bool foundHit = false;
        double x_trk0 = x_trk;

        // Begin loop over CLUSTERS
        for(int j=0; j<min(clusterNumberPerEvent,10); j++){
          if(clustersPosition[j] < 0.1) continue;
          x_trk = x_trk0;

          double x_dut = getDUTHitPosition(j);
          
          double dx = x_dut - x_trk;

          if(goodTrack && inFiducial && goodTime) {
            h1w->Fill(dx);
            if(fabs(dx)<dxWin) foundHit = true;
          }
        } // End Loop over Clusters

        // ------------------------
        // Efficiency studies, etc
        // ------------------------
        if(inFiducial && goodTrack && goodTime) {
          h12dd->Fill(x_trk); // X pos of good tracks
          if(foundHit) {
            h12dn->Fill(x_trk);  // X position when a hit was found within 200 um
          } else {   
            // Here, you might do something else when no DUT hits are not found
            if(writeEventsWithMissinhHitsToFile) 
              myfile << jentry << " " << nomStrip << " " << x_trk << " " << y_trk << endl;              
          }
        } 
      }// End Loop over Tracks
   } // End Loop over Events

   if(writeEventsWithMissinhHitsToFile) myfile.close();
   

   fout->Write();
   

}
