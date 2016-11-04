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

// Local
#include "TbTimewalkMonitor.h"

// ROOT
#include "TH1D.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbTimewalkMonitor)

  //=============================================================================
  // Standard constructor
  //=============================================================================
TbTimewalkMonitor::TbTimewalkMonitor(const std::string& name, 
    ISvcLocator* pSvcLocator)
  : TbAlgorithm(name, pSvcLocator) {

    declareProperty("TrackLocation",
        m_trackLocation = LHCb::TbTrackLocation::Default);
    declareProperty("WidthMin", m_widthMin = 37 );
    declareProperty("WidthMax", m_widthMax = 42 );
  }

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbTimewalkMonitor::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  // Book the histograms.
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string plane = std::to_string(i);
    const std::string title = geomSvc()->modules().at(i)->id();
    m_space.push_back(bookProfile1D("SpatialResidual/Plane"+plane,title,-0.5,255.5,256));

    m_spd2D.push_back(book2D("spd2D/Plane"+plane,title,-0.5,255.5,256,-100,100,20));
    m_twd.push_back(book2D("twd/Plane"+plane,title,1.,101.,100,0.,62.5,40));
    m_twdQ.push_back(book2D("twdQ/Plane"+plane,title,1000,10000,100,0.,62.5,40));
    m_twdQL.push_back(book2D("twdQL/Plane"+plane,title,1000,10000,100,0.,62.5,40));
    m_twdQR.push_back(book2D("twdQR/Plane"+plane,title,1000,10000,100,0.,62.5,40));

    setAxisLabels( m_twd[i], "ToT", "#Delta t [ns]");

    m_twdL.push_back(book2D("twdL/Plane"+plane,title,0.,100.,100,0.,62.5,40));
    m_twdR.push_back(book2D("twdR/Plane"+plane,title,0.,100.,100,0.,62.5,40));
    m_LRSYNC.emplace_back(bookProfile1D("LR_SYNC/Plane"+plane,title,-0.5,255.5,256));
    m_UDSYNC.emplace_back(bookProfile1D("UD_SYNC/Plane"+plane,title,-0.5,255.5,256));

    m_quad.emplace_back(bookProfile1D("QUAD/Plane"+plane,title,-0.5,255.5,256));

    m_inscol.emplace_back(bookProfile1D("Scol/Plane"+plane,title,-0.5,127.5,128));
    m_interscol.emplace_back(bookProfile1D("InterScol/Plane"+plane,title,-0.5,127.5,128));

    m_timewalk.push_back(bookProfile1D("Timewalk/Plane"+plane,title, 1., 401., 400));
    m_dtDist.push_back( book1D( "dtDist/Plane"+plane,title, 0., 90., 58 ) );
    m_cDist.push_back( book1D( "cDist/Plane"+plane,title,0.,90.,58 ) );
    m_timewalkQ.push_back(bookProfile1D("TimewalkQ/Plane"+plane,title,0,20000,200));
    setAxisLabels(m_timewalkQ[i], "charge [e^{-}]","#Delta t [ns]");
    setAxisLabels(m_timewalk[i], "ToT", "#Delta t [ns]");
  }

  return StatusCode::SUCCESS;
}

StatusCode TbTimewalkMonitor::finalize() {

  for( unsigned int i = 0 ; i < m_nPlanes; ++i ){
    TH1D* tdist = Gaudi::Utils::Aida2ROOT::aida2root( m_dtDist[i] );
    AIDA::IHistogram1D* cdist = m_cDist[i];
    int total = 0;    
    for (int i=0;i<tdist->GetNbinsX();i++)
    {
      total += tdist->GetBinContent(i);
      cdist->fill( tdist->GetBinCenter(i) , total /tdist->GetEntries() ) ;
    }    
  }
  return StatusCode::SUCCESS; 
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTimewalkMonitor::execute() {

  // Grab the tracks.

  const LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) return StatusCode::SUCCESS; 
  for (const LHCb::TbTrack* track : *tracks) {
 //   info() << track << endmsg; 
    const SmartRefVector<LHCb::TbCluster>& clusters = track->clusters();
    const auto ref = std::min_element(
        clusters.begin(), clusters.end(),
        [](const LHCb::TbCluster* h1,
          const LHCb::TbCluster* h2) { return h1->htime() < h2->htime(); });
    const double tref = (*ref)->htime();
    const double syncRef = (*clusters.rbegin())->htime();
    unsigned int p = (*clusters.rbegin())->plane();

    for (auto it = clusters.cbegin(), end = clusters.cend(); it != end; ++it){

      auto hits = (*it)->hits();
      for (const auto hit : hits) {
        if( p==5 ){
          m_space[(*it)->plane()]->fill( hit->col(), hit->htime() - syncRef );
          m_spd2D[(*it)->plane()]->fill( hit->col(), hit->htime() - syncRef );
        }
        m_timewalk[(*it)->plane()]->fill(hit->ToT(), hit->htime() - tref);
        m_twd[(*it)->plane()]->fill(hit->ToT(), hit->htime() - tref);
        m_timewalkQ[(*it)->plane()]->fill(hit->charge(), hit->htime() - tref );
        m_twdQ[(*it)->plane()]->fill(hit->charge(), hit->htime() - tref );
        m_dtDist[(*it)->plane()]->fill( hit->htime() - tref );   
      }
      if( (*it)->size() == 2 ){
        auto minmax = std::minmax_element(hits.begin(), hits.end(),[](LHCb::TbHit* h1, LHCb::TbHit* h2) { return h1->col() < h2->col(); });
        if( (*minmax.first)->col()  != (*minmax.second)->col() ){
          m_twdL[(*it)->plane()]->fill( (*minmax.first)->ToT(), (*minmax.first)->htime() - tref );
          m_twdR[(*it)->plane()]->fill( (*minmax.second)->ToT(), (*minmax.second)->htime() - tref );
          m_twdQL[(*it)->plane()]->fill( (*minmax.first)->charge(), (*minmax.first)->htime() - tref );
          m_twdQR[(*it)->plane()]->fill( (*minmax.second)->charge(), (*minmax.second)->htime() - tref );
        }
      }
    }
    const SmartRefVector<LHCb::TbCluster>& associatedClusters  = track->associatedClusters();

    for (auto it = associatedClusters.cbegin(), end = associatedClusters.cend(); it != end; ++it){
      auto hits = (*it)->hits();
      //info() << "Filling associated clusters for " << track << endmsg; 
      for (const auto hit : (*it)->hits()) {
        m_timewalk[(*it)->plane()]->fill(hit->ToT(), hit->htime() - tref);
        m_twd[(*it)->plane()]->fill(hit->ToT(), hit->htime() - tref);
        //m_twdQ[(*it)->plane()]->fill(hit->charge(), hit->htime() - tref );
        //m_dtDist[(*it)->plane()]->fill( hit->htime() - tref );
        m_timewalkQ[(*it)->plane()]->fill(hit->charge(), hit->htime() - tref );
        //if( (*it)->size() == 1 ) m_timewalkOneHit[(*it)->plane()]->fill(hit->ToT(), ( hit->htime() - tref ));
        //if( (*it)->size() == 2 ) m_timewalkTwoHit[(*it)->plane()]->fill(hit->ToT(), ( hit->htime() - tref ));
      }
      /*
      if( (*it)->size() == 2 ){
        auto minmax = std::minmax_element(hits.begin(), hits.end(),[](LHCb::TbHit* h1, LHCb::TbHit* h2) { return h1->col() < h2->col(); });
        if( (*minmax.first)->col()  != (*minmax.second)->col() ){
          m_twdL[(*it)->plane()]->fill( (*minmax.first)->ToT(), (*minmax.first)->htime() - tref );
          m_twdR[(*it)->plane()]->fill( (*minmax.second)->ToT(), (*minmax.second)->htime() - tref );
          m_twdQL[(*it)->plane()]->fill( (*minmax.first)->charge(), (*minmax.first)->htime() - tref );
          m_twdQR[(*it)->plane()]->fill( (*minmax.second)->charge(), (*minmax.second)->htime() - tref );
        }
      }
      */
      //info() << "Filled associated clusters " << endmsg; 
    }
  }

  return StatusCode::SUCCESS;
}
/*
   for(unsigned int plane=0;plane<m_nPlanes; ++plane ){
   const LHCb::TbClusters* clusters = 
   getIfExists<LHCb::TbClusters>(LHCb::TbClusterLocation::Default+std::to_string(plane));
   for( auto& cluster : *clusters ){
   if( cluster->size() == 2 ){
   auto hits = cluster->hits();
   auto minmax = std::minmax_element(hits.begin(), hits.end(),
   [](LHCb::TbHit* h1, LHCb::TbHit* h2) { return h1->col() < h2->col(); });
   const LHCb::TbHit* left = (*minmax.first);
   const LHCb::TbHit* right = (*minmax.second);
   if( left->col() != right->col() ){
   m_LRSYNC[plane]->fill( left->col() , left->htime() - right->htime() );
   if( int(left->col() /2 ) == int(right->col() /2 ) ) 
   m_inscol[plane]->fill( int(left->col()/2) , left->htime() - right->htime() );
   else 
   m_interscol[plane]->fill( int(left->col()/2) , left->htime() - right->htime() );
   }
   else {
   minmax = std::minmax_element(hits.begin(), hits.end(),
   [](LHCb::TbHit* h1, LHCb::TbHit* h2) { return h1->row() < h2->row(); });
   m_UDSYNC[plane]->fill( (*minmax.first)->row(), 
   (*minmax.first)->htime() - (*minmax.second)->htime());
   }
   }
   if( cluster->size() == 4 ){
   auto hits = cluster->hits();
   auto minmax = std::minmax_element(hits.begin(), hits.end(),
   [](LHCb::TbHit* h1, LHCb::TbHit* h2) { return h1->col() < h2->col(); });
   const LHCb::TbHit* left = (*minmax.first);
   const LHCb::TbHit* right = (*minmax.second);
   if( left->col() == right->col() + 4 ){
   m_quad[plane]->fill( left->col() , left->htime() - right->htime() );
   }
   }

   }
   }
   return StatusCode::SUCCESS;
   }
   */
