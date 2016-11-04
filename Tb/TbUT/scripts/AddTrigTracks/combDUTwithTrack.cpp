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

#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"

#ifdef __MAKECINT__
#pragma link C++ class vector<float>+;
#endif

using namespace std;

int main(int __argc, char *__argv[]){

  gROOT->ProcessLine("#include <vector>");
  const char *dut_filename  = "dut.root";
  const char *tp3_filename = "tp3.root";
  const char *out_filename = "outputfile.root";
	const char *noise_filename = "noise.root"; 

  int c;
  extern char* optarg;
  extern int optind;

  int _triggerPlane(-1);
  int _timeOffset = 0;

  cout << endl;
  
  /*____________________________Parse Command Line___________________________*/
  while((c = getopt(__argc,__argv,"i:t:n:o:h")) != -1){
    switch(c){
      case 'h': // help option
        cout << "Example: ./combineDUTwithTrack -i dut.root -t tp3.root -o outputfile.root" << endl; 
        return 0;
        break;
      case 'i':
        dut_filename = optarg;
        cout << "---> DUT ROOT filename: " << dut_filename << endl;
        break;
      case 't':
        tp3_filename = optarg;
        cout << "---> TimePix3 ROOT filename: " << tp3_filename << endl;
        break;
      case 'o':
        out_filename = optarg;
        cout << "---> Combined output filename: " << out_filename << endl;
        break;
      case 'n':
        noise_filename = optarg;
        cout << "---> Noise ROOT filename: " << noise_filename << endl;
        break;
      default: // unknown option flag
        printf("Error!!!! Unknown option -%c\n",c);
        cout << "Example: combineDUTwithTrack -i dut.root -t tp3.root -o outputfile.root" << endl; 
      return 0;
    }
  }

  cout << endl;
  

  // get info from Kepler file
  double trk_x, trk_y, trk_tx, trk_ty, chi2ndf, trk_htime, trg_htime0;
  ULong64_t trg_time0, trg_time0_25ns;
  ULong64_t trk_time, trk_time_25ns, timestamps;
  ULong64_t time_window = 1000;
  ULong64_t diff_time;
  ULong64_t diff_time_loc;  
  UInt_t trg_Plane;
  
  TBranch        *b_timestamps;   //!
  
	cout << "Made it this far!" << endl; 
 
  //dut_filename = "/data2/pmmannin/BoardA4_redo_v2/Run_Bias_Scan-B1-A-227-8711_Tuple.root";
  TFile *f_ut 		= new TFile(dut_filename,"READONLY");
  TFile *f_noise 	= new TFile(noise_filename,"READONLY"); 
	TTree* t_dut 		= (TTree*)f_ut->Get("TbUT/Clusters");
  t_dut->SetBranchAddress("timestamps",&timestamps, &b_timestamps);

	cout << "Made it a little further! " << endl; 
  int numEvents = t_dut->GetEntriesFast();
  cout << " ------------------------------------------------------" << endl;
  cout << " | Number of triggers found = " << numEvents << endl;
  cout << " ------------------------------------------------------" << endl;

  //tp3_filename = "/data2/sblusk/TB/July2015/TelescopeData/BoardA4/Kepler-tuple.root";
  TFile *f_tpix = new TFile(tp3_filename,"READONLY");
  TTree *t_trk = (TTree*)f_tpix->Get("TbTupleWriter/Tracks");
  t_trk->SetBranchAddress("TkTime",&trk_time);
  t_trk->SetBranchAddress("TkHTime",&trk_htime);
  t_trk->SetBranchAddress("TkX",&trk_x);
  t_trk->SetBranchAddress("TkY",&trk_y);
  t_trk->SetBranchAddress("TkTx",&trk_tx);
  t_trk->SetBranchAddress("TkTy",&trk_ty);
  t_trk->SetBranchAddress("TkChi2PerNdof",&chi2ndf);

  int nfound=0;  
  //out_filename = "/data2/sblusk/test.root";
  TFile *f_out = new TFile(out_filename,"recreate");
  TTree *t_out = (TTree*)t_dut->CloneTree(0); // grab a copy of the existing DUT tree

  //TFile *fileout = new TFile("temp.root","RECREATE");
  TH1F *h1 = new TH1F("h1","Time Diff",1000,-1000,1000);
  TH1F *hMeanNoise = new TH1F("hMeanNoise","Mean of noise, CM subtracted",512,0,512);
  TH1F *hWidthNoise = new TH1F("hWidthNoise","Gaussian noise, CM subtracted",512,0,512);
  TH1D *px[512];

  // add the new branches w/ track info
  int n_tp3_tracks;
  std::vector<double> *vec_trk_x = 0;
  std::vector<double> *vec_trk_y = 0;
  std::vector<double> *vec_trk_tx = 0;
  std::vector<double> *vec_trk_ty = 0;
  std::vector<double> *vec_trk_htime = 0; 
  std::vector<double> *vec_trk_chi2ndf = 0;


  t_out->Branch("n_tp3_tracks",&n_tp3_tracks);
  t_out->Branch("vec_trk_x",&vec_trk_x);
  t_out->Branch("vec_trk_y",&vec_trk_y);
  t_out->Branch("vec_trk_tx",&vec_trk_tx);
  t_out->Branch("vec_trk_ty",&vec_trk_ty);
  t_out->Branch("vec_trk_chi2ndf",&vec_trk_chi2ndf);
  //t_out->Branch("vec_trk_htime",&vec_trk_htime);  // don't need this one

  double dtime;
  t_out->Branch("dtime",&dtime);


  int entries = (int)t_trk->GetEntries();
  int curr_pos(0), j, stop_now;

  // First short pass, check trigger timing
  int iAli = 0;
  for(int i=0;i < min(numEvents,10000); i++){
    int nb = t_dut->GetEntry(i);
    if(nb <= 0) {

      cout << "breaking" << endl;
      break;    
    }
    //t_dut->GetEntry(i);
    if(i%1000==0) cout << "\nprocessing " << i << " curr_pos: " << curr_pos << " " << endl;
    trg_time0_25ns = timestamps - _timeOffset;

    stop_now = 0;
    j = curr_pos;
    while(j < entries && stop_now == 0){
       t_trk->GetEntry(j);
       trk_time_25ns = (trk_time>>12);
       
       if(trg_time0_25ns > trk_time_25ns){
           diff_time_loc = trg_time0_25ns - trk_time_25ns;
       }
       if(trk_time_25ns > trg_time0_25ns){
          diff_time_loc = trk_time_25ns - trg_time0_25ns;
          if(diff_time_loc > time_window) {stop_now=1; curr_pos = j-1;}
       }
       if(trk_time_25ns == trg_time0_25ns){
          diff_time_loc = trk_time_25ns - trg_time0_25ns;
       }

       double dt = diff_time_loc * 1.0;
       h1->Fill(dt);       
      j++;
    }

    if(curr_pos < 0) curr_pos = 0;
    
  }

  double _timeOffsetNew = _timeOffset;
  int ib = h1->GetMaximumBin();
  if(ib>0 && ib<h1->GetNbinsX()) _timeOffsetNew = _timeOffsetNew + h1->GetBinCenter(ib);
  cout << "-----------------------------------------------" << endl;
  cout << "Resetting time offset to " << _timeOffsetNew << endl;
  cout << "Setting time window to 5 ns " << endl;
  cout << "-----------------------------------------------" << endl;  

  time_window = 5;
  h1->Reset();

  nfound=0;
  iAli = 0;
  curr_pos = 0;
  //numEvents = 5000;
  
  for(int i=0;i < numEvents; i++){
    int nb = t_dut->GetEntry(i);
    if(nb <= 0) break;    
    if(i%10000==0) cout << "\nprocessing " << i << " curr_pos: " << curr_pos << " " << endl;

    trg_time0_25ns = timestamps - _timeOffsetNew; // in units of 25ns minus the offset
    vec_trk_x->clear();
    vec_trk_y->clear();
    vec_trk_tx->clear();
    vec_trk_ty->clear();
    vec_trk_chi2ndf->clear();
    //vec_trk_htime->clear();

    stop_now = 0;
    j = curr_pos;
    while(j < entries && stop_now == 0){
      int nb2 = t_trk->GetEntry(j);
      if(nb2 <= 0) {
	cout << "breaking!" << endl;
break;

      }
       trk_time_25ns = (trk_time>>12);
       if(trg_time0_25ns > trk_time_25ns){
           diff_time_loc = trg_time0_25ns - trk_time_25ns;
       }
       if(trk_time_25ns > trg_time0_25ns){
          diff_time_loc = trk_time_25ns - trg_time0_25ns;
          if(diff_time_loc > time_window) {stop_now=1; curr_pos = j-1;}
       }
       if(trk_time_25ns == trg_time0_25ns){
          diff_time_loc = trk_time_25ns - trg_time0_25ns;
       }

       double dt = diff_time_loc * 1.0;
       h1->Fill(dt);
       

       //if(diff_time_loc - _timeOffsetNew < time_window){
       if(dt < time_window){
         //vec_trk_htime->push_back((trk_htime - trg_htime0));
         //cout << diff_time_loc - _timeOffsetNew << " " << dt << endl;
         dtime = dt;
         vec_trk_x->push_back(trk_x);
         vec_trk_y->push_back(trk_y);
         vec_trk_tx->push_back(trk_tx);
         vec_trk_ty->push_back(trk_ty);
         vec_trk_chi2ndf->push_back(chi2ndf);
       }
       
       
      j++;
    }

    if(curr_pos < 0) curr_pos = 0;
    n_tp3_tracks = (int)vec_trk_x->size();

    if(n_tp3_tracks > 0) nfound++;
    if(nfound%1000==0 && nfound > 0) cout << "Found " << nfound << endl;
    //if(nfound == 0) {

    //cout << "no tracks" << endl;

    //}


    t_out->Fill();
  }

  //fileout->Write();
  //fileout->Close();  

  //-------------- Do noise Plots --------------//
  //--------------------------------------------//


  f_noise->cd("TbUT");
  // f_noise->cd("/afs/cern.ch/user/c/cbetanco/work/LHCb/KeplerDev_v3r0/Tb/TbUT");
  TH2D *hNoise = (TH2D*)gDirectory->Get("CMSData_vs_channel");

  f_out->cd();

  TF1 *gau = new TF1("gau","gaus(0)",-120,120);
  gau->SetParameters(10000,0.0,40.0);

  int i = 128;
  for(int i=0;i<512;i++){
    if( i>=512 ) {
      hMeanNoise->SetBinContent(i,-1000);
      continue;
    }
    px[i] = hNoise->ProjectionY(Form("px%d",i),i,i);
    px[i]->GetXaxis()->SetRangeUser(-200,200);
    px[i]->SetTitle(Form("Channel %d", i));
    px[i]->SetName(Form("Ch%d", i));
    if(px[i]->GetEntries() < 10){
      hMeanNoise->SetBinContent(i,-1000);
      continue;
    }
    double p = px[i]->GetMaximum();
    double m = px[i]->GetMean();
    double e = px[i]->GetRMS();
    gau->SetParameters(p, m, e);
    gau->SetRange(m-3*e,m+3*e);

    px[i]->Fit(gau,"RQ");
    double mn = gau->GetParameter(1);
    double wid = gau->GetParameter(2);
    double emn = gau->GetParError(1);
    double ewid = gau->GetParError(2);
    hMeanNoise->SetBinContent(i,mn);
    hMeanNoise->SetBinError(i,emn);
    hWidthNoise->SetBinContent(i,wid);
    hWidthNoise->SetBinError(i,ewid);
  }

  // Save histograms
  hMeanNoise->Write();
  hWidthNoise->Write();
  for(int i=0;i<512;i++){
   px[i]->Write();
 }  
  
  t_out->AutoSave();
  delete f_ut;
  delete f_noise;  
  delete f_tpix;
  delete f_out;
  
  // Write and close ROOT file.
  //f_out->Write();
  //f_out->Close();

  return 0;
}
