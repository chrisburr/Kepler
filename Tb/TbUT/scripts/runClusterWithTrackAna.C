#include <stdlib.h> 
#include "AnalysisBase_Inputs.h"

void runClusterWithTrackAna(){

    int bias = atoi(m_bias);
  
  gROOT->ProcessLine(".L CMS.C+");
  gROOT->ProcessLine(".L AnalysisBase.C+");
  gROOT->ProcessLine(".L ClusterWithTrackAna.C+");
  //  gROOT->ProcessLine(".L ClusterTrackAnalysis.C+");
  //  int bias = atoi(m_bias);
  //int bias = 300;
  //    ClusterWithTrackAna a;
  //a.Loop();
  
  //  TTree *t;
   ClusterWithTrackAna a(bias);
  //  ClusterTrackAnalysis a(bias);
  //AnalysisBase a(bias);
  a.Loop();


}
