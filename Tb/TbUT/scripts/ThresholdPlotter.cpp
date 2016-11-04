#include<iostream>
#include<vector>
#include<string>
#include "TH2D.h"


using namespace std;

void ThresholdPlotter()
{
    cout<<"start"<<endl;
    string fileName="noise_Mamba.dat";
    
    ifstream l_file(fileName.c_str());
    int channelsNumber=512;
    std::vector<int> thresholVector(channelsNumber,0);

    for(int channel=0;channel<channelsNumber;channel++)
    {
        double l_noiseFromFile=0;
        l_file >> l_noiseFromFile;
        thresholVector[channel]=l_noiseFromFile;
        cout<<"NoiseRetreiver===> channel: "<< channel <<"noise: "<<l_noiseFromFile<<endl;
    }
   
 
    cout<<"create histograms" <<endl;
    TH2D* noiseHistogram= new TH2D("Noise Histogram","Noise (1#sigma)",100,-0.5,channelsNumber+0.5 , 100, 0, 200);
    TH2D*  lowThresholdHistogram=new  TH2D("low Thresholds Histogram","low Thresholds ",100,-0.5,channelsNumber+0.5 , 100, 0, 200);
    TH2D* highThresholdHistogram=new  TH2D("high Thresholds Histogram","high Thresholds ",100,-0.5,channelsNumber+0.5 , 100, 0, 200);

    int lawThresholdMultiplicity=2.5;
    int highThresholdMultiplicity=3;

    cout<<"Fill histograms" <<endl;

    for(int channel=0;channel<channelsNumber;channel++)
    {
        noiseHistogram->Fill(channel, thresholVector[channel]);
        lowThresholdHistogram->Fill(channel,lawThresholdMultiplicity* thresholVector[channel]);
        highThresholdHistogram->Fill(channel,highThresholdMultiplicity* thresholVector[channel]);
    }

    TCanvas * c1 = new TCanvas("c", "c", 600, 800);
    
    noiseHistogram->GetYaxis()->SetTitle("[ADC]");
    noiseHistogram->GetXaxis()->SetTitle("channel");
    noiseHistogram->SetMarkerStyle(2);
    noiseHistogram->SetLineColor(kWhite);

    lowThresholdHistogram->SetMarkerStyle(2);
    lowThresholdHistogram->SetMarkerColor(kGreen);
    lowThresholdHistogram->SetLineColor(kWhite);

    highThresholdHistogram->SetMarkerStyle(2);
    highThresholdHistogram->SetMarkerColor(kRed);
    highThresholdHistogram->SetLineColor(kWhite);

   
    noiseHistogram->Draw();
    lowThresholdHistogram->Draw("same");
    highThresholdHistogram->Draw("same");
    
    c1->BuildLegend(0.7296238,0.8274793,0.9004702,0.8997934);
    gStyle->SetOptStat(0);

}
