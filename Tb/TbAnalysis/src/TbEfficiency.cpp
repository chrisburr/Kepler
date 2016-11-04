// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
// TODO (hschindl)
#include "GaudiKernel/IEventProcessor.h"

// Tb/TbEvent
#include "Event/TbTrack.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbEfficiency.h"

DECLARE_ALGORITHM_FACTORY(TbEfficiency)

//=============================================================================
// Standard constructor
//=============================================================================
TbEfficiency::TbEfficiency(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
			m_nTracksConsidered(0),
			m_nTracksAssociated(0),
			m_event(0),
			m_pitch(0.055) {

  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);
  declareProperty("DUT", m_dut = 4);
  declareProperty("CheckHitDUT", m_checkHitDUT = true);
  declareProperty("CheckHitAlivePixel", m_checkHitAlivePixel = true);
  declareProperty("PointingResAllowance", m_pointingResAllowance = 0.01);
  declareProperty("PointingResAllowanceDeadPixel", m_pointingResAllowance_deadPixels = 1);
  declareProperty("TakeDeadPixelsFromFile", m_takeDeadPixelsFromFile = true);
  declareProperty("nTotalTracks", m_nTotalTracks = 0);
  declareProperty("MaxChi", m_maxChi = 0);
}

//=============================================================================
// Finalization
//=============================================================================
StatusCode TbEfficiency::finalize() {

  info() <<"DUT Efficiency: "<<m_nTracksAssociated/(1.*m_nTracksConsidered)<<endmsg;
  info()<<"nTracksAssociated: "<<m_nTracksAssociated<<endmsg;
  info()<<"nTracksConsidered: "<<m_nTracksConsidered<<endmsg;
  info()<<"cf: "<<m_eff->GetEfficiency(1)<<"\t ±"<<m_eff->GetEfficiencyErrorLow(1)<<endmsg;

  TFile * f = new TFile("EfficiencyResults.root", "RECREATE");
  h_row->Write();
  h_col->Write();
  m_eff->Write();
  h_hitmap->Write();
  h_pixel->Write();
  h_pixel2x2->Write();

  m_eff->CreateGraph()->Write();
  h_hitmap->CreateHistogram()->Write();
  h_col->CreateGraph()->Write();
  h_row->CreateGraph()->Write();
  h_pixel->CreateHistogram()->Write();
  h_pixel2x2->CreateHistogram()->Write();
  f->Close();

  return TbAlgorithm::finalize();
}


//=============================================================================
// Initialization
//=============================================================================
StatusCode TbEfficiency::initialize() {
  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  uint nBins = 80;
  h_row = new TEfficiency("row", "row", 256, 0, 256);
  h_col = new TEfficiency("col", "col", 3*256+2, 0, 3*256+2);
  m_eff = new TEfficiency("eff", "eff", 1, 0, 1);
  h_hitmap = new TEfficiency("hitmap", "hitmap", 3*256+2, 0, 3*256+2, 256, 0, 256);
  h_pixel = new TEfficiency("pixel", "pixel", nBins, 0, m_pitch, nBins, 0, m_pitch);
  h_pixel2x2 = new TEfficiency("pixel2x2", "pixel2x2", 2*nBins, 0, 2*m_pitch, 2*nBins, 0, 2*m_pitch);

  if (m_takeDeadPixelsFromFile) {
    TFile * fIn = new TFile("telPlaneRef.root", "READ");
    const std::string name = "Tb/TbHitMonitor/HitMap/Plane" + std::to_string(m_dut);
    m_deadPixelMap = (TH2F*)fIn->Get(name.c_str());
    info() <<"DeadPixelMap name: "<<m_deadPixelMap->GetName()<<endmsg;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbEfficiency::execute() {
  // Grab the tracks.
  const LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    return Error("No tracks in " + m_trackLocation);
  }

	// Loop over the tracks.
	for (LHCb::TbTrack* track : *tracks) {
		if (m_nTotalTracks != 0 && m_nTracksConsidered >= m_nTotalTracks) {
			SmartIF<IEventProcessor> app(serviceLocator()->service("ApplicationMgr"));
			if (app) app->stopRun();
			return StatusCode::SUCCESS;
		}

		// Find the local position of the track intercept with the DUT.
		const Gaudi::XYZPoint interceptUG = geomSvc()->intercept(track, m_dut);
		const auto interceptUL = geomSvc()->globalToLocal(interceptUG, m_dut);

		if (!passSelectionCriteria(track, interceptUL)) continue;
		m_nTracksConsidered++;
		bool pass = false;
		if (track->associatedClusters().size() > 0) pass = true;

		int row = interceptUL.y()/m_pitch - 0.5;
		int col = interceptUL.x()/m_pitch - 0.5;

		h_pixel->Fill(pass, fmod(interceptUL.x(), m_pitch), fmod(interceptUL.y(), m_pitch));
		h_pixel2x2->Fill(pass, fmod(interceptUL.x(), 2*m_pitch), fmod(interceptUL.y(), 2*m_pitch));
		h_row->Fill(pass, row);
		h_col->Fill(pass, col);
		h_hitmap->Fill(pass, col, row);
		m_eff->Fill(pass, 0);
		if (pass) m_nTracksAssociated++;
		if (!pass && m_event == 1050) info()<<track->htime()<<endmsg;
	}

	m_event++;
  return StatusCode::SUCCESS;
}


//=============================================================================

bool TbEfficiency::passSelectionCriteria(LHCb::TbTrack * track, Gaudi::XYZPoint interceptUL)
{
	if (m_maxChi > 0 && track->chi2PerNdof() > m_maxChi) return false;
	if (m_checkHitDUT && !passedThroughDUT(interceptUL)) {
		return false;
	}
	if (m_checkHitAlivePixel && !passedThroughAlivePixel(interceptUL)) return false;

	return true;
}


//=============================================================================

bool TbEfficiency::passedThroughDUT(Gaudi::XYZPoint interceptUL)
{
	double xLow = 0.0 + m_pointingResAllowance;
	double yLow = 0.0 + m_pointingResAllowance;
	double yUp = 14.08 - m_pointingResAllowance;
	double xUp = 14.08;

	if (geomSvc()->modules().at(m_dut)->nChips() == 1)
		xUp = 14.08 - m_pointingResAllowance;
	else if (geomSvc()->modules().at(m_dut)->nChips() == 3)
		xUp = 42.35 - m_pointingResAllowance;
	else error()<<"Unknown device!"<<endmsg;

	if (interceptUL.x() < xLow ||
			interceptUL.x() > xUp ||
			interceptUL.y() < yLow ||
			interceptUL.y() > yUp) {
		counter("nTracksNotStrikingDUT") += 1;
		return false;
	}
	return true;
}


//=============================================================================

bool TbEfficiency::passedThroughAlivePixel(Gaudi::XYZPoint interceptUL)
{
	int row = interceptUL.y()/0.055 - 0.5;
  int col = interceptUL.x()/0.055 - 0.5;
  int one = 1;
  for (int iRow = row - m_pointingResAllowance_deadPixels;
  		iRow != row+m_pointingResAllowance_deadPixels+one; iRow++) {
  	for (int iCol = col - m_pointingResAllowance_deadPixels;
  			iCol != col + m_pointingResAllowance_deadPixels+one; iCol++) {

  		int colLim;
  		if (geomSvc()->modules().at(m_dut)->nChips() == 3) colLim = 3*256+2;
  		else colLim = 256;
  		if (iRow < 0 || iCol < 0 || iRow >= 256 || iCol >= colLim) continue;
			if (pixelSvc()->isMasked(pixelSvc()->address(iCol, iRow), m_dut)) {
				counter("nTracksStrikingMaskedPixels") += 1;
				return false;
			}
			else if (m_takeDeadPixelsFromFile && m_deadPixelMap->GetBinContent(iCol, iRow) == 0) {
				counter("nTracksStrikingDeadPixels") += 1;
				return false;
			}

  	}
  }
  return true;
}


//=============================================================================
