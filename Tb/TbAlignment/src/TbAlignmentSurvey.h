#pragma once

#include "TbAlignmentBase.h"

class TbAlignmentSurvey : public TbAlignmentBase {

 public:
  /// Constructor
  TbAlignmentSurvey(const std::string& type, const std::string& name,
                    const IInterface* parent);
  /// Destructor
  virtual ~TbAlignmentSurvey();

  virtual StatusCode initialize();

  virtual void align(std::vector<TbAlignmentTrack*>& alignmentTracks);
};
