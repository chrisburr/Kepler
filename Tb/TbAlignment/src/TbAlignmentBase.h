#pragma once


// Gaudi
#include "GaudiAlg/GaudiHistoTool.h"

// Tb/TbKernel
#include "TbKernel/ITbGeometrySvc.h"
#include "TbKernel/TbAlignmentTrack.h"
#include "TbKernel/TbModule.h"
#include "TbKernel/ITbTrackFit.h"

static const InterfaceID IID_TbAlignmentBase("TbAlignmentBase", 1, 0);

class TbAlignmentBase : public GaudiHistoTool {

 public:
  /// Return the interface ID
  static const InterfaceID& interfaceID() { return IID_TbAlignmentBase; }
  /// Constructor
  TbAlignmentBase(const std::string& type, const std::string& name,
                  const IInterface* parent);
  /// Destructor
  ~TbAlignmentBase() {}

  virtual StatusCode initialize();

  /// Store tracks/clusters to be used for the alignment (called each event).
  virtual StatusCode execute(std::vector<TbAlignmentTrack*>& alignmentTracks);
  /// Alignment function (called after enough tracks have been collected).
  virtual void align(std::vector<TbAlignmentTrack*>& alignmentTracks) = 0;

  bool clearTracks() const { return m_clearTracks; }
  /// Fill monitoring histograms.
  void plotResiduals(std::vector<TbAlignmentTrack*>& tracks,
                     const std::string& tag);

 protected:
  /// TES location of tracks.
  std::string m_trackLocation;
  /// Chi2 cut on tracks to be used for alignment 
  double m_maxChi2;
  /// Degrees of freedom
  std::vector<bool> m_dofs;
  /// Default degrees of freedom
  std::vector<bool> m_dofsDefault;
  /// List of masked planes
  std::vector<unsigned int> m_maskedPlanes;
  /// Flags whether a plane is masked or not.
  std::vector<bool> m_masked;
  /// Flag to produce monitoring histograms.
  bool m_monitoring;
  /// Flag to reset the track store before collecting alignment tracks.
  bool m_clearTracks;

  std::vector<TbModule*> m_modules;
  unsigned int m_nPlanes;
  /// Track fit tool
  ITbTrackFit* m_trackFit;

  bool masked(const unsigned int plane) const {
    return plane < m_masked.size() ? m_masked[plane] : false;
  }

  /// Pointer to the geometry service.
  mutable ITbGeometrySvc* m_geomSvc;
  /// On-demand access to the geometry service.
  ITbGeometrySvc* geomSvc() const {
    if (!m_geomSvc) m_geomSvc = svc<ITbGeometrySvc>("TbGeometrySvc", true);
    return m_geomSvc;
  }
  /// Determine whether a track passes the edge region of a plane.
  bool isEdge(const LHCb::TbTrack* track) {
    for (auto cluster : track->clusters()) {
      if (isEdge(cluster)) return true;
    }
    return false;
  }
  /// Determine whether a cluster is close to the edge region of a plane.
  bool isEdge(const LHCb::TbCluster* cluster) {
    return cluster->xloc() <  0.5 ||
           cluster->xloc() > 13.5 ||
           cluster->yloc() < 0.5 ||
           cluster->yloc() > 13.5 ;
  }


};
