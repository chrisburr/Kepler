// Boost
#include "boost/format.hpp"

// Local
#include "TbKernel/TbAlgorithm.h"

//=============================================================================
// Standard constructor
//=============================================================================
TbAlgorithm::TbAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : GaudiTupleAlg(name, pSvcLocator) {

  declareProperty("PrintConfiguration", m_printConfiguration = false);
  declareProperty("MaskedPlanes", m_maskedPlanes = {});

}

//=============================================================================
// Destructor
//=============================================================================
TbAlgorithm::~TbAlgorithm() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbAlgorithm::initialize() {

  // Initialise the base class.
  StatusCode sc = GaudiTupleAlg::initialize();
  if (sc.isFailure()) return sc;
  // Get the number of telescope planes.
  m_nPlanes = geomSvc()->modules().size();
  m_nDevices = geomSvc()->nDevices();
  // Set the flags whether a plane is masked or not.
  m_masked.resize(m_nPlanes, false);
  for (const unsigned int plane : m_maskedPlanes) {
    m_masked[plane] = true;
  }

  setHistoTopDir("Tb/");
  // If requested, print the properties specific to this algorithm.
  if (m_printConfiguration) {
    boost::format fmt(" %|-30.30s|%|32t| %s "); 
    const auto& props = this->getProperties();
    info() << std::string(70, '-') << endmsg;
    info() << "Configuration of " << this->name() << endmsg;
    info() << std::string(70, '-') << endmsg;
    for (auto it = props.crbegin(), end = props.crend(); it != end; ++it) {
      const std::string name = (*it)->name();
      // Stop when we reach the base class properties.
      if (name == "PrintConfiguration") break;
      info() << fmt % name % (*it)->toString() << endmsg;
    }
  }
  return StatusCode::SUCCESS;

}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbAlgorithm::execute() {

  return StatusCode::SUCCESS;

}

//=============================================================================
// Finalisation
//=============================================================================
StatusCode TbAlgorithm::finalize() {

  return GaudiTupleAlg::finalize();
  
}
