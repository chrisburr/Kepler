#include <algorithm>

// Boost
#include <boost/filesystem.hpp>

// Gaudi
#include "GaudiKernel/IEventProcessor.h"

// Tb/TbKernel
#include "TbKernel/TbFunctors.h"

// Local
#include "TbCombatBuilder.h"

DECLARE_ALGORITHM_FACTORY(TbCombatBuilder)

namespace fs = boost::filesystem;

//=============================================================================
// Standard constructor
//=============================================================================
TbCombatBuilder::TbCombatBuilder(const std::string& name,
                                 ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_nPixels(256),
      m_nEvents(0),
      m_neof(true) {

  declareProperty("CombatInputFile0", m_fileName0);
  declareProperty("CombatInputFile1", m_fileName1 = "DefaultFileName1.dat");
  declareProperty("HitLocation", m_hitLocation = LHCb::TbHitLocation::Default);
  declareProperty("MinPlanesWithHits", m_nMinPlanesWithHits = 1);  // TODO
  declareProperty("SkipEvents", m_skipEvents = 0);                 // TODO
  declareProperty("PrintFreq", m_printFreq = 100);
  declareProperty("ReadoutFormat", m_readoutFormat);
  declareProperty("FakeTime", m_fakeTime = 1);
  declareProperty("NumberArms", m_nArms);
}

//=============================================================================
// Destructor
//=============================================================================
TbCombatBuilder::~TbCombatBuilder() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbCombatBuilder::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  // Prepare the hit containers.
  m_hits.resize(m_nPlanes, NULL);

  // Open the hit files.
  m_dataStream0.open(m_fileName0.c_str());
  if (!m_dataStream0.is_open()) {
    error() << "Cannot open file name: " << m_fileName0 << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_nArms == 2 && m_readoutFormat=="Pixelman") {
    m_dataStream1.open(m_fileName1.c_str());
    if (!m_dataStream1.is_open()) {
      error() << "Cannot open file name: " << m_fileName1 << endmsg;
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbCombatBuilder::execute() {
  // Printing.
  if (m_nEvents % m_printFreq == 0)
    info() << "Loading event: " << m_nEvents << endmsg;

  // Create containers for hits.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string ext = std::to_string(i);
    m_hits[i] = new LHCb::TbHits();
    put(m_hits[i], m_hitLocation + ext);
  }


  if(m_readoutFormat=="Pixelman"){
	  if (m_nArms == 1) {
		  fillEventFromPixelmanFile(0);
	  } else if (m_nArms == 2) {
		  fillEventFromPixelmanFile(0);
		  fillEventFromPixelmanFile(1);
	  }
  }
  else if(m_readoutFormat=="RelaxD"){
	  fillEventFromRelaxDFile();
  }

  // Check whether there are any events left.
  if (!m_neof) {
    // Terminate.
    SmartIF<IEventProcessor> app(serviceLocator()->service("ApplicationMgr"));
    if (app) app->stopRun();
  } else {
    // Ordering.
    for (unsigned int i = 0; i < m_nPlanes; i++) {
      std::sort(m_hits[i]->begin(), m_hits[i]->end(),
                TbFunctors::GreaterByToT<const LHCb::TbHit*>());
    }
    m_nEvents++;
  }

  // Prepare for the next event.
  return StatusCode::SUCCESS;
}

//=============================================================================
// Finalisation
//=============================================================================
StatusCode TbCombatBuilder::finalize() { return TbAlgorithm::finalize(); }

//=============================================================================
// Event filling handling Pixelman files
//=============================================================================
void TbCombatBuilder::fillEventFromPixelmanFile(const int armID) {
  // Check we havent reached the next event.
  bool sameEvent = true;  
  while (sameEvent) {
    std::string line;
    if (armID == 0)
      m_neof = std::getline(m_dataStream0, line);
    else
      m_neof = std::getline(m_dataStream1, line);
    if (!m_neof) break;
    if (line.substr(0, 1) == "#")
      sameEvent = false;
    else
      fillHit(line, armID);
  }
}

//=============================================================================
// Event filling handling RelaxD files
//=============================================================================
void TbCombatBuilder::fillEventFromRelaxDFile() {

//For the RelaxD files, in one event the first series of hits come from the upper arm and the second from the downstream arm.
     for(int iArm=0;iArm<m_nArms;iArm++){
        // Check we havent reached the next event.
        bool sameEvent = true;
        bool frameStart = false;
        while (sameEvent) {
               std::string line;
               m_neof = std::getline(m_dataStream0, line);
               if (!m_neof) break;
               if (line.substr(0,1) != "#"){
                   frameStart= true;
                   fillHit(line, iArm);
               }
               if (line.substr(0, 1) == "#"&& frameStart==true)
                   sameEvent = false;
        }
     }
}
//=============================================================================
// Event filling
//=============================================================================
void TbCombatBuilder::fillHit(std::string line, int armID) {
  std::stringstream lineStream;
  lineStream << line;
  int rowTemp, colTemp, ToT;
  lineStream >> colTemp >> rowTemp >> ToT;

  int plane;
  if (colTemp > 255) {
    if (rowTemp > 255)
      plane = 1;
    else
      plane = 3;
  } else {
    if (rowTemp > 255)
      plane = 0;
    else
      plane = 2;
  }

  // Arm modification.
  if (armID == 1) plane += 4;

  // Make the hit.
  LHCb::TbHit* hit = new LHCb::TbHit();
  hit->setRow(rowTemp % m_nPixels);
  hit->setCol(colTemp % m_nPixels);
  hit->setToT(ToT);
  hit->setPixelAddress(hit->row() + hit->col() * m_nPixels);
  hit->setTime(m_fakeTime);
  hit->setHtime(timingSvc()->globalToLocal(hit->time()));

  m_hits[plane]->insert(hit);
}

//=============================================================================
// End
//=============================================================================
