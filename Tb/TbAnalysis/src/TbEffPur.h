#ifndef TB_EFFICIENCY_H
#define TB_EFFICIENCY_H 1

// AIDA
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "TEfficiency.h"
#include "AIDA/IAxis.h"

#include "AIDA/IProfile2D.h"
#include "AIDA/IProfile1D.h"

// Tb/TbEvent
#include "Event/TbTrack.h"
#include "Event/TbCluster.h"
#include "Event/TbVertex.h"

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/TbAlgorithm.h"
#include "TbKernel/ITbClusterFinder.h"

#include "TFile.h"
#include "GaudiUtils/Aida2ROOT.h"

#include "TH2D.h"


/** @class TbEffPur TbEffPur.h
 *
 *  @author Dan Saunders
 */

class TbEffPur : public TbAlgorithm {
 public:
  TbEffPur(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TbEffPur() {}

  // Gaudi methods.
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

 private:
  // Members.
  std::string m_trackLocation;
  std::string m_vertexLocation;
  std::string m_clusterLocation;
  unsigned int m_DUTindex;
  LHCb::TbTracks * m_tracks;
  LHCb::TbClusters * m_clusters;
  ITbTrackFit * m_trackFit;
  double m_pitch;
  int m_nDUTpixels;


  unsigned int m_nTracks;
  unsigned int m_nClusters;
  unsigned int m_nTrackedClusters;

  unsigned int m_nClustersPassedCentral;
  unsigned int m_nTracksCentral;

  unsigned int m_nClustersPassedCorner;
  unsigned int m_nTracksCorner;

  double m_eff;
  double m_pur;
  double m_telescopeClusterVetoDelT;
  double m_edgeVetoDistance;

  TEfficiency * m_effs;
  TEfficiency * m_purs;
  TEfficiency * m_effHitmap;
  TEfficiency * m_purHitmap;
  TEfficiency * m_effHitmapInterPixel;
  TEfficiency * m_effHitmapInterPixelTriple;
  TEfficiency * m_purHitmapInterPixel;
  TEfficiency * m_effX;
  TEfficiency * m_effY;
  std::vector<TEfficiency*> m_effHitmapInterPixelVsSizes;

  unsigned int m_deadAreaRadius;
  double m_xLow;
  double m_xUp;
  double m_yLow;
  double m_yUp;
  double m_probCut;

  double m_rResidualCut;
  double m_tResidualCut;

  unsigned int m_chargeCutLow;
  unsigned int m_chargeCutUp;

  unsigned int m_litSquareSide;
  unsigned int m_nEvent;
  bool m_viewerOutput;
  unsigned int m_viewerEvent;
  double m_tGap;
  double m_correlationTimeWindow;
  bool m_applyVeto;
  std::vector<bool> * m_trackAssociated;

  // Plots.
  AIDA::IHistogram2D * m_remainsCorrelationsX;
  AIDA::IHistogram2D * m_remainsCorrelationsY;
  AIDA::IHistogram2D * m_remainsDifferencesXY;
  AIDA::IHistogram2D * m_clusterRemainsPositionsGlobal;
  AIDA::IHistogram2D * m_trackRemainsPositionsGlobal;
  AIDA::IHistogram2D * m_clusterRemainsPositionsLocal;
  AIDA::IHistogram2D * m_trackRemainsPositionsLocal;
  AIDA::IHistogram2D * m_vetoTracksHitmap;
  AIDA::IHistogram2D * m_vetoClustersHitmap;
  AIDA::IHistogram2D * m_timeResidualVsColumn;


  // Methods.
  void effPur();
  void trackClusters(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks);
  bool matchTrackToCluster(LHCb::TbCluster * cluster,
		LHCb::TbTrack * track);
  double getRadialSeparation(LHCb::TbCluster * cluster,
		LHCb::TbTrack * track);
  bool litPixel(LHCb::TbCluster * cluster,
		LHCb::TbTrack * track);
  bool globalCutPosition(Gaudi::XYZPoint);
  void outputViewerData();
  void applyVeto(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks);
  void correlateRemains(std::vector<LHCb::TbTrack*> * cutTracks,
  		std::vector<LHCb::TbCluster*> * cutClusters);
  void fillTrackClusters(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks, double tlow, double tup);
  void fillAllTrackClusters(std::vector<LHCb::TbCluster*> * cutClusters,
		std::vector<LHCb::TbTrack*> * cutTracks);
  void outputDeadRegion(unsigned int, unsigned int);
  bool outsideDUT(Gaudi::XYZPoint);
  bool interceptDeadPixel(Gaudi::XYZPoint);
};
#endif
