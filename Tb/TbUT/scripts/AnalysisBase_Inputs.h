//const TString m_fileIndir = "/data2/pmmannin/FullProcessing/Nov2015/"; 
//const TString m_fileIndir = "/data2/pmmannin/FullProcessing/July2015/"; 
//const TString m_fileIndir = "/data2/pmmannin/November2015/";
//const TString m_board 	= "A1_Full"; 
//const TString m_board 	= "D7"; 
// Board, use: _board = "D5_All", A8_All" - October TB
// const TString m_board 	= "D5_All";
// =========================================================

const TString m_fileIndir = "/data2/pmmannin/FullProcessing/";

TString m_board 	= "M1";
TString m_bias  	= "300";
TString m_sector	= "2"; 
const TString m_scanType = "Bias";
const TString m_angle = "0";

const TString m_fileOutdir = "~/lhcb/testbeam/"; 

const TString plotdir 	= "root_files/";


// --------------------------------------------
// Generally no need to change anything here...
// --------------------------------------------
const double nChan 			= 512.0;
const double stripPitch = 0.190;

const bool isPType = false;
const bool writeEventsWithMissinhHitsToFile = false;   // flag to write events to file with missing DUT hit
const double trackTriggerTimeDiffCut = 2.5;   // Default = 2.5

const int kClusterChargeMin = 150;
const double ratioForHole 	= 0.05;
const double ratioForDeadStrip 	= 0.6;
const double ratioForNoisyStrip = 1.8;
const bool vetoDeadStripRegions = false;
const double k_DeadStrip 				= 0.12;

const double m_minDistFromHole = 0.05;
const bool removeTracksInHoleDef = false;

const bool makePlots = true;
const bool printPlots = false;
