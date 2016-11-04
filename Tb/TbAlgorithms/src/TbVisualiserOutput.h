#ifndef TB_VISUALISEROUTPUT_H
#define TB_VISUALISEROUTPUT_H 1

// Tb/TbKernel
#include "TbKernel/ITbClusterFinder.h"
#include "TbKernel/TbAlgorithm.h"

// Tb/TbEvent
#include "Event/TbTrack.h"

/** @class TbVisualiserOutput TbVisualiserOutput.h
 * @author Dan Saunders
 */

class TbVisualiserOutput : public TbAlgorithm {
 public:
  /// Constructor
  TbVisualiserOutput(const std::string &name, ISvcLocator *pSvcLocator);
  /// Destructor
  virtual ~TbVisualiserOutput();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  LHCb::TbTracks *m_tracks;
  ITbClusterFinder *m_clusterFinder;
  std::string m_clusterLocation;
  std::string m_trackLocation;

  unsigned int m_viewerEvent;
  unsigned int m_event;

  void outputViewerData();
};
#endif
