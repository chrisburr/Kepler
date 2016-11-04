void runClusterAnaLoop(TString board = "A1", int iSector = 1){

  if(board=="A8"){
    const int nBias = 9;  
    int bias[nBias] = {400, 350, 300, 250, 200, 150, 100, 75, 50};
  }else if(board=="D5"){
    const int nBias = 10;  
    int bias[nBias] = {500, 400, 350, 300, 250, 200, 150, 100, 75, 50};
  }else if(board=="A2"){
    const int nBias = 9;  
    int bias[nBias] = {340, 325, 300, 250, 200, 150, 100, 75, 50};
  }else if(board=="A1"){
    const int nBias = 8;  
    int bias[nBias] = {350, 300, 250, 200, 150, 100, 75, 50};
  }else if(board.Contains("D7")){
    const int nBias = 8;  
    int bias[nBias] = {500, 400, 300, 200, 150, 100, 75, 50};
  }else if(board.Contains("A4")){
    const int nBias = 9;  
    int bias[nBias] = {400, 350, 300, 250, 200, 150, 100, 75, 50};
  }else if(board.Contains("A6")){
    const int nBias = 7;  
    int bias[nBias] = {300, 250, 200, 150, 100, 75, 50};
  }

  gROOT->ProcessLine(".L AnalysisBaseCluOnly.C+");
  gROOT->ProcessLine(".L ClusterAna.C+");

  TTree *t;

  for(int i=0; i<nBias;i++) {
    int b = bias[i];
    cout << "+++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "Processing Bias = " << b << "V " << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++" << endl;    
    if(board=="D7" && iSector == 1 && b == 300) continue;
    
    
    ClusterAna a(b);  
    a.Loop();

  }
}  

