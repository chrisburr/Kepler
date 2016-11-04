#ifndef TB_TRIGGER_ASSOCIATOR_H
#define TB_TRIGGER_ASSOCIATOR_H 1

// Tb/TbEvent
#include "Event/TbTrigger.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbTriggerAssociator TbTriggerAssociator.h
 *
 *  Algorithm to link reconstructed tracks with matching trigger timestamps.
 *
 */

class TbTriggerAssociator : public TbAlgorithm {
 public:
  /// Constructor
  TbTriggerAssociator(const std::string &name, ISvcLocator *pSvcLocator);
  /// Destructor
  virtual ~TbTriggerAssociator() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  /// TES location of tracks
  std::string m_trackLocation;
  /// TES location of triggers
  std::string m_triggerLocation;

  /// Time window (in ns)
  double m_twindow;
  /// Time offset (in ns) of trigger packets with respect to tracks
  double m_toffset;
  /// Specify only a single plane to take triggers from , i.e. for multiple
  /// external users using different SPIDRS
  unsigned int m_plane;
  /// Functor for lower bound search.
  class lowerBound {
   public:
    bool operator()(const LHCb::TbTrigger *lhs, const double t) const {
      return lhs->htime() < t;
    }
  };
};
#endif
