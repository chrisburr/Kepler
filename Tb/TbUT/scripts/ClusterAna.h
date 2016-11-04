//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Feb  4 16:48:43 2016 by ROOT version 5.34/10
// from TTree Clusters/TbUT nTuple
// found on file: /data2/pmmannin/FullProcessing/BoardA2/Run_Bias_Scan-B2-A-1071-14008_Tuple.root
//////////////////////////////////////////////////////////

#ifndef ClusterAna_h
#define ClusterAna_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLegend.h>
#include <TBox.h>
#include <TLine.h>

#include "AnalysisBaseCluOnly.h"
// Header file for the classes stored in the TTree if any.
#include <vector>
#include <iostream>
#include <algorithm>
// Fixed size dimensions of array or collections stored in the TTree if any.

class ClusterAna : public AnalysisBaseCluOnly{
public :
  
  ClusterAna(int biasVal);
  virtual ~ClusterAna();
  virtual void     Loop();
  virtual TString getFileBase(TString scan="Bias", TString board="", TString bias="", TString angle="0", TString sector="1");  

  TFile *fout;

};

#endif

#ifdef ClusterAna_cxx
ClusterAna::ClusterAna(int biasVal) {
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
  TString filename;
  if(biasVal != 0) m_bias = Form("%d",biasVal);
  TTree* tree = 0;
  TFile *f;
   if (tree == 0) {
     //
     // lhcb-dev
     //
     TString scanType = "Run_Bias";
     if(m_scanType.Contains("Angle")) scanType = "Run_Angle";
     filename = m_fileIndir+"Board"+m_board+"/"+scanType+"_Scan-"+getFileBase("Bias",m_board, m_bias, m_angle,m_sector)+"_Tuple.root"; 

     f = new TFile(filename);
     tree = (TTree*)f->Get("TbUT/Clusters");
     std::cout << "===> Opening file: " << filename << std::endl;
   }

   
   Init(tree);

   TString filename2 = filename.ReplaceAll("_Tuple","");
   TFile *f2 = new TFile(filename2);
   f2->cd("TbUT");
  TH2D *hNoise = (TH2D*)gDirectory->Get("CMSData_vs_channel");
  TF1 *gau = new TF1("gau","gaus(0)",-120,120);
  gau->SetParameters(10000,0.0,40.0);

  TH1D *px;
  for(int i=0;i<512;i++){
    px = hNoise->ProjectionY(Form("px%d",i),i,i);
    px->GetXaxis()->SetRangeUser(-200,200);
    if(px->GetEntries() < 10){
      noise[i] = 1000;
      continue;
    }
    double p = px->GetMaximum();
    double m = px->GetMean();
    double e = px->GetRMS();
    gau->SetParameters(p, m, e);
    gau->SetRange(m-3*e,m+3*e);

    px->Fit(gau,"RQ");
    double mn = gau->GetParameter(1);
    double wid = gau->GetParameter(2);
    double emn = gau->GetParError(1);
    double ewid = gau->GetParError(2);
    noise[i] = wid;
  }



}

ClusterAna::~ClusterAna()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

TString ClusterAna::getFileBase(TString scan, TString board, TString bias, TString angle, TString sector) {

  TString tag = "";

  if(board.Contains("A6")){
    if(sector == "1") {
      if ( bias == "300" ) tag = "B6-A-212-8358";
      if ( bias == "250" ) tag = "B6-A-213-8359";
      if ( bias == "200" ) tag = "B6-A-214-8361";
      if ( bias == "150" ) tag = "B6-A-215-8362";
      if ( bias == "100" ) tag = "B6-A-216-8363";
      if ( bias == "75" )  tag = "B6-A-219-8365";
      if ( bias == "50" )  tag = "B6-A-220-8366";
    } else if(sector == "2"){
      if ( bias == "300" ) tag = "B6-A-242-8389";
      if ( bias == "250" ) tag = "B6-A-245-8392";
      if ( bias == "200" ) tag = "B6-A-246-8393";
      if ( bias == "150" ) tag = "B6-A-247-8394";
      if ( bias == "100" ) tag = "B6-A-248-8395";
      if ( bias == "75" )  tag = "B6-A-249-8396";
      if ( bias == "50" )  tag = "B6-A-250-8397";
    } else if(sector == "3"){
      if ( bias == "300" ) tag = "B6-A-293-8425";
      if ( bias == "250" ) tag = "B6-A-294-8426";
      if ( bias == "200" ) tag = "B6-A-295-8427";
      if ( bias == "150" ) tag = "B6-A-296-8428";
      if ( bias == "100" ) tag = "B6-A-297-8429";
      if ( bias == "75" )  tag = "B6-A-298-8430";
      if ( bias == "50" )  tag = "B6-A-299-8431";
    } else if(sector == "4"){
      if ( bias == "300" ) tag = "B6-A-326-8452";
      if ( bias == "250" ) tag = "B6-A-327-8453";
      if ( bias == "200" ) tag = "B6-A-328-8454";
      if ( bias == "150" ) tag = "B6-A-329-8455";
      if ( bias == "100" ) tag = "B6-A-330-8456";
      if ( bias == "75" )  tag = "B6-A-331-8457";
      if ( bias == "50" )  tag = "B6-A-332-8458";
    } else if(sector == "5"){
      if ( bias == "300" ) tag = "B6-A-378-8494";
      if ( bias == "250" ) tag = "B6-A-381-8497";
      if ( bias == "200" ) tag = "B6-A-382-8498";
      if ( bias == "150" ) tag = "B6-A-383-8499";
      if ( bias == "100" ) tag = "B6-A-384-8500";
      if ( bias == "75" )  tag = "B6-A-385-8501";
      if ( bias == "50" )  tag = "B6-A-386-8502";
    } else if(sector == "6"){
      if ( bias == "300" ) tag = "B6-A-408-8523";
      if ( bias == "250" ) tag = "B6-A-409-8524";
      if ( bias == "200" ) tag = "B6-A-410-8525";
      if ( bias == "150" ) tag = "B6-A-411-8526";
      if ( bias == "100" ) tag = "B6-A-412-8527";
      if ( bias == "75" )  tag = "B6-A-413-8528";
      if ( bias == "50" )  tag = "B6-A-414-8529";
    }
  } else if (board.Contains("A4")) {
    if(sector == "1") {
      if(bias == "400" ) tag = "B4-A-210-8552";
      if(bias == "350" ) tag = "B4-A-211-8553";
      if(bias == "300" ) tag = "B4-A-212-8554";
      if(bias == "250" ) tag = "B4-A-213-8555";
      if(bias == "200" ) tag = "B4-A-214-8556";
      if(bias == "150" ) tag = "B4-A-215-8557";
      if(bias == "100" ) tag = "B4-A-216-8558";
      if(bias == "75" )  tag = "B4-A-217-8559";
      if(bias == "50" )  tag = "B4-A-218-8560";
    }else if(sector == "2") {
      if(bias == "400" ) tag = "B4-A-242-8583";
      if(bias == "350" ) tag = "B4-A-243-8584";
      if(bias == "300" ) tag = "B4-A-246-8586";
      if(bias == "250" ) tag = "B4-A-247-8587";
      if(bias == "200" ) tag = "B4-A-248-8588";
      if(bias == "150" ) tag = "B4-A-249-8589";
      if(bias == "100" ) tag = "B4-A-250-8590";
      if(bias == "75" )  tag = "B4-A-251-8591";
      if(bias == "50" )  tag = "B4-A-252-8592";
    } else if(sector == "3"){
      if(bias == "400" ) tag = "B4-A-275-8615";
      if(bias == "350" ) tag = "B4-A-276-8616";
      if(bias == "300" ) tag = "B4-A-277-8617";
      if(bias == "250" ) tag = "B4-A-278-8618";
      if(bias == "200" ) tag = "B4-A-279-8619";
      if(bias == "150" ) tag = "B4-A-280-8620";
      if(bias == "100" ) tag = "B4-A-281-8621";
      if(bias == "75" )  tag = "B4-A-282-8622";
      if(bias == "50" )  tag = "B4-A-283-8623";
    } else if(sector == "4"){
      if(bias == "400" ) tag = "B4-A-217-8651";
      if(bias == "350" ) tag = "B4-A-218-8652";
      if(bias == "300" ) tag = "B4-A-219-8653";
      if(bias == "250" ) tag = "B4-A-220-8654";
      if(bias == "200" ) tag = "B4-A-221-8655";
      if(bias == "150" ) tag = "B4-A-222-8656";
      if(bias == "100" ) tag = "B4-A-223-8657";
      if(bias == "75" )  tag = "B4-A-224-8658";
      if(bias == "50" )  tag = "B4-A-225-8659";
    } else if(sector == "5"){
      if(bias == "400" ) tag = "B4-A-246-8680";
      if(bias == "350" ) tag = "B4-A-247-8681";
      if(bias == "300" ) tag = "B4-A-248-8682";
      if(bias == "250" ) tag = "B4-A-249-8683";
      if(bias == "200" ) tag = "B4-A-250-8684";
      if(bias == "150" ) tag = "B4-A-251-8685";
      if(bias == "100" ) tag = "B4-A-252-8686";
      if(bias == "75" )  tag = "B4-A-253-8687";
      if(bias == "50" )  tag = "B4-A-254-8688";
    } else if(sector == "6"){
      if(bias == "400" ) tag = "B4-A-227-8711";
      if(bias == "350" ) tag = "B4-A-228-8712";
      if(bias == "300" ) tag = "B4-A-229-8713";
      if(bias == "250" ) tag = "B4-A-230-8714";
      if(bias == "200" ) tag = "B4-A-231-8715";
      if(bias == "150" ) tag = "B4-A-232-8716";
      if(bias == "100" ) tag = "B4-A-233-8717";
      if(bias == "75" )  tag = "B4-A-234-8718";
      if(bias == "50" )  tag = "B4-A-235-8719";
      tag = tag.ReplaceAll("B4","B1");
    }
  } else if (board.Contains("A8")) {
    if(sector == "1"){
      if(bias == "500") tag = "B8-A-296-13332";
      if(bias == "400") tag = "B8-A-297-13333";
      if(bias == "350") tag = "B8-A-298-13334";
      if(bias == "300") tag = "B8-A-299-13335";
      if(bias == "250") tag = "B8-A-300-13336";
      if(bias == "200") tag = "B8-A-301-13337";
      if(bias == "150") tag = "B8-A-302-13338";
      if(bias == "100") tag = "B8-A-303-13339";
      if(bias == "75")  tag = "B8-A-304-13340";
      if(bias == "50")  tag = "B8-A-305-13341";
      if(m_scanType=="Angle" && angle=="10" && bias == "300") tag = "B8-A-378-13399";
      if(m_scanType=="Angle" && angle=="20" && bias == "300") tag = "B8-A-379-13400";
    }else if (sector == "2"){
      //if(bias == "500") tag = "B8-A-321-13354";
      if(bias == "400") tag = "B8-A-324-13355";
      if(bias == "350") tag = "B8-A-325-13356";
      if(bias == "300") tag = "B8-A-327-13357";
      if(bias == "250") tag = "B8-A-328-13358";
      if(bias == "200") tag = "B8-A-329-13359";
      if(bias == "150") tag = "B8-A-330-13360";
      if(bias == "100") tag = "B8-A-331-13361";
      if(bias == "75")  tag = "B8-A-332-13362";
      if(bias == "50") tag = "B8-A-333-13363";
    }else if (sector == "3"){
      //if(bias == "500") tag = "B8-A-358-13385";
      if(bias == "400") tag = "B8-A-359-13386";
      if(bias == "350") tag = "B8-A-360-13387";
      if(bias == "300") tag = "B8-A-361-13388";
      if(bias == "250") tag = "B8-A-365-13389";
      if(bias == "200") tag = "B8-A-366-13390";
      if(bias == "150") tag = "B8-A-367-13391";
      if(bias == "100") tag = "B8-A-368-13392";
      if(bias == "75")  tag = "B8-A-369-13393";
      if(bias == "50")  tag = "B8-A-370-13394";
    }else if (sector == "4"){
      //if(bias == "500") tag = "B8-A-309-13344";
      if(bias == "400") tag = "B8-A-311-13345";
      if(bias == "350") tag = "B8-A-312-13346";
      if(bias == "300") tag = "B8-A-314-13347";
      if(bias == "250") tag = "B8-A-315-13348";
      if(bias == "200") tag = "B8-A-316-13349";
      if(bias == "150") tag = "B8-A-317-13350";
      if(bias == "100") tag = "B8-A-318-13351";
      if(bias == "75")  tag = "B8-A-319-13352";
      if(bias == "50")  tag = "B8-A-320-13353";
    }else if (sector == "5"){
      //if(bias == "500") tag = "B8-A-334-13364";
      if(bias == "400") tag = "B8-A-337-13365";
      if(bias == "350") tag = "B8-A-339-13367";
      if(bias == "300") tag = "B8-A-340-13368";
      if(bias == "250") tag = "B8-A-341-13369";
      if(bias == "200") tag = "B8-A-342-13370";
      if(bias == "150") tag = "B8-A-343-13371";
      if(bias == "100") tag = "B8-A-344-13372";
      if(bias == "75")  tag = "B8-A-345-13373";
      if(bias == "50")  tag = "B8-A-346-13374";
      if(bias != "400") tag = tag.ReplaceAll("B8","B1");
    }else if (sector == "6"){
      //if(bias == "500") tag = "B8-A-347-13375";
      if(bias == "400") tag = "B8-A-348-13376";
      if(bias == "350") tag = "B8-A-349-13377";
      if(bias == "300") tag = "B8-A-350-13378";
      if(bias == "250") tag = "B8-A-352-13379";
      if(bias == "200") tag = "B8-A-353-13380";
      if(bias == "150") tag = "B8-A-354-13381";
      if(bias == "100") tag = "B8-A-355-13382";
      if(bias == "75")  tag = "B8-A-356-13383";
      if(bias == "50")  tag = "B8-A-357-13384";
      tag = tag.ReplaceAll("B8","B1");
    }
  } else if (board.Contains("D5")) {
    if(sector == "1"){
      if(bias == "50") tag = "B5-D-14-13066";
      if(bias == "75") tag = "B5-D-13-13065";
      if(bias == "100") tag = "B5-D-12-13064";
      if(bias == "150") tag = "B5-D-11-13063";
      if(bias == "200") tag = "B5-D-10-13062";
      if(bias == "250") tag = "B5-D-6-13058";
      if(bias == "300") tag = "B5-D-19-13071";
      if(bias == "350") tag = "B5-D-4-13056";
      if(bias == "400") tag = "B5-D-3-13055";
      if(bias == "500") tag = "B5-D-16-13068";
    }else if(sector == "2"){
      if(bias == "50")  tag = "B5-D-119-13165";
      if(bias == "75")  tag = "B5-D-118-13164";
      if(bias == "100") tag = "B5-D-117-13163";
      if(bias == "150") tag = "B5-D-116-13162";
      if(bias == "200") tag = "B5-D-115-13161";
      if(bias == "250") tag = "B5-D-114-13160";
      if(bias == "300") tag = "B5-D-113-13159";
      if(bias == "350") tag = "B5-D-111-13157";
      if(bias == "400") tag = "B5-D-110-13156";
      if(bias == "500") tag = "B5-D-109-13155";
    }else if(sector == "3"){
      if(bias == "50")  tag = "B5-D-167-13209";
      if(bias == "75")  tag = "B5-D-166-13208";
      if(bias == "100") tag = "B5-D-165-13207";
      if(bias == "150") tag = "B5-D-164-13206";
      if(bias == "200") tag = "B5-D-163-13205";
      if(bias == "250") tag = "B5-D-162-13204";
      if(bias == "300") tag = "B5-D-159-13202";
      if(bias == "350") tag = "B5-D-158-13201";
      if(bias == "400") tag = "B5-D-157-13200";
      if(bias == "500") tag = "B5-D-156-13199";
    }else if(sector == "4"){
      if(bias == "50")  tag = "B5-D-65-13114";
      if(bias == "75")  tag = "B5-D-64-13113";
      if(bias == "100") tag = "B5-D-63-13112";
      if(bias == "150") tag = "B5-D-62-13111";
      if(bias == "200") tag = "B5-D-61-13110";
      if(bias == "250") tag = "B5-D-59-13108";
      if(bias == "300") tag = "B5-D-58-13107";
      if(bias == "350") tag = "B5-D-57-13106";
      if(bias == "400") tag = "B5-D-56-13105";
      if(bias == "500") tag = "B5-D-55-13104";
    }else if(sector == "5"){
      if(bias == "50")  tag = "B5-D-218-13253";
      if(bias == "75")  tag = "B5-D-217-13252";
      if(bias == "100") tag = "B5-D-216-13251";
      if(bias == "150") tag = "B5-D-215-13250";
      if(bias == "200") tag = "B5-D-214-13249";
      if(bias == "250") tag = "B5-D-213-13248";
      if(bias == "300") tag = "B5-D-212-13247";
      if(bias == "350") tag = "B5-D-211-13246";
      if(bias == "400") tag = "B5-D-210-13245";
      if(bias == "500") tag = "B5-D-209-13244";
    }else if(sector == "6"){
      if(bias == "50")  tag = "B5-D-260-13294";
      if(bias == "75")  tag = "B5-D-259-13293";
      if(bias == "100") tag = "B5-D-258-13292";
      if(bias == "150") tag = "B5-D-257-13291";
      if(bias == "200") tag = "B5-D-256-13290";
      if(bias == "250") tag = "B5-D-255-13289";
      if(bias == "300") tag = "B5-D-254-13288";
      if(bias == "350") tag = "B5-D-253-13287";
      if(bias == "400") tag = "B5-D-252-13286";
      if(bias == "500") tag = "B5-D-251-13285";
    }
  } else if (board.Contains("D7")) {
    if(sector == "1"){
      if(bias == "50") tag = "B1-D-347-9360";
      if(bias == "75") tag = "B1-D-346-9359";
      if(bias == "100") tag = "B1-D-344-9356";
      if(bias == "150") tag = "B1-D-345-9357";
      if(bias == "200") tag = "B1-D-343-9355";
      if(bias == "300") tag = "B1-D-343-9354";
      if(bias == "400") tag = "B1-D-341-9353";
      if(bias == "500") tag = "B1-D-340-9272";
    }else if(sector == "2"){
      if(bias == "50") tag = "B1-D-471-9659";
      if(bias == "75") tag = "B1-D-470-9658";
      if(bias == "100") tag = "B1-D-469-9657";
      if(bias == "150") tag = "B1-D-468-9656";
      if(bias == "200") tag = "B1-D-467-9655";
      if(bias == "300") tag = "B1-D-466-9654";
      if(bias == "400") tag = "B1-D-465-9653";
      if(bias == "500") tag = "B1-D-464-9652";
    }else if(sector == "3"){
      if(bias == "50") tag = "B1-D-472-9660";
      if(bias == "75") tag = "B1-D-473-9661";
      if(bias == "100") tag = "B1-D-474-9662";
      if(bias == "150") tag = "B1-D-475-9663";
      if(bias == "200") tag = "B1-D-476-9664";
      if(bias == "300") tag = "B1-D-477-9665";
      if(bias == "400") tag = "B1-D-478-9666";
      if(bias == "500") tag = "B1-D-479-9667";
    }else if(sector == "4"){
      if(bias == "50") tag =  "B1-D-379-9424";
      if(bias == "75") tag =  "B1-D-378-9423";
      if(bias == "100") tag = "B1-D-377-9422";
      if(bias == "150") tag = "B1-D-376-9421";
      if(bias == "200") tag = "B1-D-375-9419";
      if(bias == "300") tag = "B1-D-374-9417";
      if(bias == "400") tag = "B1-D-373-9415";
      if(bias == "500") tag = "B1-D-372-9413";
    }else if(sector == "5"){
      if(bias == "50")  tag = "B1-D-489-9763";
      if(bias == "75")  tag = "B1-D-488-9762";
      if(bias == "100") tag = "B1-D-487-9761";
      if(bias == "150") tag = "B1-D-484-9674";
      if(bias == "200") tag = "B1-D-483-9673";
      if(bias == "300") tag = "B1-D-482-9672";
      if(bias == "400") tag = "B1-D-481-9671";
      if(bias == "500") tag = "B1-D-480-9670";
    }else if(sector == "6"){
      if(bias == "50") tag =  "B1-D-321-9252";
      if(bias == "75") tag =  "B1-D-320-9251";
      if(bias == "100") tag = "B1-D-319-9250";
      if(bias == "150") tag = "B1-D-318-9249";
      if(bias == "200") tag = "B1-D-317-9248";
      if(bias == "300") tag = "B1-D-316-9247";
      if(bias == "400") tag = "B1-D-315-9246";
      if(bias == "500") tag = "B1-D-314-9245";
    }
  } else if (board.Contains("A1")) {
    if(sector == "1"){    
      if(bias == "350") tag = "B1-A-1022-13947";
      if(bias == "300") tag = "B1-A-1021-13946";
      if(bias == "250") tag = "B1-A-1053-13971";
      if(bias == "200") tag = "B1-A-1052-13970";
      if(bias == "150") tag = "B1-A-1015-13939";
      if(bias == "100") tag = "B1-A-1014-13938";
      if(bias == "75")  tag = "B1-A-1013-13937";
      if(bias == "50")  tag = "B1-A-1012-13936";
    }else if(sector == "2"){
      if(bias == "350") tag = "B1-A-1062-13980";
      if(bias == "300") tag = "B1-A-1061-13979";
      if(bias == "250") tag = "B1-A-1060-13978";
      if(bias == "200") tag = "B1-A-1059-13977";
      if(bias == "150") tag = "B1-A-1058-13976";
      if(bias == "100") tag = "B1-A-1057-13975";
      if(bias == "75")  tag = "B1-A-1056-13974";
      if(bias == "50")  tag = "B1-A-1055-13973";
    }    
  } else if (board.Contains("A2")) {
    if(sector == "1"){    
      if(bias == "340") tag = "B2-A-1095-14030";
      if(bias == "325") tag = "B2-A-1094-14029";
      if(bias == "300") tag = "B2-A-1084-14017";
      if(bias == "250") tag = "B2-A-1083-14016";
      if(bias == "200") tag = "B2-A-1082-14015";
      if(bias == "150") tag = "B2-A-1081-14014";
      if(bias == "100") tag = "B2-A-1078-14013";
      if(bias == "75")  tag = "B2-A-1077-14012";
      if(bias == "50")  tag = "B2-A-1076-14011";
    }else if(sector == "2"){
      if(bias == "340") tag = "B2-A-1099-14034";
      if(bias == "325") tag = "B2-A-1098-14033";
      if(bias == "300") tag = "B2-A-1097-14032";
      if(bias == "250") tag = "B2-A-1100-14035";
      if(bias == "200") tag = "B2-A-1101-14036";
      if(bias == "150") tag = "B2-A-1102-14037";
      if(bias == "100") tag = "B2-A-1103-14038";
      if(bias == "75")  tag = "B2-A-1104-14039";
      if(bias == "50")  tag = "B2-A-1105-14040";
    }    
  }

  return tag;
}

#endif // #ifdef ClusterAna_cxx
