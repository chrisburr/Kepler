#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "RooLandau.h"
#include "RooFFTConvPdf.h"
#include "RooPlot.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TH1.h"
#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     /* atoi */
#include <sys/types.h>
#include <sys/stat.h>

#include "AnalysisBase_Inputs.h"

using namespace RooFit ;
using namespace std;

void getRange(TH1 *h, float &lo, float& hi, float thresh, int nSkipMax){

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

void setFrameAtt(RooPlot *frame, TString xtit, TString ytit)
{
  frame->SetMaximum(1.5*frame->GetMaximum());
  //frame->SetMaximum(35);
  frame->SetMinimum(0.1);    
  frame->SetLabelSize(0.04,"X");
  frame->SetLabelSize(0.04,"Y");
  frame->SetTitleSize(0.055,"X");
  frame->SetTitleSize(0.055,"Y");
  frame->SetTitleOffset(1.0,"X");
  frame->SetTitleOffset(1.1,"Y");
  frame->SetXTitle(xtit);
  frame->SetYTitle(ytit);
  frame->SetNdivisions(505,"X");
  frame->SetNdivisions(505,"Y");
  //frame->GetXaxis()->SetTitleFont(myconfigs->m_font);
  //frame->GetYaxis()->SetTitleFont(myconfigs->m_font);
}

RooPlot* fitLandau(TH1F *hlan1){

  RooRealVar t("Charge (ADC)","Charge (ADC)",0,1000) ;
  //t.setBins(10000,"cache") ; 

  TCanvas* cc = new TCanvas("Landau_Gauss","Landau x Gauss Fit",600,600) ;
  double wid = 20;

  // Construct landau(t,ml,sl) ;
  RooRealVar ml("MPV","mean landau",250.,50,500) ;
  RooRealVar sl("Width","sigma landau",wid,0.1,80) ;
  RooLandau landau("lx","lx",t,ml,sl) ;
  
  // Construct gauss(t,mg,sg)
  RooRealVar mg("mg","mg",0) ;
  RooRealVar sg("#sigma_{noise}","sg",30,0.2,150.0) ;
  RooGaussian gauss("gauss","gauss",t,mg,sg) ;

  // Construct landau (x) gauss
  RooFFTConvPdf lxg("lxg","landau (X) gauss",t,landau,gauss) ;

  TF1 *gau = new TF1("gau","gaus(0)",-80,80);
  gau->SetParameters(1000,0,30);

   double mv = hlan1->GetMean();
   ml.setVal(0.75*mv);
   int nStat = hlan1->GetEntries();
   
   RooDataHist* data1 = new RooDataHist("data1","", t, hlan1);
   int ip = hlan1->GetMaximumBin();
   double mn = hlan1->GetBinCenter(ip);
   ml.setVal(mn);
   sg.setVal(40.0);
   sl.setVal(0.07*mn);
   cout << "RMS = " << hlan1->GetRMS() << endl;

   t.setRange("Range1",200,1000);
   lxg.fitTo(*data1,Range("Range1"),Strategy(1)) ;
   RooPlot* frame = t.frame(Title("Cluster Charge")) ;;
    data1->plotOn(frame,MarkerSize(0.7)) ;
   gPad->SetLeftMargin(0.13) ;  gPad->SetRightMargin(0.01) ;  gPad->SetBottomMargin(0.13) ; frame->GetYaxis()->SetTitleOffset(1.3) ; frame->GetXaxis()->SetTitleOffset(0.8) ; 
    
    lxg.plotOn(frame,LineWidth(2)) ;
    setFrameAtt(frame,"Charge (ADC)","Events / 2 ADC");  
    lxg.paramOn(frame,Layout(0.45,0.88,0.89));
    frame->Draw() ;
    //hcl->Draw("same");

    delete cc;
    return frame;


}


void addGraphics(TH1 *h, int iCol = 1, TString XTitle="", TString YTitle="")
{
  //float bw = h->GetBinWidth(1);
  h->SetXTitle(XTitle);
  h->SetYTitle(YTitle);
  h->SetStats(kFALSE);
  h->SetLineColor(iCol);
  h->SetMarkerColor(iCol);
  h->SetMarkerSize(0.7);
  h->SetMinimum(0.0);
  h->SetMaximum(1.2*h->GetMaximum());
  h->SetTitleSize(0.1);
  
  //h->SetLineColor(kBlack);
  //h->SetMarkerSize(0.7);
  //h->SetMarkerStyle(20);
  h->GetXaxis()->SetTitleOffset(1.0);  
  h->GetYaxis()->SetTitleOffset(1.1);
  h->GetXaxis()->SetTitleSize(0.04);  
  h->GetYaxis()->SetTitleSize(0.04);
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
  h->GetXaxis()->SetTitleSize(0.05);  
  h->GetYaxis()->SetTitleSize(0.05);
  h->GetXaxis()->SetLabelSize(0.05);  
  h->GetYaxis()->SetLabelSize(0.05);  
  h->SetNdivisions(505,"X");
  h->SetNdivisions(505,"Y");
  h->SetLineWidth(2);
}

void addGraphics(TProfile *h, int iCol = 1, TString XTitle="", TString YTitle="")
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
  h->SetMarkerSize(0.7);

  //h->SetLineColor(kBlack);
  //h->SetMarkerSize(0.7);
  //h->SetMarkerStyle(20);
  h->GetXaxis()->SetTitleOffset(1.0);  
  h->GetYaxis()->SetTitleOffset(1.2);
  h->GetXaxis()->SetTitleSize(0.05);  
  h->GetYaxis()->SetTitleSize(0.05);
  h->GetXaxis()->SetLabelSize(0.05);  
  h->GetYaxis()->SetLabelSize(0.05);  
  h->SetNdivisions(505,"X");
  h->SetNdivisions(505,"Y");
  h->SetLineWidth(2);
}


void monitorPlots(TString theBoard="M1", TString theSector="1", TString theBias="300"){

  TString board = theBoard;
  TString bias = theBias;
  TString sector = theSector;

  double xMin = -5.0;
  double xMax = 5.0;
  double yMin = -5.0;
  double yMax = 5.0;
  
  gStyle->SetStatW(0.4);
  
  TString suffix =  "_"+board+"_"+bias+"_"+sector;

  TString indir =  m_fileOutdir + plotdir ;
  TString outdir =  m_fileOutdir + plotdir + "plots/";
  TString filename = indir+"AnalysisOutput"+suffix+".root";
  TFile *f = new TFile(filename);
  TH1F *hcAll = (TH1F*)f->Get("hcAll");
  TH1F *hcTrk = (TH1F*)f->Get("hcTrk");
  TH1F *h1 = (TH1F*)f->Get("h1");
  TH2F *h1vsx = (TH2F*)f->Get("h1vsx");

  hcAll->SetTitle("");
  h1vsx->SetTitle("");
  h1->SetTitle("");

  int iy1 = h12a->GetXaxis()->GetFirst();
  int iy2 = h12a->GetXaxis()->GetLast();
  int ix1 = h12dd->GetXaxis()->GetFirst();
  int ix2 = h12dd->GetXaxis()->GetLast();
  yMin = h12a->GetXaxis()->GetBinLowEdge(iy1);
  yMax = h12a->GetXaxis()->GetBinUpEdge(iy2);
  xMin = h12dd->GetXaxis()->GetBinLowEdge(ix1);
  xMax = h12dd->GetXaxis()->GetBinUpEdge(ix2);
  
  

  TProfile *hch = (TProfile*)f->Get("h12m");
  TH1F *heff = (TH1F*)f->Get("hepas1");

  //--------------------------
  // Begin alignment plots
  //--------------------------
  int k = 1;
  TCanvas *c0 = new TCanvas("c0","Alignment Plots",1400,700);
  c0->Divide(4,2);
  
  c0->cd(k++);
  //h1->GetXaxis()->SetRangeUser(-0.2,0.2);
  h1->SetTitle("Residual Distribution");
  h1->Draw();   
  h1a->Draw("same");
  h1b->Draw("same");
  TLegend* legend3 = new TLegend(0.18,0.70,0.94,0.88);
  legend3->SetFillStyle(0);
  legend3->SetBorderSize(0);
  legend3->SetFillStyle(0);
  legend3->SetTextSize(0.045);
  legend3->AddEntry(h1,"All clusters","L"); 
  legend3->AddEntry(h1a,"1-strip clusters","L"); 
  legend3->AddEntry(h1b,"2-strip clusters","L"); 
  legend3->Draw();

  c0->cd(k++);
  h2->Draw("colz");
  c0->cd(k++);
  h8->Draw();
  c0->cd(k++);
  h9->Draw();
  c0->cd(k++);
  h9a->Draw();
  c0->cd(k++);
  h15a->GetXaxis()->SetRangeUser(0,50);
  h15a->Draw();
  c0->cd(k++);
  h2p->Draw();


  c0->cd(k++);
  addGraphics(h15c,1,"time(DUT) - time(track) [ns]","(x10^{3})");
  h15c->Scale(0.001);
  h15c->Draw();

   c0->Print(outdir+"/AlignmentPlots_"+board+"_s"+sector+"_vb"+bias+".png");
   c0->Print(outdir+"/AlignmentPlots_"+board+"_s"+sector+"_vb"+bias+".pdf");

  // end of alignment plots



  float lo, hi;
  getRange(h4c,lo,hi,0.1,1);
  int ilo = lo - 5;
  int ihi = hi + 5;
  cout << lo << " " << hi << endl;
  
  TH1F * hn0 = (TH1F*)f->Get("hnoisePerChannel");
  hn0->SetLineColor(2);
  double mhn0 = hn0->Integral(ilo,ihi);
  double mh4c = h4c->Integral(ilo,ihi);
  double ave = mhn0/(hi-lo);
  //hn0->SetMinimum(0);
  hn0->GetYaxis()->SetRangeUser(0,1.0*ave);
  
  h4c->Scale(1.*mhn0/mh4c);
  h4c->SetLineWidth(2);
  hn0->SetTitle("Beam profile (blue) and noise (red)");
  hn0->SetStats(kFALSE);
  hn0->GetXaxis()->SetRangeUser(ilo,ihi);

  


  //return;
  
  //--------------------------
  // Begin performance plots
  //--------------------------
  RooPlot* lfit = fitLandau(hcTrk);

  TCanvas *c1 = new TCanvas("c1","Performance Plots",1100,1100);
  c1->Divide(3,4);
  int k=1;
  c1->cd(k)->SetBottomMargin(0.13);
  c1->cd(k++)->SetLeftMargin(0.13);
  //TCanvas *cn = new TCanvas("cn");
  lfit->Draw();
  
  //return;
  hcAll->SetMaximum(1.4*hcTrk->GetMaximum());  
  hcAll->SetLineColor(2);
  hcTrk->SetLineColor(1);
  hcAll->Draw("same");
  hcTrk->Draw("same"); 
  //legend2->AddEntry(hcAll,"All clusters","L"); 
  //legend2->AddEntry(hcTrk,"Clusters with Track","L");
  //legend2->Draw();

  TLatex *myLatex = new TLatex();
  myLatex->SetTextFont(42); myLatex->SetTextColor(kRed); myLatex->SetTextAlign(12); 
  myLatex->SetNDC(kTRUE); myLatex->SetTextSize(0.055);
  myLatex->DrawLatex(0.63, 0.55,"Mini-sensor: "+board);
  myLatex->DrawLatex(0.65, 0.50,"190 #mum pitch");
  myLatex->DrawLatex(0.65, 0.45,"320 #mum thick");

  c1->cd(k++);
  addGraphics(hn0, 2, "Strip number", "Noise(ADC), Counts");
  hn0->Draw();
  h4c->Draw("same");


  c1->cd(k++);  
  TH2F *h3den = (TH2F*)h3b->Clone("h3den");
  h3den->Add(h3c);
  TH2F *h3num = (TH2F*)h3c->Clone("h3num");
  TH2F *h3eff = (TH2F*)h3num->Clone("h3eff");
  h3eff->Divide(h3den);
  addGraphics(h3eff, 1, "X_{trk} [mm]", "Y_{trk} [mm] ");
  h3b->GetXaxis()->SetRangeUser(xMin-0.5,xMax+1.5);
  h3b->GetYaxis()->SetRangeUser(yMin-0.5,yMax+0.5);
  h3b->SetMarkerColor(4);
  h3c->SetMarkerColor(2);
  h3d->GetXaxis()->SetRangeUser(-6,6);
  h3d->GetYaxis()->SetRangeUser(-6,6);
  h3d->SetStats(kFALSE);
  h3d->SetTitle("Y vs X of tracks");
  h3d->Draw();
  h3c->Draw("same");
  h3b->Draw("same");
  TLegend* legend4 = new TLegend(0.60,0.75,0.94,0.88);
  legend4->SetFillStyle(0);
  legend4->SetBorderSize(0);
  legend4->SetFillStyle(0);
  legend4->SetTextSize(0.045);   
  TLegendEntry *le0 = legend4->AddEntry(h3d,"No cluster req'd","P"); 
  TLegendEntry *le1 = legend4->AddEntry(h3b,"Found Cluster","P"); 
  TLegendEntry *le2 = legend4->AddEntry(h3c,"Missed cluster","P");
  le1->SetTextColor(2);
  le2->SetTextColor(4);
  legend4->Draw();

  
  
   c1->cd(k++)->SetLeftMargin(0.13);
   addGraphics(h10d, 2, "Y_{trk} [mm]", "<ADC>");
   h10d->GetXaxis()->SetRangeUser(yMin-0.5,yMax+0.5);
   h10d->Draw();

   //c1->cd(2)->SetLeftMargin(0.13);
   //h3b->Draw();   

   

   c1->cd(k++)->SetLeftMargin(0.13);
   addGraphics(h12b, 2, "Y_{trk} [mm]", "");
   addGraphics(h12a, 1, "Y_{trk} [mm]", "");
   double mh12a = h12a->GetMaximum();
   h12a->SetMaximum(1.2*mh12a);
   h12a->GetXaxis()->SetRangeUser(yMin-0.5,yMax+0.5);
   h12a->Draw("hist");
   h12b->Draw("hist,same");

   TLegend* legend0 = new TLegend(0.15,0.75,0.94,0.89);
   legend0->SetFillStyle(0);
   legend0->SetBorderSize(0);
   legend0->SetFillStyle(0);
   legend0->SetTextSize(0.045);   
   legend0->AddEntry(h12a,"Track Y pos","LEP"); 
   legend0->AddEntry(h12b,"Track Y pos with matched cluster","LEP");
   legend0->Draw();

   c1->cd(k++)->SetLeftMargin(0.13);
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


   c1->cd(k++)->SetLeftMargin(0.13);
   addGraphics(h10e, 2, "X_{trk} [mm]", "<ADC>");
   h10e->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   h10e->Draw();

   c1->cd(k++)->SetLeftMargin(0.13);
   addGraphics(h12dd, 2, "X_{trk} [mm]", "");
   addGraphics(h12dn, 1, "X_{trk} [mm]", "");
   double mh12dd = h12dd->GetMaximum();
   h12dd->SetMaximum(1.2*mh12dd);
   h12dd->GetXaxis()->SetRangeUser(xMin-0.5,xMax+0.5);
   h12dd->Draw("hist");
   h12dn->Draw("hist,same");
   TLegend* legend5 = new TLegend(0.15,0.75,0.94,0.89);
   legend5->SetFillStyle(0);
   legend5->SetBorderSize(0);
   legend5->SetFillStyle(0);
   legend5->SetTextSize(0.045);   
   legend5->AddEntry(h12a,"Track X pos","LEP"); 
   legend5->AddEntry(h12b,"Track X pos with matched cluster","LEP");
   legend5->Draw();


   c1->cd(k++)->SetLeftMargin(0.13);
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

  c1->cd(k++);
  //hef->SetTitle("Relative Efficiency vs interstrip pos");
  h12n->Draw();


  c1->cd(k++)->SetLeftMargin(0.13);
  addGraphics(hch,1,"Interstrip position","Average charge (ADC)");
  addGraphics(heff,1,"Interstrip position","Relative Efficiency");
  hch->GetYaxis()->SetTitleOffset(1.4);
  hch->Draw();
  c1->cd(k++);
  heff->GetYaxis()->SetTitleOffset(1.0);
  //heff->GetYaxis()->SetRangeUser(0.8,1.1);
  heff->SetTitle("Relative Efficiency vs interstrip pos");
  heff->Draw();


   c1->Print(outdir+"/PerformancePlots_"+board+"_s"+sector+"_vb"+bias+".png");
   c1->Print(outdir+"/PerformancePlots_"+board+"_s"+sector+"_vb"+bias+".pdf");



  

}
