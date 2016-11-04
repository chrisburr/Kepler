#pragma once

// Local
#include "TbAlignmentMinuitBase.h"

class TbAlignmentMinuit1 : public TbAlignmentMinuitBase {

 public:
  /// Constructor
  TbAlignmentMinuit1(const std::string& type, const std::string& name,
                     const IInterface* parent);
  /// Destructor
  virtual ~TbAlignmentMinuit1();

  virtual void align(std::vector<TbAlignmentTrack*>& alignmentTracks);
  void chi2(double& f, double* par, double* g);

 private:
  /// Plane to be kept fixed.
  unsigned int m_referencePlane;
};
