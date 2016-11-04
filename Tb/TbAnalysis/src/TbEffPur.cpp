#include <fstream>

// ROOT
#include "TMath.h"

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/HistoLabels.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbEffPur.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbEffPur)

//=============================================================================
// Standard constructor
//=============================================================================
TbEffPur::TbEffPur(const std::string& name, ISvcLocator* pSvcLocator)
  : TbAlgorithm(name, pSvcLocator),
  m_pitch(0.055),
  m_nDUTpixels(256),
  m_nTracks(0),
  m_nClusters(0),
  m_nTrackedClusters(0),
	m_nClustersPassedCentral(0),
	m_nTracksCentral(0),
	m_nClustersPassedCorner(0),
	m_nTracksCorner(0),
  m_eff(0.),
  m_pur(0.),
  m_deadAreaRadius(2.),
  m_trackAssociated(NULL)
{
  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("VertexLocation",
                  m_vertexLocation = LHCb::TbVertexLocation::Default);
  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);

  // Parameters.
  declareProperty("DUTindex", m_DUTindex = 4);
  declareProperty("GlobalCutXLow", m_xLow = 3);
  declareProperty("GlobalCutXUp", m_xUp = 9);
  declareProperty("GlobalCutYLow", m_yLow = 3);
  declareProperty("GlobalCutYUp", m_yUp = 9);
  declareProperty("LocalProbabilityCut", m_probCut = 0.5);

  declareProperty("RResidualCut", m_rResidualCut = 0.1);
  declareProperty("TResidualCut", m_tResidualCut = 100);
  declareProperty("ChargeCutLow", m_chargeCutLow = 0);
  // ... note this global cut enforced at different point to above.
  declareProperty("ChargeCutUp", m_chargeCutUp = 1000000);
  declareProperty("LitSquareSide", m_litSquareSide = 0);

  declareProperty("ViewerOutput", m_viewerOutput = false);
  declareProperty("ViewerEventNum", m_viewerEvent = 100);
  declareProperty("TGap", m_tGap = 200);
  declareProperty("CorrelationTimeWindow", m_correlationTimeWindow = 100);
  declareProperty("ApplyVeto", m_applyVeto = true);
  declareProperty("TelescopeClusterVetoDelT", m_telescopeClusterVetoDelT = 30);
  declareProperty("EdgeVetoDistance", m_edgeVetoDistance = 0.025);
  m_nEvent = -1;
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbEffPur::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

	// Efficiency objects.
	m_effX = new TEfficiency("efficiencyX", "efficiencyX", 40, 0.0, 0.11);
	m_effY = new TEfficiency("efficiencyY", "efficiencyY", 40, 0.0, 0.11);
	m_effs = new TEfficiency("efficiency", "efficiency", 1, -0.5, 0.5);
  m_purs = new TEfficiency("purity", "Tb/TbEffPur/pur", 1, -0.5, 0.5);
  m_effHitmap = new TEfficiency("efficiencyHitmap", "efficiencyHitmap",
  		64, 0.0, 14.08, 64, 0.0, 14.08);
  m_purHitmap = new TEfficiency("puritHitmap", "purityHitmap",
  		64, 0.0, 14.08, 64, 0.0, 14.08);

  int nBins = 50;
  m_effHitmapInterPixel = new TEfficiency("efficiencyHitmapInterPixel", "efficiencyHitmapInterPixel",
  		nBins, 0.0, 0.055, nBins, 0.0, 0.055);
  m_purHitmapInterPixel = new TEfficiency("puritHitmapInterPixel", "purityHitmapInterPixel",
  		nBins, 0.0, 0.055, nBins, 0.0, 0.055);

  m_effHitmapInterPixelTriple = new TEfficiency("efficiencyHitmapInterPixelTriple", "efficiencyHitmapInterPixelTriple",
  		150, 0.0, 10*0.055, 20, 0.0, 2*0.055);

  for (unsigned int i=1; i<5; i++) {
  	std::string name = "efficiencyHitmapInterClusterSize" + std::to_string(i);
  	TEfficiency * e = new TEfficiency(name.c_str(), name.c_str(),
  		nBins, 0.0, 0.055, nBins, 0.0, 0.055);
  	m_effHitmapInterPixelVsSizes.push_back(e);
  }

  // Plots for post correlations.
  m_remainsCorrelationsX = book2D("remainsClusterCorrelationsX", "remainsClusterCorrelationsX", 0, 14, 200, 0, 14, 200);
  m_remainsCorrelationsY = book2D("remainsClusterCorrelationsY", "remainsClusterCorrelationsY", 0, 14, 200, 0, 14, 200);
  m_remainsDifferencesXY = book2D("remainsDifferencesXY", "remainsDifferencesXY", -1.5, 1.5, 150, -1.5, 1.5, 150);
  m_clusterRemainsPositionsLocal = book2D("clusterRemainsPositionsLocal", "clusterRemainsPositionsLocal", -5, 20, 600, -5, 20, 600);
  m_trackRemainsPositionsLocal = book2D("trackRemainsPositionsLocal", "trackRemainsPositionsLocal", 0, 256, 600, 0, 256, 600);
  m_clusterRemainsPositionsGlobal = book2D("clusterRemainsPositionsGlobal", "clusterRemainsPositionsGlobal", -5, 20, 600, -5, 20, 600);
  m_trackRemainsPositionsGlobal = book2D("trackRemainsPositionsGlobal", "trackRemainsPositionsGlobal", -5, 20, 600, -5, 20, 600);
  m_vetoTracksHitmap = book2D("vetoTrackHitmap", "vetoTrackHitmap", -5, 20, 600, -5, 20, 600);
  m_vetoClustersHitmap = book2D("vetoClusterHitmap", "vetoClusterHitmap", -5, 20, 600, -5, 20, 600);
  m_timeResidualVsColumn = book2D("timeResidualVsColumn", "timeResidualVsColumn", 0, 256, 256, -50, 50, 500);


  // Setup the tools.
  m_trackFit = tool<ITbTrackFit>("TbTrackFit", "Fitter", this);
  // h_effVsCharge = new TH2F("effVsCharge", "effVsCharge", 50, 0.5, 1, 20, 0.5, 20.5);
  // TFile * f = new TFile("/afs/cern.ch/user/d/dsaunder/cmtuser/KEPLER/KEPLER_HEAD/EDR_plots/EfficiencyResults/Eff-S22-Run6309-500V.root", "READ");
  // h_effInter = (TH2F*)f->Get("eff_histo");
  return StatusCode::SUCCESS;

}


//=============================================================================
// Finalizer
//=============================================================================
StatusCode TbEffPur::finalize() {
	m_effs->SetTotalEvents(0, m_nTracks);
	m_effs->SetPassedEvents(0, m_nTrackedClusters);

	m_purs->SetTotalEvents(0, m_nClusters);
	m_purs->SetPassedEvents(0, m_nTrackedClusters);

	m_eff = m_nTrackedClusters/(1.0*m_nTracks);
	m_pur = m_nTrackedClusters/(1.0*m_nClusters);
	std::cout<<"ith ROOT Efficiency: " << m_effs->GetEfficiency(0) << " + " << m_effs->GetEfficiencyErrorUp(0) <<" - "<< m_effs->GetEfficiencyErrorLow(0) <<std::endl;
	std::cout<<"ith ROOT Purity: " << m_purs->GetEfficiency(0) << " + " << m_purs->GetEfficiencyErrorUp(0) <<" - "<< m_purs->GetEfficiencyErrorLow(0) <<std::endl;

	std::cout<<"Corner efficiency: "<<m_nClustersPassedCorner/(1.*m_nTracksCorner)<<std::endl;
	std::cout<<"Central efficiency: "<<m_nClustersPassedCentral/(1.*m_nTracksCentral)<<std::endl;

	plot(m_eff, "Efficiency", "Efficiency", 0.0, 1, 1);
	plot(m_pur, "Purity", "Purity", 0.0, 1, 1);
	plot(m_effs->GetEfficiencyErrorUp(0), "EfficiencyError", "EfficiencyError", 0.0, 1, 1);
	plot(m_purs->GetEfficiencyErrorUp(0), "PurityError", "PurityError", 0.0, 1, 1);

	TH2D * h = Gaudi::Utils::Aida2ROOT::aida2root(m_trackRemainsPositionsLocal);
	double maxBin = h->GetBinContent(h->GetMaximumBin());
	double minBin = h->GetBinContent(h->GetMinimumBin());
	for (int i=0; i<m_trackRemainsPositionsLocal->xAxis().bins(); i++) {
		for (int j=0; j<m_trackRemainsPositionsLocal->yAxis().bins(); j++) {
			plot(m_trackRemainsPositionsLocal->binHeight(i, j), "trackRemainsBins",
					"trackRemainsBins", minBin, maxBin, int(maxBin-minBin));
		}
	}

	TFile * f = new TFile("EfficiencyResults.root", "RECREATE");
	//m_effs->Write();

	m_effHitmapInterPixel->Write();
	m_effHitmapInterPixelTriple->Write();
	//h_effVsCharge->Write();
	//m_effs->CreateHistogram()->Write();
	m_effHitmapInterPixel->CreateHistogram()->Write();
	m_effHitmapInterPixelTriple->CreateHistogram()->Write();



//	for (unsigned int i=0; i<m_effHitmapInterPixelVsSizes.size(); i++) {
//		m_effHitmapInterPixelVsSizes[i]->Write();
//		//m_effHitmapInterPixelVsSizes[i]->CreateHistogram()->Write();
//	}
	f->Close();
  return TbAlgorithm::finalize();
}


//=============================================================================
// Main execution
//=============================================================================
StatusCode TbEffPur::execute()
{
	m_nEvent++;

  m_tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  //m_vertices = getIfExists<LHCb::TbVertices>(m_vertexLocation);
  m_clusters = getIfExists<LHCb::TbClusters>(m_clusterLocation + std::to_string(m_DUTindex));

  if (m_clusters->size() < 100 || m_tracks->size() < 100) return StatusCode::SUCCESS;

  // Matching.
  effPur();
  return StatusCode::SUCCESS;
}


//=============================================================================
// Efficiency finding
//=============================================================================
void TbEffPur::effPur()
{
	// Non-veto'd containers.
  std::vector<LHCb::TbCluster*> * cutClusters = new std::vector<LHCb::TbCluster*>();
  std::vector<LHCb::TbTrack*> cutTracks;
  int nClustersPreVeto = m_clusters->size();
  int nTracksPreVeto  = m_tracks->size();


  // Apply veto.
  if (m_applyVeto) applyVeto(cutClusters, &cutTracks);
  else fillAllTrackClusters(cutClusters, &cutTracks);
  m_trackAssociated = new std::vector<bool>(cutTracks.size(), false);
  //std::cout<<"Fraction tracks not veto'd: "<<cutTracks.size()/(1.0*nTracksPreVeto)<<"\tEvent: "<<m_nEvent<<std::endl;
  //std::cout<<"Fraction clusters not veto'd: "<<cutClusters->size()/(1.0*nClustersPreVeto)<<"\tEvent: "<<m_nEvent<<std::endl;
  plot(cutClusters->size()/(1.0*nClustersPreVeto), "clusterFractionVetod", "clusterFractionVetod", 0.0, 1.0, 200);
  plot(cutTracks.size()/(1.0*nTracksPreVeto), "trackFractionVetod", "trackFractionVetod", 0.0, 1.0, 200);


  // Do matching.
  m_nClusters += cutClusters->size();
  m_nTracks += cutTracks.size();
  trackClusters(cutClusters, &cutTracks);


  // Viewer options.
  if (m_nEvent == m_viewerEvent) {
  	if (m_viewerOutput) outputViewerData();
		for (LHCb::TbClusters::iterator iclust = cutClusters->begin();
				iclust != cutClusters->end(); iclust++) {
//			if (!(*iclust)->associated())
//				std::cout<<"Note: non-associated clusters at hTime: "<<(*iclust)->htime()<<std::endl;
		}
  }

  // Correlate remains.
  //correlateRemains(&cutTracks, cutClusters);
  delete cutClusters;
  delete m_trackAssociated;
}


//=============================================================================
// Global cuts
//=============================================================================
void TbEffPur::applyVeto(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks)
{
  // Gather all cluster times (over all planes) and sort it ___________________
	std::vector<double> allClusterTimes;
	for (unsigned int i=0; i<m_nPlanes; i++) {
    LHCb::TbClusters * clusters = getIfExists<LHCb::TbClusters>(m_clusterLocation + std::to_string(i));
		for (LHCb::TbClusters::const_iterator it = clusters->begin(); it != clusters->end(); ++it)
			allClusterTimes.push_back((*it)->htime());
	}
	std::sort(allClusterTimes.begin(), allClusterTimes.end());



	// Get big enough gaps in pairs _____________________________________________
	std::vector<double> tGapCenters;
	tGapCenters.push_back(0);
	for (std::vector<double>::iterator itime = allClusterTimes.begin()+1;
			itime != allClusterTimes.end(); itime++) {
		plot((*itime) - (*(itime-1)), "allGapWidths", "allGapWidths", 0.0, 3000.0, 200);
		if ((*itime) - (*(itime-1)) > m_tGap) {
			tGapCenters.push_back(0.5*((*itime) + (*(itime-1))));
			plot((*itime) - (*(itime-1)), "bigGapWidths", "bigGapWidths", 0.0, 3000.0, 200);
		}
	}
	tGapCenters.push_back(allClusterTimes.back());
	counter("nGaps") += tGapCenters.size();
	plot(tGapCenters.size(), "nGaps", "nGaps", 0.0, 1000.0, 200);



	// Loop over these sub events and veto ______________________________________
	std::vector<double>::iterator igap;
	for (igap = tGapCenters.begin(); igap != tGapCenters.end() - 1; igap++) {
		bool veto = false;

		if (igap == tGapCenters.begin() || igap == tGapCenters.end() -1) veto = true;

		// Loop tracks.
		if (!veto) {
			for (LHCb::TbTrack* track : *m_tracks) {
				if (track->htime() > (*igap) && track->htime() < (*(igap+1))) {
					// Veto on tracks outside cuts.
					const Gaudi::XYZPoint trackInterceptGlobal = geomSvc()->intercept(track, m_DUTindex);
					if (!globalCutPosition(trackInterceptGlobal)) veto = true;
					else if (outsideDUT(trackInterceptGlobal)) veto = true;
					else if (interceptDeadPixel(trackInterceptGlobal)) veto = true;
				}
			}
		}


		// Loop vertices.
//		if (!veto) {
//			for (LHCb::TbVertex* vertex : *m_vertices) {
//				if (vertex->htime() > (*igap) && vertex->htime() < (*(igap+1))) {
//					veto = true;
//				}
//			}
//		}


		// Loop DUT clusters.
		if (!veto) {
			for (const LHCb::TbCluster* cluster : *m_clusters) {
				if (cluster->htime() > (*igap) && cluster->htime() < (*(igap+1))) {
					// Veto on clusters outside cuts.
					Gaudi::XYZPoint clusterInterceptGlobal(cluster->x(), cluster->y(), cluster->z());
					if (!globalCutPosition(clusterInterceptGlobal)) veto = true;
					if (cluster->charge() > m_chargeCutUp || cluster->charge() < m_chargeCutLow) veto = true;
				}
			}
		}


		// Loop telescope clusters.
		if (!veto) {
			for (unsigned int i=0; i<m_nPlanes; i++) {
				if (i == m_DUTindex) continue;
				LHCb::TbClusters * clusters = getIfExists<LHCb::TbClusters>(m_clusterLocation + std::to_string(i));
				for (LHCb::TbClusters::const_iterator it = clusters->begin(); it != clusters->end()-1; ++it) {
					if ((*it)->htime() > (*igap) && (*it)->htime() < (*(igap+1))) {
						double delt = (*it)->htime() - (*(it+1))->htime();
						if (fabs(delt) < m_telescopeClusterVetoDelT) {
							veto = true;
							break;
						}
					}
				}
			}
		}


		if (!veto) fillTrackClusters(cutClusters, cutTracks, (*igap), (*(igap+1)));
		else counter("nGapsFiltered") += 1;
	}
}



//=============================================================================
// Correlate remains
//=============================================================================
void TbEffPur::correlateRemains(std::vector<LHCb::TbTrack*> * cutTracks,
		std::vector<LHCb::TbCluster*> * cutClusters)
{
	for (std::vector<LHCb::TbCluster*>::iterator ic = cutClusters->begin();
	  		ic != cutClusters->end(); ++ic) {
		if (!(*ic)->associated()) {
			m_clusterRemainsPositionsGlobal->fill((*ic)->x(), (*ic)->y());
			m_clusterRemainsPositionsLocal->fill((*ic)->xloc(), (*ic)->yloc());
			plot((*ic)->charge(), "chargeClusterRemains", "chargeClusterRemains", 0.0, 1000.0, 125);
		}
	}

	unsigned int i = 0;
	for (std::vector<LHCb::TbTrack*>::iterator itrack = cutTracks->begin();
			itrack != cutTracks->end(); itrack++) {
		if (!m_trackAssociated->at(i)) {
		  Gaudi::XYZPoint trackIntercept = geomSvc()->intercept((*itrack), m_DUTindex);
		  m_trackRemainsPositionsGlobal->fill(trackIntercept.x(), trackIntercept.y());
		  auto interceptUL = geomSvc()->globalToLocal(trackIntercept, m_DUTindex);
		  m_trackRemainsPositionsLocal->fill(interceptUL.x()/m_pitch - 0.5, interceptUL.y()/m_pitch - 0.5);
		}
		i++;
	}


  // Draws correlations plots between non-associated clusters and tracks.
	for (std::vector<LHCb::TbTrack*>::iterator itrack = cutTracks->begin();
			itrack != cutTracks->end(); itrack++) {
	  for (std::vector<LHCb::TbCluster*>::iterator ic = cutClusters->begin();
	  		ic != cutClusters->end(); ++ic) {
	  	if ((*ic)->htime() < (*itrack)->htime() - m_correlationTimeWindow) continue;
	  	if (!(*ic)->associated()) {
	  		//if (m_nEvent == m_viewerEvent) std::cout<<"Impurity at time: "<<(*ic)->htime()<<std::endl;
	  		Gaudi::XYZPoint trackIntercept = geomSvc()->intercept((*itrack), m_DUTindex);
	  		m_remainsCorrelationsX->fill(trackIntercept.x(), (*ic)->x());
	  		m_remainsCorrelationsY->fill(trackIntercept.y(), (*ic)->y());

	  		plot(trackIntercept.x() - (*ic)->x(), "remainClustDifferencesX", "remainsClustDifferencesX", -1.5, 1.5, 150);
	  		plot(trackIntercept.y() - (*ic)->y(), "remainClustDifferencesY", "remainClustDifferencesY", -1.5, 1.5, 150);
	  		plot((*itrack)->htime() - (*ic)->htime(), "remainClustDifferencesT", "remainClustDifferencesT", -50, 50, 150);

	  		m_remainsDifferencesXY->fill(trackIntercept.y() - (*ic)->y(), trackIntercept.x() - (*ic)->x());
	  	}
	  	if ((*ic)->htime() > (*itrack)->htime() + m_correlationTimeWindow) break;
	  }
	}
}


//=============================================================================
//
//=============================================================================
void TbEffPur::fillAllTrackClusters(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks)
{
  for (LHCb::TbTrack* track : *m_tracks) cutTracks->push_back(track);
  for (std::vector<LHCb::TbCluster*>::iterator iClust = m_clusters->begin();
  		iClust != m_clusters->end(); iClust++) cutClusters->push_back(*iClust);
}


//=============================================================================
//
//=============================================================================
void TbEffPur::fillTrackClusters(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks, double tlow, double tup)
{
	// Push tracks and clusters inside this time window, which passed the veto.
  for (LHCb::TbTrack* track : *m_tracks) {
  	if (track->htime() > tlow && track->htime() < tup) {
  		Gaudi::XYZPoint trackIntercept = geomSvc()->intercept(track, m_DUTindex);
  		cutTracks->push_back(track);
  		m_vetoTracksHitmap->fill(trackIntercept.x(), trackIntercept.y());

  		Gaudi::XYZPoint trackInterceptLocal = geomSvc()->globalToLocal(trackIntercept, m_DUTindex);
			int row = trackInterceptLocal.y()/m_pitch - 0.5;
			int col = trackInterceptLocal.x()/m_pitch - 0.5;

			if (pixelSvc()->isMasked(pixelSvc()->address(col, row), m_DUTindex)) {
				std::cout<<"Shouldn't be here!"<<std::endl;
			}
  	}
  }

  for (LHCb::TbCluster* cluster : *m_clusters) {
		if (cluster->htime() > tlow && cluster->htime() < tup) {
			cutClusters->push_back(cluster);
			m_vetoClustersHitmap->fill(cluster->x(), cluster->y());
		}
  }
}


//=============================================================================
// Global cuts
//=============================================================================
void TbEffPur::trackClusters(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks)
{
  for (std::vector<LHCb::TbTrack*>::iterator itrack = cutTracks->begin();
  		itrack != cutTracks->end(); itrack++) {

  	// Get local position of track.
  	const auto interceptUG = geomSvc()->intercept((*itrack), m_DUTindex);
    const auto interceptUL = geomSvc()->globalToLocal(interceptUG, m_DUTindex);


  	// Loop over clusters to find the closest.
  	LHCb::TbCluster * closestCluster = NULL;
  	double bestRadialDistance;
  	for (std::vector<LHCb::TbCluster*>::iterator iclust = cutClusters->begin();
  			iclust != cutClusters->end(); iclust++) {
  		bool match = matchTrackToCluster((*iclust), (*itrack));
  		double radialDistance = getRadialSeparation((*iclust), (*itrack));
  		if (match) {
  			if (!closestCluster) {
  				closestCluster = (*iclust);
  				bestRadialDistance = radialDistance;
  			}
  			else if (radialDistance<bestRadialDistance) {
  				closestCluster = (*iclust);
  				bestRadialDistance = radialDistance;
  			}
  		}

  		if ((*iclust)->htime() - (*itrack)->htime() > m_tResidualCut) break;
  	}

  	double trackX = interceptUL.x();
  	double trackY = interceptUL.y();

		if (closestCluster != NULL && closestCluster->charge() > m_chargeCutLow) {
			closestCluster->setAssociated(true);
			m_nTrackedClusters++;
			m_effHitmap->Fill(true, trackX, trackY);
			m_effX->Fill(true, trackX);
			m_effY->Fill(true, trackY);

			if (fmod(trackX, m_pitch)<0.0183 && fmod(trackY, m_pitch) < 0.0183) m_nClustersPassedCorner++;
			if (fmod(trackX, m_pitch) > 0.0183 &&
					fmod(trackX, m_pitch) < (2*0.0183) &&
					fmod(trackY, m_pitch) > 0.0183 &&
					fmod(trackY, m_pitch) < (2*0.0183)) m_nClustersPassedCentral++;


			//h_effVsCharge->Fill(h_effInter->Interpolate(fmod(trackX, m_pitch), fmod(trackY, m_pitch)), closestCluster->charge());
			// if (trackY > 9.25 && trackY < 10.25) m_effHitmapInterPixel->Fill(true, fmod(trackX, m_pitch), fmod(trackY, m_pitch));
			// if (trackX > 28.05 &&
			// 		trackX < 28.6) m_effHitmapInterPixelTriple->Fill(true, trackX-28.05, fmod(trackY, m_pitch));


      m_effHitmapInterPixel->Fill(true, fmod(trackX, m_pitch), fmod(trackY, m_pitch));
      m_effHitmapInterPixelTriple->Fill(true, trackX-28.05, fmod(trackY, m_pitch));


			if (closestCluster->size() <= m_effHitmapInterPixelVsSizes.size())
				m_effHitmapInterPixelVsSizes[closestCluster->size()-1]->Fill(true, fmod(trackX, m_pitch), fmod(trackY, m_pitch));
			m_trackAssociated->at(itrack - cutTracks->begin()) = true;
			plot(closestCluster->x() - interceptUG.x(), "xResidualsClustMinusTrack", "xResidualsClustMinusTrack", -0.2, 0.2, 400);
			plot(closestCluster->y() - interceptUG.y(), "yResidualsClustMinusTrack", "yResidualsClustMinusTrack", -0.2, 0.2, 400);
			plot(closestCluster->htime() - (*itrack)->htime(), "hTimeResidualClustMinusTrack", "hTimeResidualClustMinusTrack", -50, 50, 400);
			if (closestCluster->size() == 1) {
				auto hits = closestCluster->hits();
				int col = (*hits.begin())->col();
				m_timeResidualVsColumn->fill(col, closestCluster->htime() - (*itrack)->htime());
			}
			if ((*itrack)->vertexed()) counter("nVerticesCorrelated") += 1;
		}
		else {
			m_effHitmap->Fill(false, trackX, trackY);
			// if (trackY > 9.25 && trackY < 10.25) m_effHitmapInterPixel->Fill(false, fmod(trackX, m_pitch), fmod(trackY, m_pitch));
			// if (trackX > 28.05 &&
			// 		trackX < 28.6) m_effHitmapInterPixelTriple->Fill(false, trackX-28.05, fmod(trackY, m_pitch));

      m_effHitmapInterPixel->Fill(false, fmod(trackX, m_pitch), fmod(trackY, m_pitch));
      m_effHitmapInterPixelTriple->Fill(false, trackX-28.05, fmod(trackY, m_pitch));
			m_effX->Fill(false, trackX);
			m_effY->Fill(false, trackY);
			//std::cout<<"Inefficiency at time: "<<(*itrack)->htime()<<std::endl;
		}
		if (fmod(trackX, m_pitch)<0.0183 && fmod(trackY, m_pitch) < 0.0183) m_nTracksCorner++;
		if (fmod(trackX, m_pitch) > 0.0183 &&
					fmod(trackX, m_pitch) < (2*0.0183) &&
					fmod(trackY, m_pitch) > 0.0183 &&
					fmod(trackY, m_pitch) < (2*0.0183)) m_nTracksCentral++;
  }
}



//=============================================================================
//
//=============================================================================
double TbEffPur::getRadialSeparation(LHCb::TbCluster * cluster,
		LHCb::TbTrack * track)
{
  Gaudi::XYZPoint trackIntercept = geomSvc()->intercept(track, m_DUTindex);
	double xResidual = cluster->x() - trackIntercept.x();
	double yResidual = cluster->y() - trackIntercept.y();

	double r2 = xResidual*xResidual + yResidual*yResidual;
	return pow(r2, 0.5);
}


//=============================================================================
// Local cuts
//=============================================================================
bool TbEffPur::matchTrackToCluster(LHCb::TbCluster * cluster,
		LHCb::TbTrack * track)
{
  Gaudi::XYZPoint trackIntercept = geomSvc()->intercept(track, m_DUTindex);
	double xResidual = cluster->x() - trackIntercept.x();
	double yResidual = cluster->y() - trackIntercept.y();
	double tResidual = cluster->htime() - track->htime();
	double delr = pow(pow(xResidual, 2) + pow(yResidual, 2), 0.5);

//	if (track->vertexed()) plot(xResidual, "vertexResiduals/X", "vertexResiduals/X", -0.5, 0.5, 200);
//	if (track->vertexed()) plot(yResidual, "vertexResiduals/Y", "vertexResiduals/Y", -0.5, 0.5, 200);
//	if (track->vertexed()) plot(tResidual, "vertexResiduals/T", "vertexResiduals/T", -0.5, 0.5, 200);
//
//	if (track->vertexed() && litPixel(cluster, track)) {
//		plot(xResidual, "vertexResiduals/Xlit", "vertexResiduals/Xlit", -0.5, 0.5, 200);
//		plot(yResidual, "vertexResiduals/Ylit", "vertexResiduals/Ylit", -0.5, 0.5, 200);
//	}


	if (fabs(tResidual) > m_tResidualCut) {
  	counter("nTimeRejected") += 1;
		return false;
	}

  if (delr > m_rResidualCut) {
  	counter("nSpatialRejected") += 1;
  	if (litPixel(cluster, track)) return true;
  	else {
  		counter("nSpatialAndLitRejected") += 1;
  		return false;
  	}
  }

//	if (!litPixel(cluster, track)) return false;

  return true;
}


//=============================================================================
//
//=============================================================================
bool TbEffPur::interceptDeadPixel(Gaudi::XYZPoint trackInterceptGlobal)
{
	// Find the row and column corresponding to the track intercept.
	Gaudi::XYZPoint trackInterceptLocal = geomSvc()->globalToLocal(trackInterceptGlobal, m_DUTindex);
  int row = trackInterceptLocal.y()/m_pitch - 0.5;
  int col = trackInterceptLocal.x()/m_pitch - 0.5;

  for (int icol = col - 1; icol != col+2; icol++) {
  	for (int irow = row - 1; irow != row+2; irow++) {
  		if (icol >= 0 && icol <256 && irow>=0 && irow<256) {
  			if (pixelSvc()->isMasked(pixelSvc()->address(icol, irow), m_DUTindex)) {
					return true;
				}

//  			if (icol == 17 && irow == 36) {
//  				return true;
//  			}
//				if (icol == 18 && irow == 36) {
//  				return true;
//  			}
//				if (icol == 53 && irow == 3) {
//  				return true;
//  			}
//				if (icol == 54 && irow == 3) {
//  				return true;
//  			}
//				if (icol == 73 && irow == 26) {
//  				return true;
//  			}
//				if (icol == 74 && irow == 26) {
//  				return true;
//  			}
//				if (icol == 19 && irow == 103) {
//  				return true;
//  			}
//				if (icol == 31 && irow == 106) {
//  				return true;
//  			}
//				if (icol == 32 && irow == 106) {
//  				return true;
//  			}
//				if (icol == 38 && irow == 108) {
//  				return true;
//  			}
//				if (icol == 63 && irow == 95) {
//  				return true;
//  			}
//				if (icol == 64 && irow == 95) {
//  				return true;
//  			}
//				if (icol == 103 && irow == 23) {
//  				return true;
//  			}
//				if (icol == 104 && irow == 23) {
//  				return true;
//  			}
//				if (icol == 115 && irow == 20) {
//  				return true;
//  			}
//				if (icol == 116 && irow == 94) {
//  				return true;
//  			}
  		}
  	}
  }



  return false;
}

//=============================================================================
//
//=============================================================================
bool TbEffPur::litPixel(LHCb::TbCluster * cluster, LHCb::TbTrack * track)
{
	// Find the row and column corresponding to the track intercept.
	Gaudi::XYZPoint trackIntercept = geomSvc()->intercept(track, m_DUTindex);
	Gaudi::XYZPoint pLocal = geomSvc()->globalToLocal(trackIntercept, m_DUTindex);
  int row = pLocal.y()/m_pitch - 0.5;
  int col = pLocal.x()/m_pitch - 0.5;

  // See if within a pre-defined square of pixels.
  for (unsigned int i=0; i<cluster->size(); i++) {
  	unsigned int delRow = abs(row - int(cluster->hits()[i]->row()));
  	unsigned int delCol = abs(col - int(cluster->hits()[i]->col()));

  	if (delRow <= m_litSquareSide && delCol <= m_litSquareSide) {
		  counter("nLitPixels") += 1;
  		return true;
  	}
  }

  return false;
}


//=============================================================================
// Excluding dead regions.
//=============================================================================
bool TbEffPur::globalCutPosition(Gaudi::XYZPoint pGlobal)
{
	if (pGlobal.x() < m_xLow + m_edgeVetoDistance) return false;
	else if (pGlobal.x() > m_xUp - m_edgeVetoDistance) return false;
	else if (pGlobal.y() < m_yLow + m_edgeVetoDistance) return false;
	else if	(pGlobal.y() > m_yUp - m_edgeVetoDistance) return false;
  return true; // Inside.
}


//=============================================================================
// Excluding dead regions.
//=============================================================================
bool TbEffPur::outsideDUT(Gaudi::XYZPoint interceptUG)
{
	const auto interceptUL = geomSvc()->globalToLocal(interceptUG, m_DUTindex);
	if (interceptUL.x() < 0 + m_edgeVetoDistance ||
			interceptUL.x() > 14.08 - m_edgeVetoDistance ||
			interceptUL.y() < 0 + m_edgeVetoDistance||
			interceptUL.y() > 14.08 - m_edgeVetoDistance) return true;
	return false;
}



//=============================================================================
// Viewer output.
//=============================================================================
//=============================================================================
// Viewer output.
//=============================================================================
//=============================================================================
// Viewer output.
//=============================================================================

void TbEffPur::outputDeadRegion(unsigned int col, unsigned int row) {
	std::ofstream myfile;
  myfile.open("/afs/cern.ch/user/d/dsaunder/KeplerViewerData.dat", std::ofstream::app);
	std::string outputLine;
	for (int irow = std::max(int(row-m_deadAreaRadius), 0);
  		irow < std::min(int(row+m_deadAreaRadius+1), m_nDUTpixels); irow++) {
  	for (int icol = std::max(int(col-m_deadAreaRadius), 0);
  		icol < std::min(int(col+m_deadAreaRadius+1), m_nDUTpixels); icol++) {


			outputLine = "DeadPixel ";
			const double xLocal = (col-icol) * m_pitch; // Dont want the middle!
			const double yLocal = (row-irow) * m_pitch; // Dont want the middle!
			Gaudi::XYZPoint pLocal(xLocal, yLocal, 0.);

			Gaudi::XYZPoint posn = geomSvc()->localToGlobal(pLocal, m_DUTindex);
			outputLine += std::to_string(posn.x()) + " ";
			outputLine += std::to_string(posn.y()) + " ";
			outputLine += std::to_string(posn.z()) + " ";

			Gaudi::XYZPoint posn2(pLocal.x() + 0.055, pLocal.y(), 0.);
			posn = geomSvc()->localToGlobal(posn2, m_DUTindex);
			outputLine += std::to_string(posn.x()) + " ";
			outputLine += std::to_string(posn.y()) + " ";
			outputLine += std::to_string(posn.z()) + " ";

			Gaudi::XYZPoint posn3(pLocal.x() + 0.055, pLocal.y()+0.055, 0.);
			posn = geomSvc()->localToGlobal(posn3, m_DUTindex);
			outputLine += std::to_string(posn.x()) + " ";
			outputLine += std::to_string(posn.y()) + " ";
			outputLine += std::to_string(posn.z()) + " ";

			Gaudi::XYZPoint posn4(pLocal.x(), pLocal.y()+0.055, 0.);
			posn = geomSvc()->localToGlobal(posn4, m_DUTindex);
			outputLine += std::to_string(posn.x()) + " ";
			outputLine += std::to_string(posn.y()) + " ";
			outputLine += std::to_string(posn.z()) + " ";

			outputLine += "\n";
			myfile << outputLine;
  	}
	}
	myfile.close();
}


//=============================================================================
// Viewer output.
//=============================================================================

void TbEffPur::outputViewerData()
{
  std::ofstream myfile;
  myfile.open("/afs/cern.ch/user/d/dsaunder/KeplerViewerData.dat", std::ofstream::app);
  std::string outputLine;
//
//  // First output the chips.
//  for (unsigned int i=0; i<m_nPlanes; i++) {
//  	outputLine = "Chip ";
//  	Gaudi::XYZPoint posn1(0., 14.08, 0.);
//    Gaudi::XYZPoint posn = geomSvc()->localToGlobal(posn1, i);
//    outputLine += std::to_string(posn.x()) + " ";
//    outputLine += std::to_string(posn.y()) + " ";
//    outputLine += std::to_string(posn.z()) + " ";
//
//    Gaudi::XYZPoint posn2(14.08, 14.08, 0.);
//    posn = geomSvc()->localToGlobal(posn2, i);
//    outputLine += std::to_string(posn.x()) + " ";
//    outputLine += std::to_string(posn.y()) + " ";
//    outputLine += std::to_string(posn.z()) + " ";
//
//    Gaudi::XYZPoint posn3(14.08, 0., 0.);
//    posn = geomSvc()->localToGlobal(posn3, i);
//    outputLine += std::to_string(posn.x()) + " ";
//    outputLine += std::to_string(posn.y()) + " ";
//    outputLine += std::to_string(posn.z()) + " ";
//
//    Gaudi::XYZPoint posn4(0., 0., 0.);
//    posn = geomSvc()->localToGlobal(posn4, i);
//    outputLine += std::to_string(posn.x()) + " ";
//    outputLine += std::to_string(posn.y()) + " ";
//    outputLine += std::to_string(posn.z()) + " ";
//
//    outputLine += "\n";
//    myfile << outputLine;
//  }


  // Clusters.
  auto ic = m_clusters->begin();
	const auto end = m_clusters->end();
	for (; ic != end; ++ic) {
		outputLine = "Cluster ";
		outputLine += std::to_string((*ic)->x()) + " ";
		outputLine += std::to_string((*ic)->y()) + " ";
		outputLine += std::to_string((*ic)->z()) + " ";
		outputLine += std::to_string((*ic)->htime()) + " ";

		// if ((*ic)->endCluster() && (*ic)->vertexed()) outputLine += "5 \n";
		//if ((*ic)->vertexed()) outputLine += "4 \n";
//			else if ((*ic)->endCluster()) outputLine += "3 \n";
		if ((*ic)->associated()) outputLine += "2 \n";
//			else if ((*ic)->volumed()) outputLine += "1 \n";

		else {
			outputLine += "0 \n";
		}
		myfile << outputLine;
	}

	outputLine = "CentralRegion ";
	outputLine += std::to_string(m_xLow) + " ";
	outputLine += std::to_string(m_xUp) + " ";
	outputLine += std::to_string(m_yLow) + " ";
	outputLine += std::to_string(m_yUp) + " ";
	outputLine += std::to_string(geomSvc()->module(m_DUTindex)->z()) + " ";
	myfile << outputLine;

//		// Its hits.
//		for (auto hit : (*ic)->hits()) {
//			outputLine = "Pixel ";
//			const double xLocal = (hit->col()) * m_pitch; // Dont want the middle!
//			const double yLocal = (hit->row()) * m_pitch; // Dont want the middle!
//			Gaudi::XYZPoint pLocal(xLocal, yLocal, 0.);
//
//			Gaudi::XYZPoint posn = geomSvc()->localToGlobal(pLocal, i);
//			outputLine += std::to_string(posn.x()) + " ";
//			outputLine += std::to_string(posn.y()) + " ";
//			outputLine += std::to_string(posn.z()) + " ";
//
//			Gaudi::XYZPoint posn2(pLocal.x() + 0.055, pLocal.y(), 0.);
//			posn = geomSvc()->localToGlobal(posn2, i);
//			outputLine += std::to_string(posn.x()) + " ";
//			outputLine += std::to_string(posn.y()) + " ";
//			outputLine += std::to_string(posn.z()) + " ";
//
//			Gaudi::XYZPoint posn3(pLocal.x() + 0.055, pLocal.y()+0.055, 0.);
//			posn = geomSvc()->localToGlobal(posn3, i);
//			outputLine += std::to_string(posn.x()) + " ";
//			outputLine += std::to_string(posn.y()) + " ";
//			outputLine += std::to_string(posn.z()) + " ";
//
//			Gaudi::XYZPoint posn4(pLocal.x(), pLocal.y()+0.055, 0.);
//			posn = geomSvc()->localToGlobal(posn4, i);
//			outputLine += std::to_string(posn.x()) + " ";
//			outputLine += std::to_string(posn.y()) + " ";
//			outputLine += std::to_string(posn.z()) + " ";
//
//			outputLine += std::to_string(hit->htime()) + " ";
//			outputLine += std::to_string(hit->ToT()) + " ";
//
//			outputLine += "\n";
//			myfile << outputLine;
//		}

  myfile.close();
}


//=============================================================================
// END
//=============================================================================
