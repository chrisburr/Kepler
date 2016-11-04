double nChan = 512.0;
double stripPitch = 0.190;
double z_DUT = 370;
//double Rz = -0.0218;
double Rz = -0.0218;
double Ry = 0.00;
double dxWin = 0.25;
double xGloOff = -9.2;
double yGloOff = -7.5;
double xOff = -54.78;

int kClusterChargeMin = 120;

double ratioForHole = 0.05;

double ratioForDeadStrip = 0.6;
double ratioForNoisyStrip = 1.8;

bool vetoDeadStripRegions = true;
double k_DeadStrip = 0.12;

bool writeEventsWithMissinhHitsToFile = false;   // flag to write events to file with missing DUT hit

double trackTriggerTimeDiffCut = 2.5;

bool isPType = true;


// Notes
// For Rz alignment, one should SUBTRACT the slope found from the deltaX vs Y plot.
// For Z alignment, one should ADD the slope found from the deltaX vs Y plot.
//
