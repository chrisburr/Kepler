// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/HistoLabels.h"
#include "GaudiUtils/Aida2ROOT.h"

// Tb/TbEvent
#include "Event/TbTrack.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"
#include "TbKernel/TbFunctors.h"

// Local
#include "TbVertexing.h"

// ROOT
#include "TH1D.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbVertexing)

  template <class TYPE>
  class itlowerBound {
    public:
      bool operator()(TYPE lhs, const double t) const { return (*lhs)->htime() < t; }
  };

//=============================================================================
// Standard constructor
//=============================================================================
TbVertexing::TbVertexing(const std::string& name, 
    ISvcLocator* pSvcLocator)
  : TbAlgorithm(name, pSvcLocator) {
    declareProperty("TimeWindow",m_timeWindow);
    declareProperty("MaxDoca2",m_maxDoca2);
    declareProperty("TrackLocation", m_trackLocation = LHCb::TbTrackLocation::Default);
  }

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbVertexing::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  m_trackFit = tool<ITbTrackFit>("TbTrackFit", "Fitter", this);
  if( m_trackFit == NULL ){
    error() << "Failed to initialise trackfit" << endmsg;
    return StatusCode::FAILURE;
  }
  NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");
  NTuplePtr nt(ntupleSvc(), "/NTUPLES/FILE1/TbTupleWriter/Vertices");
  // Check if already booked.
  nt = ntupleSvc()->book("/NTUPLES/FILE1/TbTupleWriter/Vertices",
      CLID_ColumnWiseTuple, "nTuple of Vertices");
  nt->addItem("nPlanes", m_index, 0, 10);
  nt->addIndexedItem("X", m_index, m_vtxX);
  nt->addIndexedItem("Y", m_index, m_vtxY);
  nt->addIndexedItem("Z", m_index, m_vtxZ);
  nt->addItem("Matched", m_matched );
 // nt->addItem("Tk1Size",m_tkSize1);
 // nt->addItem("Tk2Size",m_tkSize2);
  return StatusCode::SUCCESS;
}

StatusCode TbVertexing::finalize() {

  return StatusCode::SUCCESS; 
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbVertexing::execute() {

  // Grab the tracks.

  const LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (tracks) {
    for( auto& track1 : *tracks ){

      const double tMin = track1->htime() - m_timeWindow;
      const double tMax = track1->htime() + m_timeWindow;
      auto track2 = std::lower_bound(tracks->begin(),
          tracks->end(),
          tMin, lowerBound<const LHCb::TbTrack*>());

      for(; track2 != tracks->end() && (*track2)->htime() < tMax ; ++track2 ){
        if( track1 == *track2 ) continue; 
        const double DOCA = doca2(track1,*track2);
        if( DOCA > m_maxDoca2 ) continue;
        /// track separation ///
        const double avgX = ( track1->firstState().tx() + (*track2)->firstState().tx() ) /2.;
        const double avgY = ( track1->firstState().ty() + (*track2)->firstState().ty() ) /2.;

        const double dt = pow ( ( track1->firstState().tx() - (*track2)->firstState().tx() ) / avgX, 2.0 ) +
          pow ( ( ( track1->firstState().ty() - (*track2)->firstState().ty() ) ) / avgY , 2.0 ) ; 
        auto chi2_pos = vertexPosition( {track1,*track2} );
        plot( sqrt(DOCA), "DOCA", 0, 0.5, 100 ); /// 500 micron 
        plot( dt, "Angle", -2., 2., 1000);
        plot( chi2_pos[0]  , "VertexChi2_X",0.,14.,100);
        plot( chi2_pos[1]  , "VertexChi2_Y",0.,14.,100);
        plot( chi2_pos[2]  , "VertexChi2_Z",-700.,650.,2500);
        m_vtxX[0] =  chi2_pos[0];
        m_vtxY[0] =  chi2_pos[1];
        m_vtxZ[0] =  chi2_pos[2];

        ntupleSvc()->writeRecord("/NTUPLES/FILE1/TbTupleWriter/Vertices");
        m_index = 10;
        m_matched = matched( track1, *track2 );
        for( unsigned int plane = 0 ; plane != 9 ; ++plane ){
          m_trackFit->maskPlane( plane );
          m_trackFit->fit( track1 );
          m_trackFit->fit( *track2 );
          //m_vtxX[plane+1] 
          auto up = vertexPosition( {track1,*track2});
          m_vtxX[plane+1] = up[0];
          m_vtxY[plane+1] = up[1];
          m_vtxZ[plane+1] = up[2];
          plot( up[2]  , "VertexChi2_Z_"+std::to_string(plane),-700.,650.,2500);
          m_trackFit->unmaskPlane(plane);
        }
        m_trackFit->fit( track1 );
        m_trackFit->fit( *track2);
      }
    }

  }

  return StatusCode::SUCCESS;
}

