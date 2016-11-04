// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/Aida2ROOT.h"

// Local
#include "TbTestMC.h"

/** @file TbTestMC.cpp
 *
 *  Implementation of class : TbTestMC
 *	Author: Dan Saunders
 */

DECLARE_ALGORITHM_FACTORY(TbTestMC)

//=============================================================================
/// Standard constructor
//=============================================================================
TbTestMC::TbTestMC(const std::string& name, ISvcLocator* pSvcLocator)
    : GaudiTupleAlg(name, pSvcLocator) {
  declareProperty("InitAlignment", m_filename = "Alignment_raw.dat");
  declareProperty("doMisAlign", m_misalign = false);
  declareProperty("NTracks", m_nTracks = 100); // Per event. Homogenous in t.
  
  declareProperty("RunDuration", m_EventLength = 10000); // Max clock time per event.
  declareProperty("NNoiseClusters", m_nNoiseClusters = 20); // Per event, per plane. Homogenous in t.


  declareProperty("HitTimeJitter", m_HitTimeJitter = 10); // Time jitter width (clocks).
  declareProperty("ClusterPosnError", m_ClustPosnResolution = 0.01);

  declareProperty("ChargeSharingWidth", m_ChargeSharingWidth = 0.01);
  declareProperty("ThresholdCut", m_ThresholdCut = 50.0);
  declareProperty("ClusterCharge", m_charge = 300.0);
  declareProperty("ForceEfficiency", m_ForceEfficiency = true);
  // ...A measure of charge sharing - see below.

  m_ChipWidth = 14.08;  // mm
  m_PosnSpread = 4.0;   // mm

  m_ChargeSigma = 150;
  m_Pitch = 0.055;
  m_nEvents = 0;
  m_nExportedHits = 0;
  m_nExportedTracks = 0;
  m_ExportHits = true;
  m_ExportClusters = false;
  m_ExportTracks = false;
}

//=============================================================================
/// Initialization
//=============================================================================
StatusCode TbTestMC::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize();
  if (sc.isFailure()) return sc;

  m_trackFit = tool<ITbTrackFit>("TbTrackFit", "Fitter", this);
  setHistoTopDir("Tb/");

  if (m_misalign) {
    geomSvc()->readConditions(m_filename, m_modules);
    geomSvc()->printAlignment(m_modules);
  }

  else m_modules = geomSvc()->modules();
  m_nPlanes = m_modules.size();

  m_Hits.resize(m_nPlanes);

  // Initialize the random number generators.
  sc = m_uniform.initialize(randSvc(), Rndm::Flat(0.0, 1.0));
  if (!sc) {
    error() << "Cannot initialize uniform random number generator." << endmsg;
    return sc;
  }
  sc = m_gauss.initialize(randSvc(), Rndm::Gauss(0.0, 1.0));
  if (!sc) {
    error() << "Cannot initialize Gaussian random number generator." << endmsg;
    return sc;
  }
  sc = m_landau.initialize(randSvc(), Rndm::Landau(m_charge, m_ChargeSigma));
  if (!sc) {
    error() << "Cannot initialize Landau random number generator." << endmsg;
    return sc;
  }
  sc = m_gauss2.initialize(randSvc(), Rndm::Gauss(0.0, 1.0e-4));
  if (!sc) {
    error() << "Cannot initialize Gaussian random number generator." << endmsg;
    return sc;
  }
  sc = m_uniform2.initialize(randSvc(), Rndm::Flat(0,m_ChipWidth ));
  if (!sc) {
    error() << "Cannot initialize uniform random number generator." << endmsg;
    return sc;
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
/// Main execution
//=============================================================================
StatusCode TbTestMC::execute() {
  if (m_nEvents % 10 == 0)
    std::cout << "Event number: " << m_nEvents << std::endl;

  generate_tracks();
  createClustFromTrack();

  //make_noise();
  //make_tracks();

  sort_stuff();
  add_to_TES();

  m_nEvents++;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    m_nExportedHits += m_Hits[i].size();
    // Prepare for next event.
    m_Hits[i].clear();
  }
  m_Clusters.clear();
  m_Tracks.clear();
  return StatusCode::SUCCESS;
}

//=============================================================================
// Finalize
//=============================================================================
StatusCode TbTestMC::finalize() {

  info() << "------------------------------------------------------" << endmsg;
  info() << "                      TbTestMC                      " << endmsg;
  info() << "------------------------------------------------------" << endmsg;
  info() << "Number of exported hits:\t"<<m_nExportedHits<<endmsg;
  info() << "------------------------------------------------------" << endmsg;
  // Finalise the base class.
  return GaudiTupleAlg::finalize();

}

//=============================================================================
/// Sorting
//=============================================================================
void TbTestMC::add_to_TES() {
  // Hits.
  if (m_ExportHits) ExportHits();

  // Clusters.
  if (m_ExportClusters) {
    LHCb::TbClusters* clusters = new LHCb::TbClusters();
    for (std::vector<LHCb::TbCluster*>::iterator ic = m_Clusters.begin();
         ic != m_Clusters.end(); ic++)
      clusters->insert(*ic);

    put(clusters, LHCb::TbClusterLocation::Default);
  }

  // Tracks.
  if (m_ExportTracks) {
    LHCb::TbTracks* tracks = new LHCb::TbTracks();
    for (std::vector<LHCb::TbTrack*>::iterator it = m_Tracks.begin();
         it != m_Tracks.end(); it++)
      tracks->insert(*it);

    put(tracks, LHCb::TbTrackLocation::Default);
  }
}

//=============================================================================
/// Export hits to different TES locations.
//=============================================================================
void TbTestMC::ExportHits() {
  std::vector<LHCb::TbHits*> all_hits;
  for (unsigned int i = 0; i < m_nPlanes; i++) {
    LHCb::TbHits* hits = new LHCb::TbHits();
    put(hits, LHCb::TbHitLocation::Default + std::to_string(i));
    for (std::vector<LHCb::TbHit*>::iterator ih = m_Hits[i].begin();
         ih != m_Hits[i].end(); ih++) {
      hits->insert(*ih);
    }
  }
}

//=============================================================================
/// Sorting - no sorting if not exporting.
//=============================================================================
void TbTestMC::sort_stuff() {
  sort_by_chip();
  if (m_ExportHits) {
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      hit_torder(i);
    }
  }
  if (m_ExportClusters) cluster_torder();
  if (m_ExportTracks) track_torder();
}

//=============================================================================
/// Sorting by chip
//=============================================================================
void TbTestMC::sort_by_chip() {
  std::vector<LHCb::TbCluster*> old_Clusters(m_Clusters);
  m_Clusters.clear();
  std::vector<TbModule*>::iterator m;
  for (unsigned int iplane = 0; iplane < m_nPlanes; iplane++) {
    std::vector<LHCb::TbCluster*>::iterator ic;
    for (ic = old_Clusters.begin(); ic != old_Clusters.end(); ++ic) {
      if ((*ic)->plane() == (unsigned int)iplane) m_Clusters.push_back(*ic);
    }
  }

}

//=============================================================================
/// Track time ordering - honeycomb
//=============================================================================
void TbTestMC::track_torder() {
  int N = m_Tracks.size();
  float s_factor = 1.3;
  LHCb::TbTrack* track1, *track2;
  int gap = N / s_factor;
  bool swapped = false;

  // Start the swap loops.
  while (gap > 1 || swapped) {
    if (gap > 1) gap /= s_factor;

    swapped = false;  // Reset per swap loop.

    // Do the swaps.
    for (int i = 0; gap + i < N; ++i) {
      track1 = m_Tracks[i];
      track2 = m_Tracks[i + gap];

      if (track1->time() > track2->time()) {
        // Do the swap.
        m_Tracks[i] = track2;
        m_Tracks[i + gap] = track1;

        swapped = true;
      }
    }
  }
}

//=============================================================================
/// Hit time ordering - honeycomb
//=============================================================================
void TbTestMC::hit_torder(const unsigned int plane) {
  int N = m_Hits[plane].size();
  float s_factor = 1.3;
  LHCb::TbHit* hit1, *hit2;
  int gap = N / s_factor;
  bool swapped = false;

  // Start the swap loops.
  while (gap > 1 || swapped) {
    if (gap > 1) gap /= s_factor;

    swapped = false;  // Reset per swap loop.

    // Do the swaps.
    for (int i = 0; gap + i < N; ++i) {
      hit1 = m_Hits[plane][i];
      hit2 = m_Hits[plane][i + gap];

      if (hit1->time() > hit2->time()) {
        // Do the swap.
        m_Hits[plane][i] = hit2;
        m_Hits[plane][i + gap] = hit1;

        swapped = true;
      }
    }
  }
}

//=============================================================================
/// Cluster time ordering - honeycomb
//=============================================================================
void TbTestMC::cluster_torder() {
  int N = m_Clusters.size();
  float s_factor = 1.3;
  LHCb::TbCluster* clust1, *clust2;
  int gap = N / s_factor;
  bool swapped = false;

  // Start the swap loops.
  while (gap > 1 || swapped) {
    if (gap > 1) gap /= s_factor;

    swapped = false;  // Reset per swap loop.

    // Do the swaps.
    for (int i = 0; gap + i < N; ++i) {
      clust1 = m_Clusters[i];
      clust2 = m_Clusters[i + gap];

      if (clust1->time() > clust2->time() &&
          clust1->plane() == clust2->plane()) {

        // Do the swap.
        m_Clusters[i] = clust2;
        m_Clusters[i + gap] = clust1;

        swapped = true;
      }
    }
  }
}

//=============================================================================
/// Noise cluster posn
//=============================================================================
Gaudi::XYZPoint TbTestMC::ClustGposn(int iplane) {
  float x, y;
  bool on = true;
  while (on) {
    x = 0.5 * m_ChipWidth + m_gauss() * m_PosnSpread;
    y = 0.5 * m_ChipWidth + m_gauss() * m_PosnSpread;

    if (x > 0.028 + (2 * 0.055) && x < 14.05 - (2 * 0.055) &&
        y > 0.028 + (2 * 0.055) && y < 14.05 - (2 * 0.055))
      on = false;
  }  // NO EDGES TAKEN INTO ACCOUNT YET.

  Gaudi::XYZPoint posn(x, y, m_modules.at((unsigned int)iplane)->z());
  return posn;
}

//=============================================================================
/// Noise cluster charge
//=============================================================================
int TbTestMC::ClustCharge() {
  float charge;
  bool on = true;
  while (on) {
    charge = m_landau();
    if (charge > 3.0 && charge < 1000) on = false;
  }
  return (int)charge;
}

//=============================================================================
/// Tracks
//=============================================================================
void TbTestMC::make_tracks() {
  for (int i = 0; i < m_nTracks; i++) {
    // Make a track at a particular position. Clusters and hits saved auto.
    int time = (int)(m_uniform() * m_EventLength);
    LHCb::TbTrack* track = make_track(ClustGposn(0), time, i);
    m_Tracks.push_back(track);

    unsigned int nActualClusters = 0;
    for (unsigned int i=0; i<track->clusters().size(); i++) {
    	if (track->clusters()[i]->size() != 0) nActualClusters++;
    }

    if (nActualClusters == m_nPlanes || nActualClusters == m_nPlanes-1)
    		m_nExportedTracks++;
  }
}

//=============================================================================
/// Track
//=============================================================================
LHCb::TbTrack* TbTestMC::make_track(Gaudi::XYZPoint p, float t, int itrack) {
  LHCb::TbTrack* track = new LHCb::TbTrack();
  for (unsigned int iplane = 0; iplane < m_nPlanes; iplane++) {
    Gaudi::XYZPoint ClustPosn(p.x(), p.y(), m_modules.at(iplane)->z());
    LHCb::TbCluster* cluster = make_cluster(ClustPosn, t, iplane, itrack);
    track->addToClusters(cluster);
    m_Clusters.push_back(cluster);
  }
  m_trackFit->fit(track);
  return track;
}

//=============================================================================
/// Noise
//=============================================================================
void TbTestMC::make_noise() {
  for (unsigned int iplane = 0; iplane < m_nPlanes; iplane++) {
    for (int i = 0; i < m_nNoiseClusters; i++) {
      // Make a cluster at a particular position. Hits saved auto.
      int time = (int)(m_uniform() * m_EventLength);
      LHCb::TbCluster* cluster = make_cluster(ClustGposn(iplane), time, iplane, -1);
      m_Clusters.push_back(cluster);
    }
  }
}

//=============================================================================
/// MC cluster maker.
//=============================================================================
LHCb::TbCluster* TbTestMC::make_cluster(Gaudi::XYZPoint pGlobal_perfect,
                                        float t, int iplane, int track_tag) {
  LHCb::TbCluster* clust = new LHCb::TbCluster();
  clust->setTime(t);

  double x = pGlobal_perfect.x();
  double y = pGlobal_perfect.y();

  
  x += m_gauss() * m_ClustPosnResolution;
  y += m_gauss() * m_ClustPosnResolution;

  Gaudi::XYZPoint pGlobal_actual(x, y, pGlobal_perfect.z());

  Gaudi::XYZPoint pLocal = geomSvc()->globalToLocal(pGlobal_actual, iplane);
  clust->setXloc(pLocal.x());
  clust->setYloc(pLocal.y());

  //std::cout<<clust->xloc()<<"\t"<<pGlobal_actual.x()<<"\t"<<clust->yloc()<<"\t"<<pGlobal_actual.y()<<std::endl;

  clust->setX(pGlobal_actual.x());
  clust->setY(pGlobal_actual.y());
  clust->setZ(pGlobal_actual.z());

  clust->setPlane(iplane);
  
  setClusterHitsAndCharge(clust, track_tag); // Also sets Charge.

  return clust;
}

//=============================================================================
/// Cluster hits
//=============================================================================
void TbTestMC::setClusterHitsAndCharge(LHCb::TbCluster* clust, int /*track_tag*/) {
  // This version will model the charge cloud as a gaussian that is ceneted
  // at the cluster position, and whose height is set by the cluster charge
  // (albeit abit sneakily). Width is set above. The charge of surrounding
  // pixels is then taken to be the height of the gaussian at that pixels
  // position (if passing the threshold - kind of zero suppression);
  // like poor mans integration.
  double SigSq = m_ChargeSharingWidth*m_ChargeSharingWidth;
  int clustChargeish = ClustCharge();
  double N = clustChargeish*0.4;
  int SeedHitX = (int)(clust->xloc()/m_Pitch);
  int SeedHitY = (int)(clust->yloc()/m_Pitch);


  int ClustCharge = 0;
  int HalfAreaSide = 1; // 3x3
  // Search over area around the seed.
  //std::cout<<"New cluster at:\t"<<clust->xloc()<<"\t"<<clust->yloc()<<std::endl;
  for (int ix=SeedHitX-HalfAreaSide; ix!=SeedHitX+HalfAreaSide+1; ix++){
	for (int iy=SeedHitY-HalfAreaSide; iy!=SeedHitY+HalfAreaSide+1; iy++){
	  // Find distance to the pixel from the center of the gaussian.
	  double delx = (double(ix)+0.5)*m_Pitch - clust->xloc();
      double dely = (double(iy)+0.5)*m_Pitch - clust->yloc();
	  double delrSq = delx*delx + dely*dely;
	  int pixToT = (int)N*exp(-delrSq/SigSq);


	  if ((float)pixToT > m_ThresholdCut) {
	    LHCb::TbHit * hit = new LHCb::TbHit();
            hit->setDevice(clust->plane());
		hit->setCol(ix);
		hit->setRow(iy);
		hit->setToT(pixToT);
		hit->setTime(clust->time() + m_gauss() * m_HitTimeJitter);
		clust->addToHits(hit);
		m_Hits[clust->plane()].push_back(hit);

		// hit->setTrackTag(track_tag);
		ClustCharge += pixToT;
	  }
	}
  }

  if (clust->size()==0 && m_ForceEfficiency){ // i.e. none of the hits passed the thresehold
	LHCb::TbHit * hit = new LHCb::TbHit();
	hit->setCol(SeedHitX);
	hit->setRow(SeedHitY);
	hit->setToT(clustChargeish);
	hit->setTime(clust->time() + m_gauss() * m_HitTimeJitter);

	// hit->setTrackTag(track_tag);

	clust->addToHits(hit);
	m_Hits[clust->plane()].push_back(hit);
	ClustCharge += clustChargeish;
  }
  clust->setCharge(ClustCharge);
}

//=============================================================================
// Randomly generates tracks (C.Hombach)
//=============================================================================

void TbTestMC::generate_tracks() 
{
  double slope_xz;
  double slope_yz;
  double inter_x;
  double inter_y;
  
for (int i = 0; i < m_nTracks; i++) {  
  //Generate slopes and intercepts equally distributed over the sensor
  slope_xz = m_gauss2();
  slope_yz = m_gauss2();
  inter_x  = m_uniform2();
  inter_y  = m_uniform2();
  plot(slope_xz, "slopeXZ", "slopeXZ", -1.e-3,1.e-3,1000);
  plot(slope_yz, "slopeYZ", "slopeYZ", -1.e-3,1.e-3,1000);
  //Create track
  LHCb::TbState state;// = new LHCb::TbState();
  state.setTx(slope_xz);
  state.setTy(slope_yz);
  state.setX(inter_x);
  state.setY(inter_y);
  
  LHCb::TbTrack* track = new LHCb::TbTrack();
  track->setFirstState(state);
  m_Tracks.push_back(track);
  
 }
}

//=============================================================================
//Calculates cluster from track intercepts with module planes
//=============================================================================

void TbTestMC::createClustFromTrack()
{
  std::vector<LHCb::TbTrack*>::iterator it = m_Tracks.begin();
  for( ; it != m_Tracks.end(); ++it)
  {
    int time = (int)(m_uniform() * m_EventLength);
    std::vector<TbModule*>::iterator im = m_modules.begin();
    for( ; im != m_modules.end(); ++im)
    {
      
      Gaudi::XYZPoint ip = getIntercept(im - m_modules.begin(), (*it));
      Gaudi::XYZPoint pLocal = geomSvc()->globalToLocal(ip, im - m_modules.begin());
            
      LHCb::TbCluster* clust = new LHCb::TbCluster();
      clust->setTime(time);
      clust->setXloc(pLocal.x());
      clust->setYloc(pLocal.y());      
      clust->setX(ip.x());
      clust->setY(ip.y());
      clust->setZ(ip.z());
      clust->setCharge(ClustCharge());
      clust->setPlane(im - m_modules.begin());
      
      setClusterHitsAndCharge(clust,-1);
      m_Clusters.push_back(clust);
    }
    
  }
  
  
}

//============================================================================
// Calculates Intercept of track and a module (Needs to be moved somewhere else)
//============================================================================

Gaudi::XYZPoint TbTestMC::getIntercept(const unsigned int& plane, LHCb::TbTrack* track)
{
  Gaudi::XYZPoint p1Local(0., 0., 0.), p2Local(0., 0., 1.);
  Gaudi::XYZPoint p1Global =
    geomSvc()->localToGlobal(p1Local, plane);
  Gaudi::XYZPoint p2Global =
    geomSvc()->localToGlobal(p2Local, plane);
  Gaudi::XYZPoint normal(p2Global.x() - p1Global.x(),
                         p2Global.y() - p1Global.y(),
                         p2Global.z() - p1Global.z());
  //  std::cout << track->firstState().tx() << std::endl;
  const double dir_r =
    sqrt(track->firstState().tx() * track->firstState().tx() +
         track->firstState().ty() * track->firstState().ty() + 1.);
  const double dir_x = track->firstState().tx() / dir_r;
  const double dir_y = track->firstState().ty() / dir_r;
  const double dir_z = 1. / dir_r;
  const double length =
    ((p1Global.x() - track->firstState().x()) * normal.x() +
     (p1Global.y() - track->firstState().y()) * normal.y() +
     (p1Global.z() - 0.) * normal.z()) /
    (dir_x * normal.x() + dir_y * normal.y() + dir_z * normal.z());
  const double x_inter = track->firstState().x() + dir_x * length;
  const double y_inter = track->firstState().y() + dir_y * length;
  const double z_inter = 0. + dir_z * length;
  Gaudi::XYZPoint intersect_global(x_inter, y_inter, z_inter);
  
  return intersect_global;
  
}


//=============================================================================
/// End
//=============================================================================
