#include <fstream>

// Gaudi
#include "GaudiKernel/IEventProcessor.h"

// Tb/TbKernel
#include "TbKernel/TbModule.h"
#include "TbKernel/TbAlignmentTrack.h"

// Local
#include "TbAlignment.h"

DECLARE_ALGORITHM_FACTORY(TbAlignment)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbAlignment::TbAlignment(const std::string &name, ISvcLocator *pSvcLocator)
    : TbAlgorithm(name, pSvcLocator), m_lastTrackPrint(0), m_tracks() {

  declareProperty("OutputAlignmentFile", m_outputFile = "Alignment_out.dat");
  declareProperty("AlignmentTechniques", m_alignmentSequence);
  declareProperty("NTracks", m_nTracks = 0);
}

//=============================================================================
// Destructor
//=============================================================================
TbAlignment::~TbAlignment() {

  // Delete the alignment tracks.
  for (auto track : m_tracks) {
    if (track) delete track;
  }
  m_tracks.clear();
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbAlignment::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  if (m_alignmentSequence.empty()) {
    return Error("Alignment sequence is empty.");
  }
  unsigned int scounter = 0;
  for (const auto &toolName : m_alignmentSequence) {
    info() << "Adding tool " << toolName << endmsg;
    const std::string name = "s" + std::to_string(scounter++);
    TbAlignmentBase* newTool = tool<TbAlignmentBase>(toolName, name, this);
    if (!newTool) return Error("Cannot retrieve tool " + toolName);
    m_toolChain.push_back(newTool);
  }
  m_toolIterator = m_toolChain.begin();
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbAlignment::execute() {

  auto currentTool = *m_toolIterator;
  StatusCode sc = currentTool->execute(m_tracks);
  if (sc.isFailure()) return sc;

  const unsigned int nTracks = m_tracks.size();
  if (nTracks - (nTracks % 1000) != m_lastTrackPrint) {
    m_lastTrackPrint = nTracks - (nTracks % 1000);
    info() << "Collected " << m_lastTrackPrint << " alignment tracks" << endmsg;
  }
  // If requested, stop processing events after a given number of tracks.
  if (m_nTracks > 0 && nTracks > m_nTracks) {
    currentTool->plotResiduals(m_tracks, "Before");
    currentTool->align(m_tracks);
    currentTool->plotResiduals(m_tracks, "After");
    ++m_toolIterator;
    if (m_toolIterator == m_toolChain.end()) {
      SmartIF<IEventProcessor> app(serviceLocator()->service("ApplicationMgr"));
      if (app) app->stopRun();
      return StatusCode::SUCCESS;
    }
    currentTool = *m_toolIterator;
    // Reset track cache
    if (currentTool->clearTracks()) {
      for (auto it = m_tracks.begin(), end = m_tracks.end(); it != end; ++it) {
        if (*it) delete *it;
      }
      m_tracks.clear();
      m_lastTrackPrint = 0;
    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Finalization
//=============================================================================
StatusCode TbAlignment::finalize() {

  if (m_toolIterator != m_toolChain.end()) {
    info() << "Event loop terminating before chain completion" << endmsg;
    (*m_toolIterator)->align(m_tracks);
  }
  writeAlignmentFile();
  // Finalise the base class.
  return TbAlgorithm::finalize();
}

//=============================================================================
// Save the alignment constants to file
//=============================================================================
bool TbAlignment::writeAlignmentFile() {
  // Write results to output file
  info() << "Writing alignment output file to " << m_outputFile << endmsg;
  std::ofstream txtfile(m_outputFile.c_str());
  if (!txtfile) {
    error() << "Cannot create file " << m_outputFile << endmsg;
    return false;
  }
  auto modules = geomSvc()->modules();
  for (auto im = modules.begin(), end = modules.end(); im != end; ++im) {
    txtfile << (*im)->id() << "  " << std::setw(12) << std::setprecision(10)
            << (*im)->x() << "  " << std::setw(12) << std::setprecision(10)
            << (*im)->y() << "  " << std::setw(12) << std::setprecision(10)
            << (*im)->z() << "  " << std::setw(12) << std::setprecision(10)
            << (*im)->rotX() << "  " << std::setw(12) << std::setprecision(10)
            << (*im)->rotY() << "  " << std::setw(12) << std::setprecision(10)
            << (*im)->rotZ() << std::endl;
  }
  txtfile.close();
  return true;
}
