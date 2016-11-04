#pragma once

// ROOT
#include "TFitter.h"

// Local
#include "TbAlignmentBase.h"

class TbAlignmentMinuitBase : public TbAlignmentBase {

 public:
  /// Constructor
  TbAlignmentMinuitBase(const std::string& type, const std::string& name,
                        const IInterface* parent);
  /// Destructor
  virtual ~TbAlignmentMinuitBase();

  virtual void align(std::vector<TbAlignmentTrack*>& alignmentTracks);
  virtual void chi2(double& f, double* par, double* g) = 0;

  virtual void updateGeometry();

 protected:
  /// Number of iterations
  unsigned int m_nIterations;
  /// Minuit fit strategy (allowed values are 0, 1, 2).
  int m_fitStrategy;

  std::vector<TbAlignmentTrack*> m_tracks;
  TFitter* m_fitter;

  void setParameters();
};
