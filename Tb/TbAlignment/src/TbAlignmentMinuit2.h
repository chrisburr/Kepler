#pragma once

// Local
#include "TbAlignmentMinuitBase.h"

class TbAlignmentMinuit2 : public TbAlignmentMinuitBase {

 public:
  /// Constructor
  TbAlignmentMinuit2(const std::string& type, const std::string& name,
                     const IInterface* parent);
  /// Destructor
  virtual ~TbAlignmentMinuit2();

  /// Collect tracks and clusters for alignment (called at each event).
  virtual StatusCode execute(std::vector<TbAlignmentTrack*>& tracks);

  virtual void align(std::vector<TbAlignmentTrack*>& tracks);
  void chi2(double& f, double* par, double* g);

 private:
  /// TES location prefix of clusters
  std::string m_clusterLocation;
  /// Plane index of the device to align
  unsigned int m_deviceToAlign;
  /// Flag whether the device to align is excluded from the pattern recognition
  bool m_isDUT;
  bool m_refitTracks;
  bool m_ignoreEdge;
  /// Time window for associating clusters to a track
  double m_twindow;
  /// Spatial window for associating clusters to a track
  double m_xwindow;
};
