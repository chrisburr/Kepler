{
  //-------------------------
  // To execute:
  // SetupProject LHCb v36r2
  // root[]: .x runExampleAnalysis.C
  //
  //-------------------------

  gROOT->ProcessLine(".L CMS.C+");
  gROOT->ProcessLine(".L AnalysisBase.C+");
  gROOT->ProcessLine(".L ExampleAnalysis.C+");

  ExampleAnalysis a;
  a.Loop();

}
