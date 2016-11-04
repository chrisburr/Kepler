#ifndef TB_COMBATBUILDER_H
#define TB_COMBATBUILDER_H 1

#include <fstream>

// Boost
#include <boost/iostreams/device/mapped_file.hpp>

// Tb/TbEvent
#include "Event/TbHit.h"
#include "Event/TbTrigger.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbCombatBuilder TbCombatBuilder.h
 *  @author Dan Saunders & Vinicius Franco
 *  @date   2014-09-22
 */

class TbCombatBuilder : public TbAlgorithm {
 public:
  /// Standard constructor
  TbCombatBuilder(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbCombatBuilder();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();    ///< Algorithm termination

 private:
  /// Input data files
  std::string m_readoutFormat;
  std::string m_fileName0;
  std::string m_fileName1;
  std::ifstream m_dataStream0;
  std::ifstream m_dataStream1;
  /// TES location of output hits
  std::string m_hitLocation;
  int m_nPixels;

  /// Number of events to skip TODO
  unsigned int m_skipEvents;
  /// Min. number of non-empty planes required to make an event TODO
  unsigned int m_nMinPlanesWithHits;

  /// Frequency to print event count.
  unsigned int m_printFreq;

  /// Number of processed events
  unsigned int m_nEvents;
  std::vector<LHCb::TbHits*> m_hits;

  /// Fake timestamp (not htime).
  int m_fakeTime;
  bool m_neof;
  int m_nArms;  // 1 for vertical lab cases, 2 for testbeams.

  void fillHit(std::string, int);
  void fillEventFromPixelmanFile(const int arm);
  void fillEventFromRelaxDFile();
};

#endif
