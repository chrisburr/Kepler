#ifndef TB_TESTMC_H
#define TB_TESTMC_H 1

#include "TH1.h"
#include "TH2.h"

#include "GaudiAlg/GaudiTupleAlg.h"
#include "GaudiKernel/RndmGenerators.h"

#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/ITbGeometrySvc.h"
#include "TbKernel/TbModule.h"
#include "Event/TbHit.h"
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"

/** @class TbTestMC TbTestMC.h
 *  Author: Dan Saunders
 */

class TbTestMC : public GaudiTupleAlg {
 public:
  TbTestMC(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TbTestMC() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();    ///< Algorithm finalization
  int m_nEvents;
  void generate_tracks();
  void createClustFromTrack();
  void add_to_TES();
  void ExportHits();
  void sort_stuff();
  void sort_by_chip();
  void track_torder();
  void cluster_torder();
  void hit_torder(const unsigned int plane);
  Gaudi::XYZPoint ClustGposn(int);
  int ClustCharge();
  void make_tracks();
  LHCb::TbTrack* make_track(Gaudi::XYZPoint, float, int);
  void make_noise();
  LHCb::TbCluster* make_cluster(Gaudi::XYZPoint, float, int, int);
  int ClustSize();
  void setClusterHitsAndCharge(LHCb::TbCluster*, int);
  Gaudi::XYZPoint getIntercept(const unsigned int& plane, LHCb::TbTrack*);
  

  int m_nTracks;
  int m_EventLength;
  int m_nNoiseClusters;
  int m_HitTimeJitter;
  float m_ClustPosnResolution;
  unsigned int m_nPlanes;
  float m_ChipWidth;
  float m_PosnSpread;
  float m_charge;
  bool m_ForceEfficiency;
  float m_ChargeSigma;
  float m_Pitch;
  double m_ChargeSharingWidth;
  float m_ThresholdCut;
  unsigned int m_nExportedTracks;
  bool m_ExportHits;
  bool m_ExportClusters;
  bool m_ExportTracks;

  int m_nExportedHits;

  std::string m_filename;
  bool m_misalign;
  
  std::vector<TbModule*> m_modules;

  std::vector<float> m_ClustSizeFracs;

  std::vector<std::vector<LHCb::TbHit*> > m_Hits;          // Per event.
  std::vector<LHCb::TbCluster*> m_Clusters;  // Per event.
  std::vector<LHCb::TbTrack*> m_Tracks;      // Per event.


  // Geometry service.
  mutable ITbGeometrySvc* m_geomSvc;
  ITbGeometrySvc* geomSvc() const {
    if (!m_geomSvc) m_geomSvc = svc<ITbGeometrySvc>("TbGeometrySvc", true);
    return m_geomSvc;
  }
  ITbTrackFit* m_trackFit;


  // Random number genorators.
  Rndm::Numbers m_uniform;
  Rndm::Numbers m_gauss;
  Rndm::Numbers m_landau;
  Rndm::Numbers m_gauss2;
  Rndm::Numbers m_uniform2;
};
#endif
