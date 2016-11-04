// ROOT
#include <TH1D.h>
#include <TProfile.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>

// AIDA
#include "AIDA/IHistogram1D.h"
#include "AIDA/IProfile1D.h"

// Gaudi
#include "GaudiUtils/Aida2ROOT.h"

// Local
#include "TbAlignmentBase.h"

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbAlignmentBase::TbAlignmentBase(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
    : GaudiHistoTool(type, name, parent),
      m_trackFit(nullptr),
      m_geomSvc(nullptr) {

  declareInterface<TbAlignmentBase>(this);

  declareProperty("TrackLocation", 
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("DOFs", m_dofs = {});
  declareProperty("ClearTracks", m_clearTracks = true);
  declareProperty("MaskedPlanes", m_maskedPlanes = {});
  declareProperty("Monitoring", m_monitoring = false);
  declareProperty("MaxChi2", m_maxChi2 = 100.);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbAlignmentBase::initialize() {

  // Initialise the base class.
  StatusCode sc = GaudiHistoTool::initialize();
  if (sc.isFailure()) return sc;
  m_modules = geomSvc()->modules();
  m_nPlanes = m_modules.size();
  m_masked.resize(m_nPlanes, false);
  for (const unsigned int plane : m_maskedPlanes) m_masked[plane] = true;

  // Set the degrees of freedom.
  if (m_dofs.size() != 6) {
    info() << "Using the default degrees of freedom:" << endmsg;
    if (m_dofsDefault.size() != 6) {
      // Default DoFs are not defined. Set them.
      m_dofsDefault = {true, true, false, true, true, true};
    }
    m_dofs = m_dofsDefault;
  } else {
    info() << "Using the following degrees of freedom:" << endmsg;
  }
  // Print the degrees of freedom.
  const std::vector<std::string> labels = {"Translation X", "Translation Y",
                                           "Translation Z", "Rotation X",
                                           "Rotation Y",    "Rotation Z"};
  for (unsigned int i = 0; i < 6; ++i) {
    const std::string on = m_dofs[i] ? "ON" : "OFF";
    info() << format("  %-14s  %-3s", labels[i].c_str(), on.c_str()) << endmsg;
  }

  // Set up the track fit tool.
  m_trackFit = tool<ITbTrackFit>("TbTrackFit", "Fitter", this);
  if (!m_trackFit) {
    return Error("Failed to initialise track fit.");
  }
  return StatusCode::SUCCESS;
}

void TbAlignmentBase::plotResiduals(std::vector<TbAlignmentTrack*>& tracks,
                                    const std::string& tag) {

  if (!m_monitoring) return;
  // Book histograms.
  std::vector<AIDA::IHistogram1D*> hResGlobalX;
  std::vector<AIDA::IHistogram1D*> hResGlobalY;
  std::vector<AIDA::IProfile1D*> hResGlobalXvsGlobalX;
  std::vector<AIDA::IProfile1D*> hResGlobalYvsGlobalY;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string plane = std::to_string(i);
    const std::string title = "Plane " + plane;
    // Distribution of unbiased global x-residuals.
    std::string name = tag + "/UnbiasedResiduals/GlobalX/Plane" + plane;
    double low(-0.2);
    double high(0.2);
    unsigned int bins(200);
    hResGlobalX.push_back(book1D(name, title, low, high, bins));
    // Distribution of unbiased global y-residuals.
    name = tag + "/UnbiasedResiduals/GlobalY/Plane" + plane;
    hResGlobalY.push_back(book1D(name, title, low, high, bins));
    // Profile of unbiased global x-residuals as function of global x.
    low = -20.;
    high = 20.;
    bins = 200;
    name = "UnbiasedResiduals/GlobalResXvsGlobalX/Plane" + plane;
    hResGlobalXvsGlobalX.push_back(
        bookProfile1D(name, title, low, high, bins));
    // Profile of unbiased global y-residuals as function of global y.
    name = "UnbiasedResiduals/GlobalResYvsGlobalY/Plane" + plane;
    hResGlobalYvsGlobalY.push_back(
        bookProfile1D(name, title, low, high, bins));
  }
  std::vector<double> ty(m_nPlanes, 0.);
  std::vector<double> yty(m_nPlanes, 0.);
  std::vector<double> y(m_nPlanes, 0.);
  std::vector<double> yy(m_nPlanes, 0.);
  std::vector<double> tyty(m_nPlanes, 0.);

  for (auto& at : tracks) {
    // Get the track object.
    LHCb::TbTrack* track = at->track();
    // Loop over the clusters on the track.
    auto clusters = track->clusters();
    for (auto ic = clusters.cbegin(), end = clusters.cend(); ic != end; ++ic) {
      const unsigned int plane = (*ic)->plane();
      const LHCb::TbCluster* cluster = *ic;
      // Refit the track without this cluster.
      m_trackFit->maskPlane(plane);
      m_trackFit->fit(track);
      // Calculate the global x and y residuals.
      const Gaudi::XYZPoint intercept = geomSvc()->intercept(track, plane);
      const auto pLocal = Gaudi::XYZPoint(cluster->xloc(), cluster->yloc(), 0);
      const auto pGlobal = geomSvc()->localToGlobal(pLocal, plane);
      const double dxug = pGlobal.x() - intercept.x();
      const double dyug = pGlobal.y() - intercept.y();

      ty[plane] += track->firstState().ty();
      yty[plane] += track->firstState().ty() * pGlobal.y();
      y[plane] += pGlobal.y();
      yy[plane] += pGlobal.y() * pGlobal.y();
      tyty[plane] += track->firstState().ty() * track->firstState().ty();

      hResGlobalX[plane]->fill(dxug);
      hResGlobalY[plane]->fill(dyug);
      hResGlobalXvsGlobalX[plane]->fill(pGlobal.x(), dxug);
      hResGlobalYvsGlobalY[plane]->fill(pGlobal.y(), dyug);
      m_trackFit->unmaskPlane(plane);
    }
    m_trackFit->fit(track);
    // Loop over the associated clusters.
    auto aclusters = track->associatedClusters();
    for (auto it = aclusters.cbegin(), end = aclusters.cend(); it != end; ++it) {
      const unsigned int plane = (*it)->plane();
      // Calculate the global x and y residuals.
      const Gaudi::XYZPoint intercept = geomSvc()->intercept(track, plane);
      const auto pLocal = Gaudi::XYZPoint((*it)->xloc(), (*it)->yloc(), 0);
      const auto pGlobal = geomSvc()->localToGlobal(pLocal, plane);
      const double dxug = pGlobal.x() - intercept.x();
      const double dyug = pGlobal.y() - intercept.y();
      hResGlobalX[plane]->fill(dxug);
      hResGlobalY[plane]->fill(dyug);
      hResGlobalXvsGlobalX[plane]->fill(pGlobal.x(), dxug);
      hResGlobalYvsGlobalY[plane]->fill(pGlobal.y(), dyug);
    }
  }

  if (msgLevel(MSG::DEBUG)) {
    const unsigned int nTracks = tracks.size();
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      y[i] /= nTracks;
      ty[i] /= nTracks;
      yty[i] /= nTracks;
      yy[i] /= nTracks;
      tyty[i] /= nTracks;
      info() << "Plane " << i << ": Pearson-coefficient = "
             << (yty[i] - y[i] * ty[i]) /
                    (sqrt(yy[i] - y[i] * y[i]) *
                     sqrt(tyty[i] - ty[i] * ty[i])) << endmsg;
    }
  }
  // Fit the residual distributions and print the fit results.
  const std::string line(85, '-');
  info() << line << endmsg; 
  info() << "Plane     Mean X [\u03bcm]       Mean Y [\u03bcm]      "
         << "Sigma X [\u03bcm]     Sigma Y [\u03bcm]" << endmsg; 
  info() << line << endmsg; 
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    auto hx = Gaudi::Utils::Aida2ROOT::aida2root(hResGlobalX[i]);
    auto hy = Gaudi::Utils::Aida2ROOT::aida2root(hResGlobalY[i]);
    if (hx->GetEntries() == 0 || hy->GetEntries() == 0) continue;
    TFitResultPtr rx = hx->Fit("gaus", "QS0");
    TFitResultPtr ry = hy->Fit("gaus", "QS0");
    if (!rx.Get() || !ry.Get()) continue;
    info() << std::setprecision(4);
    info() << format(" %3u   % 5.3f +/- %4.3f  % 5.3f +/- %4.3f ", i,
                     1.e3 * rx->Parameter(1), 1.e3 * rx->Error(1),
                     1.e3 * ry->Parameter(1), 1.e3 * ry->Error(1)) 
           << format(" %4.3f +/- %4.3f  %4.3f +/- %4.3f", 
                     1.e3 * rx->Parameter(2), 1.e3 * rx->Error(2),
                     1.e3 * ry->Parameter(2), 1.e3 * ry->Error(2)) << endmsg; 
  }
  // Fit the profiles (x/y residuals vs. global x/y) and print the results.
  info() << line << endmsg; 
  info() << "Plane     Slope X [\u03bcm]       Slope Y [\u03bcm]" << endmsg;
  info() << line << endmsg; 
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    auto xgx = Gaudi::Utils::Aida2ROOT::aida2root(hResGlobalXvsGlobalX[i]);
    auto ygy = Gaudi::Utils::Aida2ROOT::aida2root(hResGlobalYvsGlobalY[i]);
    if (xgx->GetEntries() == 0 || ygy->GetEntries() == 0) continue;
    TFitResultPtr rbx = xgx->Fit("pol1", "QS0");
    TFitResultPtr rby = ygy->Fit("pol1", "QS0");
    if (!rbx.Get() || !rby.Get()) continue;
    info() << std::setprecision(4);
    info() << format(" %3u   % 5.3f +/- %4.3f   % 5.3f +/- %4.3f ", i,
                     1.e3 * rbx->Parameter(1), 1.e3 * rbx->Error(1),
                     1.e3 * rby->Parameter(1), 1.e3 * rby->Error(1))
           << endmsg;
  }
  info() << line << endmsg; 

}

//=============================================================================
// Collect alignment tracks (called at each event).
//=============================================================================
StatusCode TbAlignmentBase::execute(
    std::vector<TbAlignmentTrack*>& alignmentTracks) {

  // Get the tracks.
  LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) {
    return Error("No tracks in " + m_trackLocation);
  }
  // Add them to the alignment track store.
  for (LHCb::TbTrack* track : *tracks) {
    if (track->chi2() > m_maxChi2 || isEdge( track ) ) continue ;  
    TbAlignmentTrack* alignmentTrack = new TbAlignmentTrack(track);
    alignmentTracks.push_back(alignmentTrack);
  }
  return StatusCode::SUCCESS;
}
