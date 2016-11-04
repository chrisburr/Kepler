//#define AnalysisBase_cxx
#include "AnalysisBase.h"


AnalysisBase::AnalysisBase(TTree *tree) : fChain(0) 
{

// Notes
// For Rz alignment, one should SUBTRACT the slope found from the deltaX vs Y plot.
// For Z alignment, one should ADD the slope found from the deltaX vs Y plot.
// For Ry: If slope of DX vs X is negative (positive), reduce (increase) Ry
//
  //these have to be able to change so we have to make sure there is only 1 copy of them, so global scope would be bad news

  polarity = 1.0;
  if(isPType) polarity = -1;


  minDistFromHole =  m_minDistFromHole;
  holeSector = m_board.Contains("D") && (m_sector=="1" || m_sector=="2" || m_sector == "3");
  if(!holeSector) minDistFromHole = -9999;
  
  stripPitch = 0.190;
  z_DUT 			= 370; //This needs to be set for different boards
  Rz 				= -0.0218;
  Ry 				= 0.00;
  dxWin 			= 0.25;
  
  xGloOff 		= -9.2;
  yGloOff 		= -7.5;
  xOff 			= -54.78;
  correctForZRotation = true;
  
  channelOffset = 0.0;
  xLeftHole = 999;
  xRightHole = -999;
  stripGap[0] = 0;
  stripGap[1] = 0;
  stripGap[2] = 0;
  stripGap[3] = 0;
  nDeadRegion = 0;
  if(  m_board.Contains("D")) {
    stripPitch = 0.095;
    channelOffset = 512.0;
    //stripGap[0]=19;     
    //stripGap[1]=39;     
    //stripGap[2]=58;     
    //stripGap[3]=0;
    stripGap[0]=58;     
    stripGap[1]=39;     
    stripGap[2]=19;     
    stripGap[3]=0;
    if(m_board.Contains("D5")) {
      z_DUT = 1100.0;
      //Rz = -0.0230;  
    }else if(m_board.Contains("D7")) {
      z_DUT = 1123.0;
      if(m_sector=="5") z_DUT = 1087.0;
      if(m_sector=="1") z_DUT = 1040.0;
      //Ry = -0.008;  
    }
  }



  yInt1[0] = 2.8; yInt1[1] = 3.4;
  yInt2[0] = 3.4; yInt2[1] = 4.1;
  yInt3[0] = 1.0; yInt3[1] = 2.3;   

  if(m_board.Contains("A8")){
    z_DUT = 1100.0;
    Rz = -0.0286;
    yInt1[0] = 1.8; yInt1[1] = 2.0;
    yInt2[0] = 2.0; yInt2[1] = 2.4;
    yInt3[0] = 1.0; yInt3[1] = 2.3;
    if(m_sector=="3" || m_sector=="6") {
      channelOffset = -128.0*2+22;
    }else{
      channelOffset = -128.0;
    }
    if(m_scanType.Contains("Angle")){
      if(m_angle == "10") {
        Ry = 9.3*(TMath::Pi()/180.0);
        z_DUT = z_DUT - 18.0;
      } else if(m_angle == "20") {
        Ry = 19*(TMath::Pi()/180.0);
        z_DUT = z_DUT - 18.0;
      }      
     
    }    
  }else if( m_board.Contains("A4")){
    z_DUT = 381.0;     
    Rz = -0.0238;
    if(m_sector=="3" || m_sector=="6") {
      channelOffset = -128.0*2.0+19;
    }else{
      channelOffset = -128.0;
    }
  }else if( m_board.Contains("A6")){
    yInt1[0] = 2.5; yInt1[1] = 3.1;
    yInt2[0] = 3.1; yInt2[1] = 3.7;
    yInt3[0] = 1.0; yInt3[1] = 2.3;
    if(m_sector=="3" || m_sector=="6") {
      channelOffset = -128.0*2+22;
    }else{
      channelOffset = -128.0;
    }    
  }else if( m_board.Contains("A2") || m_board.Contains("A1")){
    z_DUT = 310;
    yInt1[0] = -5.0; yInt1[1] = -2.0;
    yInt2[0] = -2.0; yInt2[1] = 1.0;
    yInt3[0] = 1.0; yInt3[1] = 5.0;
  }else if( m_board.Contains("M1") || m_board.Contains("M2") || m_board.Contains("M3") ||   m_board.Contains("M4")){
    z_DUT = 343;
    polarity = 1.0; // all p-in-n
  }else if( m_board.Contains("F1") || m_board.Contains("F2") || m_board.Contains("F3") ||   m_board.Contains("F4")){
    z_DUT = 343;
    if(m_board.Contains("F1") || m_board.Contains("F2")) polarity = 1.0; // should be p-in-n
    if(m_board.Contains("F3") || m_board.Contains("F4")) polarity = -1.0;    // should be n-in-p
  }

  // Board A1 alignment is really messed up for a few runs! ADHOC here!
  if(m_board.Contains("A1")){// && (m_bias=="300" || m_bias=="350" || m_bias=="150" || m_bias==")){  
    dxWin = 1.0;
    correctForZRotation = false;
  }
   
  holeQuadPar[0] = 0;
  holeQuadPar[1] = 0;
  holeQuadPar[2] = 0;
  removeTracksInHole = removeTracksInHoleDef;
  // Only look to remove tacks from hole area if D-type and in sectors 1, 2, or 3.
  if(!m_board.Contains("D") || !(m_sector=="1" || m_sector=="2" || m_sector=="3") )  removeTracksInHole = false;

  for(int i=0; i<nChan; i++){badStrips[i] = 0;}
  nbadStrips = 0;



}

AnalysisBase::~AnalysisBase()
{
  if (!fChain) return;
  delete fChain->GetCurrentFile();
}

Int_t AnalysisBase::GetEntry(Long64_t entry)
{
  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}
Long64_t AnalysisBase::LoadTree(Long64_t entry)
{
  // Set the environment to read one entry
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
    Notify();
  }
  return centry;
}

void AnalysisBase::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  // It is normally not necessary to make changes to the generated
  // code, but the routine can be extended by the user if needed.
  // Init() will be called many times when running on PROOF
  // (once per file to be processed).

  // Set object pointer
  vec_trk_x = 0;
  vec_trk_y = 0;
  vec_trk_tx = 0;
  vec_trk_ty = 0;
  vec_trk_chi2ndf = 0;
  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);

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
  fChain->SetBranchAddress("n_tp3_tracks", &n_tp3_tracks, &b_n_tp3_tracks);
  fChain->SetBranchAddress("vec_trk_x", &vec_trk_x, &b_vec_trk_x);
  fChain->SetBranchAddress("vec_trk_y", &vec_trk_y, &b_vec_trk_y);
  fChain->SetBranchAddress("vec_trk_tx", &vec_trk_tx, &b_vec_trk_tx);
  fChain->SetBranchAddress("vec_trk_ty", &vec_trk_ty, &b_vec_trk_ty);
  fChain->SetBranchAddress("vec_trk_chi2ndf", &vec_trk_chi2ndf, &b_vec_trk_chi2ndf);
  fChain->SetBranchAddress("dtime", &dtime, &b_dtime);

  Notify();
}

Bool_t AnalysisBase::Notify()
{
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.

  return kTRUE;
}

void AnalysisBase::Show(Long64_t entry)
{
  // Print contents of entry.
  // If entry is not specified, print current entry
  if (!fChain) return;
  fChain->Show(entry);
}
/*
Int_t AnalysisBase::Cut(Long64_t entry)
{
  // This function may be called from Loop.
  // returns  1 if entry is accepted.
  // returns -1 otherwise.
  return 1;
}
*/
void AnalysisBase::getRange(TH1 *h, float &lo, float& hi, float thresh, int nSkipMax){

  int nfail = 0;
  int iMaxBin = h->GetMaximumBin();
  double maxValue;
  iMaxBin = h->GetMaximumBin();
  maxValue = h->GetBinContent(iMaxBin);
  lo = iMaxBin;
  hi = iMaxBin;
  
  // Only for debugging....
  //h->Draw();
  //std::cout << "maxValue = " << iMaxBin << "  " << maxValue << std::endl;
  

  for(int j=iMaxBin;j>=0;j--){
    double v = h->GetBinContent(j);
    double r = v / maxValue;
    //std::cout << "r = " << r << std::endl;  // Only for debugging
    if( r > thresh) {
      lo = j ;
      nfail = 0;
    }else if( r <= thresh) {
      nfail++;
    }
    if(nfail >= nSkipMax) break;
  }

  nfail = 0;
  for(int j=iMaxBin;j<=h->GetNbinsX();j++){
    double v = h->GetBinContent(j);
    double r = v / maxValue;
    //std::cout << "r = " << r << std::endl;  // Only for debugging
    if( r > thresh) {
      hi = j;
      nfail = 0;
    }else if( r <= thresh) {
      nfail++;
    }
    if(nfail >= nSkipMax) break;
  } 
  lo = h->GetBinCenter(lo);
  hi = h->GetBinCenter(hi);

  //std::cout << "lo, hi = " << lo << " " << hi << std::endl; // only for debugging
  

  return;
}


void AnalysisBase::getTDCBins(TProfile* h, float& lo, float& hi){
  getRange(h,lo,hi,0.97,1);
  lo = lo - 1.0 + 0.5;
  hi = hi + 0.5;
  return;
}



void AnalysisBase::getBeamLocation(TH1F *h, float &lo, float& hi){
  //double dif = 10;
  for(int  j = 0; j<10; j++){
    std::cout << "====> Finding Beam, iteration " << j+1 << std::endl;
    getRange(h,lo,hi,0.1,1);
    int dif = hi - lo + 1;
    if(dif < 10){
      int i1 = h->FindBin(lo);
      int i2 = h->FindBin(hi);
      for(int k=i1; k<=i2; k++){
        h->SetBinContent(k,0);
      }
    }else{
      break;
    } 
  }

  //h->Draw();
  return;
}

double AnalysisBase::getXOffset(TH1F *h1w){
  float xlo = 0, xhi=0;
  getRange(h1w,xlo,xhi,0.1,1);
  
  return (xlo+xhi)/2.0;

}

void AnalysisBase::getBeamLoc(){

  TH1F* ha = new TH1F("ha","Strip # of cluster with track",512,0.0,512);
  //TH1F* hb = new TH1F("hb","Strip # of cluster with track",512,0.0,512);
  Long64_t nentries = fChain->GetEntriesFast();
  float lo = 0, hi = 0;

  Long64_t nbytes = 0, nb = 0;
  std::cout << "======================================= " << std::endl;
  std::cout << "getBeamLoc(): Determining Beam Position " << nentries << std::endl;
  std::cout << "======================================= " << std::endl;
  for (Long64_t jentry=0; jentry<max(50000,(int)nentries);jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    //cout << clusterNumberPerEvent << endl;
    
    for(int j=0; j<min((int)clusterNumberPerEvent,10); j++){
      //std::cout << clustersPosition[j] << std::endl;
      //cout << polarity << " " << polarity*clustersCharge[j] << endl;
      if(polarity*clustersCharge[j] < kClusterChargeMin) continue;
      int iChan = clustersSeedPosition[j];
      //if(j<500) cout << "iChan = " << iChan << clustersCharge[j] << " " << 5*noise[iChan] << endl;
      if(polarity*clustersCharge[j]<4*noise[iChan]) continue;
      if(clustersPosition[j]>0.1&&clustersSize[j]==1) ha->Fill(clustersPosition[j]);
      if(clustersPosition[j]>0.1&&clustersSize[j]==2) ha->Fill(clustersPosition[j]);
    }
  }
     
  int num = ha->GetEntries();
  for(int i=0; i<1000; i++){
    if(num < 1000) continue;
    break;
  }

  if(num < 1000){
    std::cout << "ERROR: Something wrong here, insufficient entries in GetBeamLoc(), nEntries =  " << num << std::endl;
    //exit(1);
  }
   
  //ha->Draw();
  getBeamLocation(ha,lo,hi);
  iLo = lo;
  iHi = hi;
  std::cout << "====> Beam is between strips " << iLo << " -- " << iHi << std::endl;

  //delete ha;
  return;

}

void AnalysisBase::getTDC(){
  TProfile *hb = new TProfile("hb","Cluster Charge vs TDC time",12,0,12,100,1000);
  Long64_t nentries = fChain->GetEntriesFast();
  float lo = 0, hi = 0;

  Long64_t nbytes = 0, nb = 0;
  std::cout << "============================================" << std::endl;
  std::cout << "getTDC(): Determining Optimal TDC time range" << std::endl;
  std::cout << "============================================" << std::endl;   
  for (Long64_t jentry=0; jentry<max(50000,(int)nentries);jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;

    for(int j=0; j<min((int)clusterNumberPerEvent,10); j++){
      //std::cout << "j: " << ", clustersTDC: " << clustersTDC << ", clustersCharge[j]: " << clustersCharge[j] << ", clustersPosition[j]: " << clustersPosition[j] << std::endl; 
      if(clustersPosition[j] < 0.1) continue;
      if(polarity*clustersCharge[j] < kClusterChargeMin) continue;
      if(clustersPosition[j]>=iLo && clustersPosition[j]<=iHi && clustersTDC>1.0 && 
         polarity*clustersCharge[j]>150 && polarity*clustersCharge[j]<500) hb->Fill(clustersTDC+0.1,polarity*clustersCharge[j]);
    }
  }

  getTDCBins(hb,lo,hi);
  tdcLo = lo;
  tdcHi = hi;
  std::cout << "====> TDC Range determined to be from " << tdcLo << " -- " << tdcHi << std::endl;
  hb->Draw();
  //delete hb;
  

  return;
}

void AnalysisBase::findBeamRegionAndAlign(int iPass){

  cout << "==================================================================" << endl;
  cout << "findBeamRegionAndAlign(): Determining Fiducial Region, Pass = " << iPass << endl;
  cout << "==================================================================" << endl;

  double dxWindow = dxWin;
  if(iPass == 1) dxWindow = 1000.0;
  if(iPass == 2) dxWindow = 1.0;

  TH1F* hthx = new TH1F("hthx","#theta_{X}",1000,-5.0,5.0);
  TH1F* hthy = new TH1F("hthy","#theta_{Y}",1000,-5.0,5.0);
  TH1F* hx = new TH1F("hx","Y position of matched cluster",800,-40.0,40.0);
  TH1F* hxdut = new TH1F("hxdut","Y position of matched cluster",800,-40.0,40.0);
  TH1F* hs = new TH1F("hs","Strip number of matched cluster",512,0,512.0);
  TH1F* hy = new TH1F("hy","Y position of matched cluster",800,-10.0,10.0);
  TH1F* hw = new TH1F("hw","#DeltaX",20000,-100.0,100.0);
  TH1F* hn = new TH1F("hn","#DeltaX",4000,-2.0,2.0);
  TH1F* hnn = new TH1F("hnn","#DeltaX",500,-0.5,0.5);
  TH2F* hxy = new TH2F("hxy","Y_{trk} vs X_{trk}, with cluster",640,-8,8.0,640,-8,8);
  TProfile *ht = new TProfile("ht","Cluster Charge vs TDC time",12,0,12,100,1000);
  TProfile *hzrot = new TProfile("hzrot","#DeltaX vs Y_{trk} at DUT",1600,-8,8,-1.0,1.0);
  TH2F* hca = new TH2F("hca","Y_{trk} vs X_{trk}, with found cluster",160,-8,8.0,320,-8,8);
  TH2F* hcf = new TH2F("hcf","Y_{trk} vs X_{trk}, with found cluster",160,-8,8.0,320,-8,8);
  TProfile  *hdxvsthx= new TProfile("hdxvsthx","#DeltaX vs #theta_{trk}",100,-5,5,-1.0,1.0);
  
  hca->Sumw2();
  hcf->Sumw2();
  

  double nomStrip=0, detStrip = 0;
  Long64_t nbytes = 0, nb = 0;
  Int_t nentries = fChain->GetEntriesFast();
  for (Long64_t jentry=0; jentry<max(nentries,200000);jentry++) {
    Long64_t ientry = LoadTree(jentry);
    //if(jentry%1000==0) cout << "At entry = " << jentry << endl;
    
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    if(n_tp3_tracks > 1) continue;      

    for(int k=0; k<n_tp3_tracks; k++){
      bool goodTime = (clustersTDC >= tdcLo && clustersTDC < tdcHi);
      goodTime = clustersTDC>1.0;
      if(!goodTime) continue;
      double x_trk = vec_trk_tx->at(k)*z_DUT+vec_trk_x->at(k);
      double y_trk = vec_trk_ty->at(k)*z_DUT+vec_trk_y->at(k);

      transformTrackToDUTFrame(k, x_trk, y_trk, nomStrip, detStrip);

      double tx = 1000*vec_trk_tx->at(k);
      double ty = 1000*vec_trk_ty->at(k);

      double x_trk0 = x_trk;

      for(int j=0; j<min(clusterNumberPerEvent,10); j++){
        if(clustersPosition[j] < 0.1) continue;
        if(polarity*clustersCharge[j] < kClusterChargeMin) continue;
        bool goodHit = (clustersPosition[j]>iLo && clustersPosition[j]<iHi);
        double x_dut = getDUTHitPosition(j);

        x_trk = x_trk0;

        double dx = x_dut - x_trk;
        hn->Fill(dx);
        hnn->Fill(dx);

        hca->Fill(x_trk,y_trk);
        if(fabs(dx)<dxWindow || iPass==1) {
          if(goodHit) {
            hx->Fill(x_trk);
            hxdut->Fill(x_dut);
            hs->Fill(clustersPosition[j]);
            hy->Fill(y_trk);
            hthx->Fill(tx);
            hthy->Fill(ty);
            hw->Fill(dx);     
            hxy->Fill(x_trk,y_trk);
            ht->Fill(clustersTDC+0.1,polarity*clustersCharge[j]);
            hzrot->Fill(y_trk,dx);
            hcf->Fill(x_trk,y_trk);
            hdxvsthx->Fill(tx,dx);
          } 
        }
      }
    }
  }

  //hnn->Draw();

  if(iPass==-1) {
    findCutoutRegion(hcf);
  }else {  

    float xmin=0,xmax=0,ymin=0,ymax=0,txmin=0,txmax=0,tymin=0,tymax=0;

    getRange(hthx,txmin,txmax,0.05,2);
    getRange(hthy,tymin,tymax,0.05,2);
    getRange(hx,xmin,xmax,0.05,2);
    getRange(hy,ymin,ymax,0.2,2);
    
    xMin = xmin;
    xMax = xmax;
    yMin = ymin;
    yMax = ymax;
    txMin = txmin;
    txMax = txmax;
    tyMin = tymin;
    tyMax = tymax;
    
    float xlo = 0, xhi=0;
    if(iPass==1) getRange(hw,xlo,xhi,0.1,1);
    if(iPass==2) getRange(hn,xlo,xhi,0.1,1);
    if(iPass==3) getRange(hn,xlo,xhi,0.2,1);
    if(iPass==4) getRange(hnn,xlo,xhi,0.2,1);
    double XOFF = (xhi + xlo)/2.0;
    xOff = xOff - XOFF;
    double x_ave = (xMax + xMin)/2.0;
    double y_ave = (yMax + yMin)/2.0;
    xGloOff = xGloOff - x_ave;
    yGloOff = yGloOff - y_ave;
    
    // Check/correct for z rotation
    if(iPass==3 && correctForZRotation){
      cout << "Checking for z-rotation" << endl;
      TF1* p1 = new TF1("p1","[0]+[1]*x",yMin,yMax);
      p1->SetParameters(0.0,0.0001);
      hzrot->Fit(p1,"0R");
      double rz = p1->GetParameter(1);
      cout << "=======================================================================" << endl;
      cout << "==> Updating z rotation angle from " << Rz << " to " << Rz-rz << " mrad" << endl;    
      Rz = Rz - rz;
      delete p1;
    }
    
    
    float lo = 0, hi = 0;
    getTDCBins(ht,lo,hi);
    tdcLo = lo;
    tdcHi = hi;
    cout << "=================================================================" << endl;
    cout << "====> TDC Range updated to be from " << tdcLo << " -- " << tdcHi << std::endl;
    cout << "=================================================================" << endl;
    cout << "====> Fiducial regions, xLo, xHi (Strip numbers) = " << iLo << " " << iHi << endl;
    cout << "====> Fiducial regions, xLo, xHi (pos in mm) = " << xMin << " " << xMax << " mm " << endl;
    cout << "====> Fiducial regions, yLo, yHi (pos in mm) = " << yMin << " " << yMax << " mm " << endl;
    cout << "====> Fiducial regions, thxLo, thxHi (pos in mrad) = " << txMin << " " << txMax << " mrad" << endl;
    cout << "====> Fiducial regions, thyLo, thxHi (pos in mrad) = " << tyMin << " " << tyMax << " mrad" << endl;
    cout << "=================================================================" << endl;   
    cout << "====> Shifting sensor by dX = " << XOFF << " mm " << ", new xOff = " << xOff << " " << xlo << " " << xhi << endl;
    cout << "====> Global X, Y shifts: " << x_ave << " " << y_ave << endl;
    cout << "====> New global x,y = " << xGloOff << " " << yGloOff << endl;
    
  }
  
  //if(iPass==1) hw->Draw();  // for debugging   
  //if(iPass==2) hy->Draw();

  //return;
  

  delete hxdut;
  delete hthx;
  delete hthy;
  delete hx;
  delete hy;
  delete hxy;
  delete hw;
  delete hn;
  delete hnn;
  delete hs;
  delete ht;
  delete hzrot;
  delete hcf;
  delete hca;
  delete hdxvsthx;

   
  return;
  

}


Double_t AnalysisBase::getCorrChannel(double ch){
  //if(ch>=128 && ch<=255) return (ch + stripGap[0]);
  //if(ch>=256 && ch<=383) return (ch + stripGap[1]);
  //if(ch>=384 && ch<=512) return (ch + stripGap[2]);

  if(ch<=128) return (ch - stripGap[0]);
  if(ch>=128 && ch<=255) return (ch - stripGap[1]);
  if(ch>=256 && ch<=383) return (ch - stripGap[2]);
  if(ch>=384 && ch<=512) return (ch - stripGap[3]);

  return ch;  
}

void AnalysisBase::findChipBoundary(){
  // Look for holes in hit profile, due to boundary between chips
  std::cout << "=======================================================================================" << std::endl;
  std::cout << "findChipBoundary(): Looking for gaps due to unconnected strips in between Beetle chips " << std::endl;
  std::cout << "=======================================================================================" << std::endl;
   
  TH1F* hx2 = new TH1F("hx2","X position of cluster",200,-10.0,10.0);

  Long64_t nbytes = 0, nb = 0;
  Long64_t nentries = fChain->GetEntriesFast();
  for (Long64_t jentry=0; jentry<max((int)nentries,20000);jentry++) {
    Long64_t ientry = LoadTree(jentry);    
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    for(int j=0; j<min((int)clusterNumberPerEvent,10); j++){
      if(clustersPosition[j] < 0.1) continue;
      if(polarity*clustersCharge[j] < kClusterChargeMin) continue;
      bool goodHit = (clustersPosition[j]>iLo && clustersPosition[j]<iHi);
      goodHit = goodHit &&  (clustersTDC >= tdcLo && clustersTDC <= tdcHi);

      double x_dut = getDUTHitPosition(j);
         
      hx2->Fill(x_dut);
    }
  }
  
  //hx2->Draw(); // for debugging only

  int i1 = hx2->FindBin(xMin);
  int i2 = hx2->FindBin(xMax);
  double ave = 0.0;
  int ixL=i1, ixH=i2;
  for(int i=i1; i<i2; i++){
    ave = ave +  hx2->GetBinContent(i);
  }
  ave = ave / (i2 - i1 + 1);
  for(int i=i1; i<i2; i++){
    double v  = hx2->GetBinContent(i);
    double xx  = hx2->GetBinCenter(i);
    double r = v/ave;
    if(r < ratioForHole && xx < xLeftHole) {
      xLeftHole = xx;
      ixL = i;
    }
    if(r < ratioForHole && xx > xRightHole) {
      xRightHole = xx;
      ixH = i;
    }
  }
  // Count the number of "dead" channels in this region
  float ndead = 0;
  for(int i=ixL; i<=ixH; i++){
    double v  = hx2->GetBinContent(i);
    double r = v/ave;
    if(r < ratioForHole) ndead = ndead + 1;
  }
  double frdead = ndead / (ixH - ixL + 1);

  if(frdead>0.8 && ndead>4){
    xLeftHole = xLeftHole - 0.2;
    xRightHole = xRightHole + 0.2;
  }else{
    xLeftHole = 999;
    xRightHole = -999;
  }
  
  hx2->Draw();
  
  
  //std::cout << "Xmin, xMax = " << xMin << " " << xMax << std::endl;
  //std::cout << "Bin Edges: " << i1 << "  " << i2 << std::endl;
  //std::cout << "Hole position: " << xLeftHole << "  " << xRightHole << " , fraction dead = " << ndead << " " << frdead << std::endl;
  
  delete hx2;

}

void AnalysisBase::findDeadRegions(){

  cout << "=================================================" << endl;
  cout << "findDeadRegions(): Looking for dead strip regions " << endl;
  cout << "=================================================" << endl;

  TH1F* hf = new TH1F("hf","X position of matched cluster",200,-10.0,10.0);
  TH1F* hnf = new TH1F("hnf","X position of matched cluster",200,-10.0,10.0);
  
  double nomStrip = 0, detStrip = 0;
  Long64_t nbytes = 0, nb = 0;
  Int_t nentries = fChain->GetEntriesFast();
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
      
    if(n_tp3_tracks > 1) continue;

    // Loop over TPIX tracks in event
    for(int k=0; k<n_tp3_tracks; k++){
      double x_trk = vec_trk_tx->at(k)*z_DUT+vec_trk_x->at(k);
      double y_trk = vec_trk_ty->at(k)*z_DUT+vec_trk_y->at(k);

      transformTrackToDUTFrame(k, x_trk, y_trk, nomStrip, detStrip );

      bool goodTrack = false;
      bool inFiducial = false;
      if(x_trk>xMin && x_trk<xMax && y_trk>yMin && y_trk<yMax) inFiducial = true;          
      inFiducial = inFiducial && (x_trk<xLeftHole || x_trk>xRightHole);
      double tx = 1000*vec_trk_tx->at(k);
      double ty = 1000*vec_trk_ty->at(k);
      if(tx>txMin && tx<txMax && ty>tyMin && ty<tyMax) goodTrack = true;        
      bool goodTime =  (clustersTDC >= tdcLo && clustersTDC <= tdcHi);

      bool foundHit = false;
      // Loop over clusters
      for(int j=0; j<min(clusterNumberPerEvent,10); j++){
        if(clustersPosition[j] < 0.1) continue;
        if(polarity*clustersCharge[j] < kClusterChargeMin) continue;
        //bool goodHit = (clustersPosition[j]>iLo || clustersPosition[j]<iHi);
        double x_dut = getDUTHitPosition(j);

        double dx = x_dut - x_trk;

        if(goodTrack && inFiducial && goodTime && fabs(dx)<dxWin) {
          foundHit = true;
        }   
      }
      if(inFiducial && goodTrack && goodTime) {
        hf->Fill(x_trk);    
        if(!foundHit) hnf->Fill(x_trk);          
      }
        
    }
  }

  TH1F *hineff = (TH1F*)hnf->Clone("hineff");
  hineff->Divide(hf);

  // Only for debugging
  /*
    TCanvas *cc = new TCanvas("cc","Hits",800,800);
    cc->Divide(2,2);
   
    cc->cd(1);
    hnf->Draw();
    cc->cd(2);
    hf->Draw();
    cc->cd(3);
    hineff->Draw();
  */


  for(int k = 0; k<hineff->GetNbinsX()-1; k++){
    if(nDeadRegion > 20){
      cout << "WARNING: Exceeded 20 dead regions, exiting from loop" << endl;       
      break;
    }     
    double v1 = hineff->GetBinContent(k);
    if(v1 > k_DeadStrip){
      deadRegionLo[nDeadRegion] = hf->GetBinLowEdge(k);
      for(int j = k; j<hineff->GetNbinsX(); j++){
        double v2 = hineff->GetBinContent(j);
        if(v2 < k_DeadStrip){
          deadRegionHi[nDeadRegion] = hf->GetBinLowEdge(j);
          cout << "====> Found inefficiency strip region from x :    " 
               << deadRegionLo[nDeadRegion] << " <===> " << deadRegionHi[nDeadRegion] << " mm" << endl;
          k = j + 1;
          nDeadRegion++;
          break;
        }    
      }
    }
  }
   
  delete hf;
  delete hnf;
  delete hineff;
   
  return;
}

void AnalysisBase::transformTrackToDUTFrame(int k, double& x_trk, double& y_trk, double& elecStrip, double& detStrip){
  y_trk = y_trk + yGloOff;

  double dzs = x_trk * sin(Ry);
  x_trk = x_trk - Rz*y_trk;
  y_trk = y_trk + Rz*x_trk;
  x_trk = x_trk + vec_trk_tx->at(k)*dzs;
  x_trk = x_trk/cos(Ry);
  double rStrip = nChan - (x_trk-xOff)/stripPitch; 
  double corStrip = getCorrChannel(rStrip);
  //double dx = (corStrip - rStrip)*stripPitch;
  //x_trk = x_trk + dx;
  detStrip = rStrip; //nChan - (x_trk-xOff)/stripPitch; 
  elecStrip = corStrip;
  
  x_trk = x_trk + xGloOff;

  return;
}

void AnalysisBase::PrepareDUT(){

   //----------------------------
   // Get Beam Location (Strips)
   //----------------------------
   getBeamLoc();
   //return;
   
   
   // ==================================
   // Determining Optimal TDC time range
   // ==================================
   getTDC();
   
   // --------------------------------------------------------------
   // Find beam fiducial region (slopes & Y range) & align residuals
   // ---------------------------------------------------------------
   findBeamRegionAndAlign(1);
   //return;
   
   
   findBeamRegionAndAlign(2);   
   yMid = yMin + (yMax-yMin)/2.0;
   yHi2 = yMax - 2.0;
   //return;
   

   findBeamRegionAndAlign(3);   
   yMid = yMin + (yMax-yMin)/2.0;
   yHi2 = yMax - 2.0;
   //return;
   
   if(yInt2[1] > yMax){
     yInt1[0] = yMax-0.8; yInt1[1] = yMax-0.4;
     yInt2[0] = yMax-0.4; yInt2[1] = yMax;
     yInt3[0] = yMin; yInt3[1] = yMax-0.8;
   }
   cout << "Y Region Definitions:" << endl;
   cout << " ===> Region 1: " << yInt1[0] << " -- " << yInt1[1] << " mm" << endl;
   cout << " ===> Region 2: " << yInt2[0] << " -- " << yInt2[1] << " mm" << endl;
   cout << " ===> Region 3: " << yInt3[0] << " -- " << yInt3[1] << " mm" << endl;
   
   //return;
   // Correct for gaps between Beetle chips
   correctForStripGaps();

   findBeamRegionAndAlign(4);   
   if(holeSector) findBeamRegionAndAlign(4);

   findChipBoundary();
   cout << "====> Hole position: " << xLeftHole << "  " << xRightHole << endl;
   //return;

   if(vetoDeadStripRegions) findDeadRegions();
   //return;

   setCrossTalkCorr();

}

Double_t AnalysisBase::getDUTHitPosition(int j){
  double pos = getCorrChannel(clustersPosition[j]);
  //double pos = clustersPosition[j];
  double x_dut = (nChan - pos)*stripPitch + xOff;
  x_dut = x_dut + xGloOff;  
  return x_dut;
  
}

void AnalysisBase::setCrossTalkCorr(){
  float biasVal = atof(m_bias);
  chargeCorrSlopeOdd = 0.0;
  chargeCorrSlopeEven = 0.0;
  cout << "Bias Value = " << m_bias <<  " " << biasVal << endl;
  
  if(m_board.Contains("A2") && m_sector=="1" && biasVal < 260){
    chargeCorrSlopeOdd = 0.435;
    chargeCorrSlopeEven = 0.400;
  }else if(m_board.Contains("A2") && m_sector=="1" && biasVal > 320){
    chargeCorrSlopeOdd = 0.148;
    chargeCorrSlopeEven = 0.094;
  }else if(m_board.Contains("A2") && m_sector=="1" && biasVal == 300){
    chargeCorrSlopeOdd = 0.130;
    chargeCorrSlopeEven = 0.088;
  }else if(m_board.Contains("A2") && m_sector=="2"){
    chargeCorrSlopeOdd = 0.120;
    chargeCorrSlopeEven = 0.085;
  }
  
}


void AnalysisBase::findCutoutRegion(TH2F* h){

  cout << "===============================================================" << endl;
  cout << "findCutoutRegion(): Looking for cutout region in D type sensor " << endl;
  cout << "===============================================================" << endl;


  int ilx = h->GetXaxis()->FindBin(xMin) + 1;
  int ihx = h->GetXaxis()->FindBin(xMax) - 1;
  int ily = 1;//h->GetYaxis()->GetBinFindBin(yMin) - 10;
  int ihy = h->GetYaxis()->FindBin(yMax) + 10;
  if(ily < 0) ily = 0;
  if(ihy > h->GetYaxis()->GetNbins()) ihy = h->GetYaxis()->GetNbins();  
  int nb = h->GetXaxis()->GetNbins(); 

  double xlow = h->GetXaxis()->GetBinLowEdge(1);
  double xhi = h->GetXaxis()->GetBinLowEdge(nb)+h->GetXaxis()->GetBinWidth(1);

  TH1F *hpr = new TH1F("hpr","Y vs X, edge",nb,xlow,xhi);

  TH1D *hpy;
  int ipeak = 0;
  for(int i=ilx; i<ihx-1; i++){
    hpy = h->ProjectionY("hpy",i,i);
    double maxcon  = 0.0;
    double maxbin  = 0.0;
    for(int j=ily;j<ihy;j++){
      maxcon = maxcon + hpy->GetBinContent(j);
      if(hpy->GetBinContent(j) > maxbin) {
        maxbin = hpy->GetBinContent(j) ;
        ipeak = j;
      }
    }
    // Find the lower "edge"
    for(int j=ipeak; j>=ily; j--){
      double r0 = (hpy->GetBinContent(j-1)+hpy->GetBinContent(j-2))/2.0;
      double r1 = hpy->GetBinContent(j);
      double r2 = (hpy->GetBinContent(j+3)+hpy->GetBinContent(j+4))/2.0;
      if(r1<=2 && r0<=1.5 && r2>5*r1 && r2>8){
        hpr->SetBinContent(i,hpy->GetBinCenter(j)+2*hpy->GetBinWidth(j));
        hpr->SetBinError(i,hpy->GetBinWidth(1)/2.0);
        //cout << "found: " << i << " " << hpy->GetEntries() << " " << hpy->GetBinCenter(j) << " " 
        //     << hpy->GetBinContent(j) << " " << r0 << " " << r1 << " " << r2 << " " << endl;
        break;
      }
    }
    
  }

  TF1* poly2 = new TF1("poly2","[0]+[1]*x+[2]*x*x",xMin,xMax);
  poly2->SetParameters(-1.5,-0.17,-0.15);
  hpr->Fit(poly2,"R0");
  hpr->SetLineColor(kRed);

  holeQuadPar[0] = poly2->GetParameter(0);
  holeQuadPar[1] = poly2->GetParameter(1);
  holeQuadPar[2] = poly2->GetParameter(2);

  //return;
  

  delete poly2;
  delete hpr;
  delete hpy;
  
  return;
}

bool AnalysisBase::isInCutoutRegion(double xtrk, double ytrk){  
  // Some protections here..
  if(!removeTracksInHole) return false;
  if(holeQuadPar[0]==0 || holeQuadPar[1]==0 || holeQuadPar[2]==0) return false;
  // Ok, looks like we mean to really remove these tracks
  double yhole = holeQuadPar[0]+holeQuadPar[1]*xtrk+holeQuadPar[2]*xtrk*xtrk;
  if(ytrk < yhole + minDistFromHole) return true;
  return false;
}

double AnalysisBase::DistToCutoutRegion(double xtrk, double ytrk){  
  // Some protections here..
  if(holeQuadPar[0]==0 || holeQuadPar[1]==0 || holeQuadPar[2]==0) return 999.0;
  // Ok, looks like we mean to really remove these tracks
  double yhole = holeQuadPar[0]+holeQuadPar[1]*xtrk+holeQuadPar[2]*xtrk*xtrk;
  double a = 2.0*holeQuadPar[2]*xtrk+holeQuadPar[1];
  double c = yhole - a*xtrk;
  double b = -1.0;
  
  double dist = fabs(a*xtrk + b*ytrk + c) / sqrt(a*a+b*b);
  if(ytrk < yhole) dist = -1.0*dist;
  return dist;
}

void AnalysisBase::correctForStripGaps(){   
  cout << "================================================================================" << endl;
  cout << "correctForStripGaps(): Correcting x fiducial range for gaps between Beetle chips" << endl;
  cout << "================================================================================" << endl;

  double x1 = getCorrChannel(iLo);
  double x2 = getCorrChannel(iHi);
  xMax = (nChan - x1)*stripPitch + xOff + xGloOff;
  xMin = (nChan - x2)*stripPitch + xOff + xGloOff;
  double xave = (xMax + xMin)/2.;
  xGloOff = xGloOff - xave;
  xMin = xMin - xave;
  xMax = xMax - xave;   
  std::cout << "====> Updating Global Offset to use Strips, *** New xMin, xMax == " << xMin << " " << xMax << endl;
  std::cout << "====> xMin, xMax = " << xMin << " " << xMax << endl;
  std::cout << "====> xMin, xMax = " << yMin << " " << yMax << endl;
  return;
  
}


