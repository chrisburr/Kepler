// Tb/TbEvent
#include "Event/TbCluster.h"
#include "Event/TbTrack.h"
#include "Event/TbNode.h"

// Local
#include "TbTrackFit.h"

DECLARE_TOOL_FACTORY(TbTrackFit)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbTrackFit::TbTrackFit(const std::string& type, const std::string& name,
                       const IInterface* parent)
    : GaudiTool(type, name, parent) {

  declareInterface<ITbTrackFit>(this);

  declareProperty("MaskedPlanes", m_maskedPlanes = {});
}

//=============================================================================
// Destructor
//=============================================================================
TbTrackFit::~TbTrackFit() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbTrackFit::initialize() {

  // Initialise the base class.
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;
  // Get the number of telescope planes.
  const auto nPlanes = geomSvc()->modules().size();
  // Set the flags whether a plane is masked or not.
  m_masked.resize(nPlanes, false);
  for (const unsigned int plane : m_maskedPlanes) {
    m_masked[plane] = true;
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Exclude a plane from the fit
//=============================================================================
void TbTrackFit::maskPlane(const unsigned int plane) {

  if (plane < m_masked.size()) m_masked[plane] = true;
}

//=============================================================================
// (Re-)include a plane in the fit
//=============================================================================
void TbTrackFit::unmaskPlane(const unsigned int plane) {

  if (plane < m_masked.size()) m_masked[plane] = false;
}

//=========================================================================
/// Perform a straight-line fit to the clusters of a given track
//=========================================================================
void TbTrackFit::fit(LHCb::TbTrack* track) {
  
  track->setFitStatus(LHCb::TbTrack::None);
  track->clearNodes();
  // Sums for the x fit
  double s0 = 0.;
  double sx = 0.;
  double sz = 0.;
  double sxz = 0.;
  double sz2 = 0.;
  // Sums for the y fit
  double u0 = 0.;
  double uy = 0.;
  double uz = 0.;
  double uyz = 0.;
  double uz2 = 0.;

  // Count the number of planes included in the fit.
  unsigned int nd = 0;

  // Loop through the clusters.
  auto clusters = track->clusters();
  for (auto it = clusters.cbegin(), end = clusters.cend(); it != end; ++it) {
    if (!(*it)) continue;
    // Skip masked planes.
    if (m_masked[(*it)->plane()]) continue;
    ++nd;
    const double wx = (*it)->wx();
    const double wy = (*it)->wy();
    const double x = (*it)->x();
    const double y = (*it)->y();
    const double z = (*it)->z();

    // Straight line fit in x
    s0 += wx;
    sx += wx * x;
    sz += wx * z;
    sxz += wx * x * z;
    sz2 += wx * z * z;
    // Straight line fit in y
    u0 += wy;
    uy += wy * y;
    uz += wy * z;
    uyz += wy * y * z;
    uz2 += wy * z * z;
  }
  if (nd < 3) {
    Error("Invalid track. Only " + std::to_string(nd) + 
          " non-masked clusters").ignore();
    return;
  }

  // Compute the track parameters for x.
  double den = (sz2 * s0 - sz * sz);
  if (fabs(den) < 10e-10) den = 1.;
  const double tx = (sxz * s0 - sx * sz) / den;
  const double x0 = (sx * sz2 - sxz * sz) / den;

  // Compute the track parameters for y.
  den = (uz2 * u0 - uz * uz);
  if (fabs(den) < 10e-10) den = 1.;
  const double ty = (uyz * u0 - uy * uz) / den;
  const double y0 = (uy * uz2 - uyz * uz) / den;

  // Calculate the covariance matrix at z = 0.
  const double denx = sz2 * s0 - sz * sz;
  const double deny = uz2 * u0 - uz * uz;

  Gaudi::SymMatrix4x4 fcov;
  fcov(0, 0) =  sz2 / denx;
  fcov(2, 0) = -sz / denx;
  fcov(2, 2) =  s0 / denx;
  fcov(1, 1) =  uz2 / deny;
  fcov(3, 1) = -uz / deny;
  fcov(3, 3) =  u0 / deny;

  // Create the first state.
  LHCb::TbState fstate(Gaudi::Vector4(x0, y0, tx, ty), fcov, 0., 0);
  track->setFirstState(fstate);

  // Compute chi2 and track time.
  double chi2 = 0.;
  double time = 0.;
  for (auto it = clusters.cbegin(), end = clusters.cend(); it != end; ++it) {
    if (!(*it)) continue;
    const unsigned int plane = (*it)->plane();
    // Skip masked planes.
    if (m_masked[plane]) continue;
    const double wx = (*it)->wx();
    const double wy = (*it)->wy();
    // Calculate global (biased) residuals in x and y.
    /*
    const Gaudi::XYZPoint intercept = geomSvc()->intercept(track, plane);
    const Gaudi::XYZPoint local = geomSvc()->globalToLocal(intercept, plane);
    const double dx = (*it)->xloc() - local.x();
    const double dy = (*it)->yloc() - local.y();
    */
    const double z = (*it)->z();
    const double x = x0 + tx * z;
    const double y = y0 + ty * z;
    const double dx = (*it)->x() - x;
    const double dy = (*it)->y() - y;
    chi2 += (dx * dx) * wx + (dy * dy) * wy;
    time += (*it)->htime();
    // Calculate the covariance matrix.
    Gaudi::SymMatrix4x4 cov;
    cov(0, 0) = (sz2 - 2 * z * sz + z * z * s0) / denx;
    cov(1, 1) = (uz2 - 2 * z * uz + z * z * u0) / deny;
    cov(2, 0) = -(sz - z * s0) / denx;
    cov(3, 1) = -(uz - z * u0) / deny;
    cov(2, 2) = s0 / denx; 
    cov(3, 3) = u0 / deny;
    // Create a state.
    LHCb::TbState state(Gaudi::Vector4(x, y, tx, ty), cov, z, plane); 
    // Add the node (measurement and state) to the track.
    track->addToNodes(LHCb::TbNode(*(*it), state));
  }
  // Set track chi2PerNdof and ndof.
  const unsigned int ndof = 2 * nd - 4;
  track->setNdof(ndof);
  track->setChi2PerNdof(chi2 / (double)ndof);
  // Update the fit status.
  track->setFitStatus(LHCb::TbTrack::StraightLine);
  // Finally, set the track time.
  time /= nd;
  track->setHtime(time);
}
