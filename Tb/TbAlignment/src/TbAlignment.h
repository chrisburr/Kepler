#ifndef TBALIGNMENT_H
#define TBALIGNMENT_H 1

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

// Local
#include "TbAlignmentBase.h"

/** @class TbAlignment TbAlignment.h
 *
 *  Algorithm for telescope alignment.
 *
 *  @author Angelo Di Canto
 *  @date   2014-04-22
 */

class TbAlignmentTrack;

class TbAlignment : public TbAlgorithm {

 public:
  /// Standard constructor
  TbAlignment(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbAlignment();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();    ///< Algorithm finalization

 private:
  /// Output alignment file
  std::string m_outputFile;
  /// Alignment methods to be run
  std::vector<std::string> m_alignmentSequence;
  /// For the track counter printout
  unsigned int m_lastTrackPrint;

  /// Tracks for alignment
  std::vector<TbAlignmentTrack*> m_tracks;
  /// Number of alignment tracks after which to stop processsing.
  unsigned int m_nTracks;

  std::vector<TbAlignmentBase*> m_toolChain;
  std::vector<TbAlignmentBase*>::iterator m_toolIterator;

  bool writeAlignmentFile();
};

#endif  // TBALIGNMENT_H
