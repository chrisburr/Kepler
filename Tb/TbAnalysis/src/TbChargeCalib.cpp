// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/HistoLabels.h"

// Tb/TbEvent
#include "Event/TbTrack.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbChargeCalib.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbChargeCalib)

//=============================================================================
// Standard constructor
//=============================================================================
TbChargeCalib::TbChargeCalib(const std::string& name, 
                                     ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator) {

  declareProperty("ClusterLocation",
                  m_clusterLocation = LHCb::TbClusterLocation::Default);

}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbChargeCalib::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  info() << "Booking histograms for Chargecalib ... " << endmsg;
  m_ToTHists.reserve(256*256);
  for( unsigned int i = 0 ; i < 256*256; ++i){
    const std::string name = "c=" + std::to_string( i/256 ) + ", r=" + std::to_string(i%256);
    if( i % 256 == 0 ) info() << "Booked histogram for column " << i << endmsg;
    m_ToTHists.push_back(book1D(name, name, 0.5, 200.5, 200));
    setAxisLabels(m_ToTHists[i], "ToT", "Entries");
  }
  info() << "Booked 60000 ish hists" << endmsg; 
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbChargeCalib::execute() {

   LHCb::TbClusters* clusters = getIfExists<LHCb::TbClusters>(m_clusterLocation+std::to_string(0));
   if (!clusters) {
     error() << "No clusters in " << m_clusterLocation << endmsg;
     return StatusCode::FAILURE;
   }
  for( const LHCb::TbCluster* c : *clusters  ){
    if( c->size() != 1 ) continue;
 
    unsigned int col = c->hits()[0]->col();
    unsigned int row = c->hits()[0]->row();
    unsigned int tot = c->hits()[0]->ToT();
    //info() << c->hits()[0] << endmsg;
    m_ToTHists[ col*256 + row ]->fill( tot );

  } 
  return StatusCode::SUCCESS;
}

