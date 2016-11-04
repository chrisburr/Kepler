// ROOT
#include "TMath.h"

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiUtils/HistoLabels.h"
#include "GaudiUtils/Aida2ROOT.h"

// Tb/TbEvent
#include "Event/TbHit.h"

// Tb/TbKernel
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbModule.h"

// Local
#include "TbTrackPlots.h"

using namespace Gaudi::Utils::Histos;

DECLARE_ALGORITHM_FACTORY(TbTrackPlots)

//=============================================================================
// Standard constructor
//=============================================================================
TbTrackPlots::TbTrackPlots(const std::string& name, ISvcLocator* pSvcLocator)
    : TbAlgorithm(name, pSvcLocator),
      m_parChi2("", 0., 30., 300),
      m_parResidualsXY("", -0.2, 0.2, 500),
      m_parResidualsT("", -50., 50., 2000),
      m_parXY("", -20, 20, 500),
      m_parSlope("", -0.0001, 0.0001, 500),
      m_trackFit(nullptr) {

  declareProperty("TrackLocation",
                  m_trackLocation = LHCb::TbTrackLocation::Default);
  declareProperty("TrackFitTool", m_trackFitTool = "TbTrackFit");
  declareProperty("RefitTracks", m_refit = false);
  declareProperty("PlotAngleBetweenArms", m_plotAngle = false);
 
  declareProperty("ParametersChi2", m_parChi2);
  declareProperty("ParametersResidualsXY", m_parResidualsXY);
  declareProperty("ParametersResidualsT", m_parResidualsT);
  declareProperty("ParametersXY", m_parXY);
  declareProperty("ParametersSlope", m_parSlope);
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbTrackPlots::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlgorithm::initialize();
  if (sc.isFailure()) return sc;
  // Setup the track fit tool.
  m_trackFit = tool<ITbTrackFit>(m_trackFitTool, "Fitter", this);
  if (!m_trackFit) return Error("Cannot retrieve track fit tool.");
  setupPlots();
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TbTrackPlots::execute() {

  // Grab the tracks.
  LHCb::TbTracks* tracks = getIfExists<LHCb::TbTracks>(m_trackLocation);
  if (!tracks) return Error("No tracks in " + m_trackLocation);

  for (LHCb::TbTrack* track : *tracks) {
    m_hChi2->fill(track->chi2PerNdof());
    m_hTrackSize->fill(track->size());
    m_hProb->fill(TMath::Prob(track->chi2(), track->ndof()));
    m_hSlopeXZ->fill(track->firstState().tx());
    m_hSlopeYZ->fill(track->firstState().ty());
    m_hFirstStateX->fill(track->firstState().x());
    m_hFirstStateY->fill(track->firstState().y());
    m_hSlopeXvsX->fill(track->firstState().tx(), track->firstState().x());
    m_hSlopeYvsY->fill(track->firstState().ty(), track->firstState().y());
    // Calculate the intercept of the track with the detector planes.
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      const Gaudi::XYZPoint intercept = geomSvc()->intercept(track, i);
      m_hIntercepts[i]->fill(intercept.x(), intercept.y());
      // Plot the intercepts of tracks with/without an associated trigger. 
      if (track->triggers().empty()) {
        m_hInterceptsNonAssociated[i]->fill(intercept.x(), intercept.y());
      } else {
        m_hInterceptsAssociated[i]->fill(intercept.x(), intercept.y());
      }
    }
    // if (track->chi2PerNdof() < 4.) { 
      if (m_refit) {
        fillResiduals(track);
      } else {
        fillResidualsFromNodes(track);
      }
    // }
    fillTimingPlots(track);

    if (!m_plotAngle) continue;
    // Delta angle.
    LHCb::TbTrack track1, track2;
    for (auto cluster : track->clusters()) {
      if (cluster->plane() < 4) {
        track1.addToClusters(cluster);
      } else {
        track2.addToClusters(cluster);
      }
    }
    if (track2.clusters().size() > 2 && track1.clusters().size() > 2) {
      m_trackFit->fit(&track1);
      m_trackFit->fit(&track2);
      const double dtx = track2.firstState().tx() - track1.firstState().tx();
      const double dty = track2.firstState().ty() - track1.firstState().ty();
      plot(dtx, "delAngleArmsX", "delAngleArmsX", -0.001, 0.001, 300);
      plot(dty, "delAngleArmsY", "delAngleArmsY", -0.001, 0.001, 300);
    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Fill timing plots.
//=============================================================================
void TbTrackPlots::fillTimingPlots(const LHCb::TbTrack* track) {

  const double t = track->htime();
  const auto triggers = track->triggers();
  for (auto trigger : triggers) {
    m_hTimeDifferenceTrackTrigger->fill(trigger->htime() - t);
  }

  const auto clusters = track->clusters();
  if (clusters.empty()) return;
  const double t0 = clusters.front()->htime();
  for (auto it = clusters.cbegin(), end = clusters.cend(); it != end; ++it) {
    const unsigned int plane = (*it)->plane();
    const double dt = (*it)->htime() - t;
    m_hBiasedResT[plane]->fill(dt);
    m_hBiasedResTvsGX[plane]->fill((*it)->x(), dt);

    auto hits = (*it)->hits();
    for (auto ith = hits.cbegin(), hend = hits.cend(); ith != hend; ++ith) {
      const double dth = (*ith)->htime() - t;
      m_hBiasedResPixelTvsColumn[plane]->fill((*ith)->col(), dth);
    }

    // Fill time synchronisation histograms, with respect to chip zero,
    // and also the synchronisation stability
    const double dt0 = t0 - (*it)->htime();
    m_hSyncDifferences[plane]->fill(dt0);
    m_hSyncInRun[plane]->fill(t0 / Tb::minute, dt0);
  }
}

//=============================================================================
// Fill residual distributions.
//=============================================================================
void TbTrackPlots::fillResidualsFromNodes(LHCb::TbTrack* track) {

  const auto nodes = track->nodes();
  for (const auto& node : track->nodes()) {
    const unsigned int plane = node.plane();

    // Fill the global unbiased residual histograms.
    const double dxug = node.residualX() * node.covX() / node.residualCovX();
    const double dyug = node.residualY() * node.covY() / node.residualCovY();
    m_hUnbiasedResGX[plane]->fill(dxug);
    m_hUnbiasedResGY[plane]->fill(dyug);
    m_hUnbiasedResGXvsLX[plane]->fill(node.cluster()->xloc(), dxug);
    m_hUnbiasedResGYvsLY[plane]->fill(node.cluster()->yloc(), dyug);
    m_hUnbiasedResGXvsGX[plane]->fill(node.cluster()->x(), dxug);
    m_hUnbiasedResGYvsGY[plane]->fill(node.cluster()->y(), dyug);

    // Fill the global biased residual histograms.
    const double dxg = node.residualX();
    const double dyg = node.residualY();
    m_hBiasedResGX[plane]->fill(dxg);
    m_hBiasedResGY[plane]->fill(dyg);
    m_hBiasedResGXvsLX[plane]->fill(node.cluster()->xloc(), dxg);
    m_hBiasedResGYvsLY[plane]->fill(node.cluster()->yloc(), dyg);
    m_hBiasedResGXvsGX[plane]->fill(node.cluster()->x(), dxg);
    m_hBiasedResGYvsGY[plane]->fill(node.cluster()->y(), dyg);

    const double trprob = TMath::Prob(track->chi2(), track->ndof());
    m_hBiasedResGXvsTrackProb[plane]->fill(trprob, dxg);
    m_hBiasedResGYvsTrackProb[plane]->fill(trprob, dyg);

    // Fill the pull distributions.
    const double resErrX = std::sqrt(node.residualCovX());
    const double resErrY = std::sqrt(node.residualCovY());
    m_hPullX[plane]->fill(dxg / resErrX);
    m_hPullY[plane]->fill(dyg / resErrY);

    // Fill the histograms for the residuals with unbiased RMS.
    const double dx = dxg * sqrt(node.covX() / node.residualCovX());
    const double dy = dyg * sqrt(node.covY() / node.residualCovY());
    m_hResolutionX[plane]->fill(dx);
    m_hResolutionY[plane]->fill(dy);

    continue;
    // Additional plots for tuning/debuging.
    const unsigned int cols = node.cluster()->cols();
    const unsigned int rows = node.cluster()->rows();
    const std::string pl = "Plane" + std::to_string(plane);
    const std::string namex = pl + "/Col" + std::to_string(cols);
    const std::string namey = pl + "/Row" + std::to_string(rows);
    plot(dxg / resErrX, "PullX/" + namex, pl, -5., 5., 100);
    plot(dyg / resErrY, "PullY/" + namey, pl, -5., 5., 100);
    plot(dx, "HitErrorX/" + namex, pl, -0.1, 0.1, 100);
    plot(dy, "HitErrorY/" + namey, pl, -0.1, 0.1, 100);
  }
}

//=============================================================================
// Fill residual distributions.
//=============================================================================
void TbTrackPlots::fillResiduals(LHCb::TbTrack* track) {

  const auto clusters = track->clusters();
  for (auto it = clusters.cbegin(), end = clusters.cend(); it != end; ++it) {
    const unsigned int plane = (*it)->plane();

    // Mask the plane under consideration to calculate the unbiased residuals.
    m_trackFit->maskPlane(plane);
    m_trackFit->fit(track);

    // Calculate the global unbiased residuals.
    const auto interceptUG = geomSvc()->intercept(track, plane);
    const double dxug = (*it)->x() - interceptUG.x();
    const double dyug = (*it)->y() - interceptUG.y();
    m_hUnbiasedResGX[plane]->fill(dxug);
    m_hUnbiasedResGY[plane]->fill(dyug);
    m_hUnbiasedResGXvsLX[plane]->fill((*it)->xloc(), dxug);
    m_hUnbiasedResGYvsLY[plane]->fill((*it)->yloc(), dyug);
    m_hUnbiasedResGXvsGX[plane]->fill((*it)->x(), dxug);
    m_hUnbiasedResGYvsGY[plane]->fill((*it)->y(), dyug);

    // Calculate the local unbiased residuals.
    const auto interceptUL = geomSvc()->globalToLocal(interceptUG, plane);
    const double dxul = (*it)->xloc() - interceptUL.x();
    const double dyul = (*it)->yloc() - interceptUL.y();
    m_hUnbiasedResLX[plane]->fill(dxul);
    m_hUnbiasedResLY[plane]->fill(dyul);
    plot(dxul, "ResX/Plane" + std::to_string((*it)->plane()) + "/Col" + std::to_string((*it)->cols()), "ResX", -0.1, 0.1, 200);
    plot(dyul, "ResY/Plane" + std::to_string((*it)->plane()) + "/Row" + std::to_string((*it)->rows()), "ResY", -0.1, 0.1, 200);

    // Re-include the plane under consideration in the fit and refit.
    m_trackFit->unmaskPlane(plane);
    m_trackFit->fit(track);

    // Calculate the global biased residuals in x and y.
    const Gaudi::XYZPoint interceptG = geomSvc()->intercept(track, plane);
    const double dxg = (*it)->x() - interceptG.x();
    const double dyg = (*it)->y() - interceptG.y();
    m_hBiasedResGX[plane]->fill(dxg);
    m_hBiasedResGY[plane]->fill(dyg);
    m_hBiasedResGXvsLX[plane]->fill((*it)->xloc(), dxg);
    m_hBiasedResGYvsLY[plane]->fill((*it)->yloc(), dyg);
    m_hBiasedResGXvsGX[plane]->fill((*it)->x(), dxg);
    m_hBiasedResGYvsGY[plane]->fill((*it)->y(), dyg);

    // Calculate the local biased residuals.
    const auto interceptL = geomSvc()->globalToLocal(interceptG, plane);
    const double dxl = (*it)->xloc() - interceptL.x();
    const double dyl = (*it)->yloc() - interceptL.y();
    m_hBiasedResLX[plane]->fill(dxl);
    m_hBiasedResLY[plane]->fill(dyl);

    const double trprob = TMath::Prob(track->chi2(), track->ndof());
    m_hBiasedResGXvsTrackProb[plane]->fill(trprob, dxg);
    m_hBiasedResGYvsTrackProb[plane]->fill(trprob, dyg);

  }
}

//=============================================================================
// Setup plots
//=============================================================================
void TbTrackPlots::setupPlots() {

  unsigned int bins = m_parChi2.bins();
  double low = m_parChi2.lowEdge();
  double high = m_parChi2.highEdge();
  m_hChi2 = book1D("Chi2PerDof", low, high, bins);
  setAxisLabels(m_hChi2, "#chi^{2} / n_{dof}", "entries");

  m_hProb = book1D("Probability", 0., 1., 1000);
  setAxisLabels(m_hProb, "probability", "entries");

  m_hTrackSize = book1D("TrackSize", 0.5, 12.5, 12);
  setAxisLabels(m_hTrackSize, "number of clusters", "entries");

  bins = m_parSlope.bins();
  low = m_parSlope.lowEdge();
  high = m_parSlope.highEdge();
  m_hSlopeXZ = book1D("SlopeXZ", low, high, bins);
  m_hSlopeYZ = book1D("SlopeYZ", low, high, bins);
  setAxisLabels(m_hSlopeXZ, "#it{t}_{x}", "entries");
  setAxisLabels(m_hSlopeYZ, "#it{t}_{y}", "entries");

  bins = m_parXY.bins();
  low = m_parXY.lowEdge();
  high = m_parXY.highEdge();
  m_hFirstStateX = book1D("FirstStateX", low, high, bins);
  m_hFirstStateY = book1D("FirstStateY", low, high, bins);

  m_hSlopeXvsX =
      book2D("SlopeXZvsFirstStateX", "t_{x} vs x", m_parSlope.lowEdge(),
             m_parSlope.highEdge(), m_parSlope.bins(), low, high, bins);
  m_hSlopeYvsY =
      book2D("SlopeYZvsFirstStateY", "t_{y} vs y", m_parSlope.lowEdge(),
             m_parSlope.highEdge(), m_parSlope.bins(), low, high, bins);

  setAxisLabels(m_hSlopeXvsX, "#it{t}_{x}", "#it{x}_{0} [mm]");
  setAxisLabels(m_hSlopeYvsY, "#it{t}_{y}", "#it{y}_{0} [mm]");

  setAxisLabels(m_hFirstStateX, "#it{x}_{0} [mm]", "entries");
  setAxisLabels(m_hFirstStateY, "#it{y}_{0} [mm]", "entries");

  m_hTimeDifferenceTrackTrigger = 
      book1D("TimeDifferenceTrackTriggers", "#Deltat", -1000., 1000., 500);
  setAxisLabels(m_hTimeDifferenceTrackTrigger,
                "#it{t}_{trigger} - #it{t}_track [ns]", "entries");

  const std::string labelx = "#it{x} - #it{x}_{track} [mm]";
  const std::string labely = "#it{y} - #it{y}_{track} [mm]";
  const std::string labelt = "#it{t} - #it{t}_{track} [ns]";
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const std::string plane = std::to_string(i);
    const std::string title = geomSvc()->modules().at(i)->id();
    // Track intercepts
    bins = m_parXY.bins();
    low = m_parXY.lowEdge();
    high = m_parXY.highEdge();
    std::string name = "Intercepts/Plane" + plane;
    m_hIntercepts.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    name = "InterceptsAssociated/Plane" + plane;
    m_hInterceptsAssociated.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    name = "InterceptsNonAssociated/Plane" + plane;
    m_hInterceptsNonAssociated.push_back(
        book2D(name, title, low, high, bins, low, high, bins));
    setAxisLabels(m_hIntercepts[i], "global #it{x} [mm]", "global #it{y} [mm]");
    setAxisLabels(m_hInterceptsAssociated[i], "global #it{x} [mm]",
                  "global #it{y} [mm]");
    setAxisLabels(m_hInterceptsNonAssociated[i], "global #it{x} [mm]",
                  "global #it{y} [mm]");

    // Biased x/y residuals
    bins = m_parResidualsXY.bins();
    low = m_parResidualsXY.lowEdge();
    high = m_parResidualsXY.highEdge();
    name = "BiasedResiduals/GlobalX/Plane" + plane;
    m_hBiasedResGX.push_back(book1D(name, title, low, high, bins));
    name = "BiasedResiduals/GlobalY/Plane" + plane;
    m_hBiasedResGY.push_back(book1D(name, title, low, high, bins));
    name = "BiasedResiduals/LocalX/Plane" + plane;
    m_hBiasedResLX.push_back(book1D(name, title, low, high, bins));
    name = "BiasedResiduals/LocalY/Plane" + plane;
    m_hBiasedResLY.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hBiasedResGX[i], labelx, "entries");
    setAxisLabels(m_hBiasedResGY[i], labely, "entries");
    setAxisLabels(m_hBiasedResLX[i], labelx, "entries");
    setAxisLabels(m_hBiasedResLY[i], labely, "entries");

    // Biased residuals vs. x/y
    const unsigned int binsXY = m_parXY.bins();
    const double lowXY = m_parXY.lowEdge();
    const double highXY = m_parXY.highEdge();
    name = "BiasedResiduals/GlobalResXvsLocalX/Plane" + plane;
    m_hBiasedResGXvsLX.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    name = "BiasedResiduals/GlobalResYvsLocalY/Plane" + plane;
    m_hBiasedResGYvsLY.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    name = "BiasedResiduals/GlobalResXvsGlobalX/Plane" + plane;
    m_hBiasedResGXvsGX.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    name = "BiasedResiduals/GlobalResYvsGlobalY/Plane" + plane;
    m_hBiasedResGYvsGY.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    setAxisLabels(m_hBiasedResGXvsLX[i], "local #it{x} [mm]", labelx);
    setAxisLabels(m_hBiasedResGYvsLY[i], "local #it{y} [mm]", labely);
    setAxisLabels(m_hBiasedResGXvsGX[i], "global #it{x} [mm]", labelx);
    setAxisLabels(m_hBiasedResGYvsGY[i], "global #it{y} [mm]", labely);

    // Unbiased x/y residuals
    name = "UnbiasedResiduals/GlobalX/Plane" + plane;
    m_hUnbiasedResGX.push_back(book1D(name, title, low, high, bins));
    name = "UnbiasedResiduals/GlobalY/Plane" + plane;
    m_hUnbiasedResGY.push_back(book1D(name, title, low, high, bins));
    name = "UnbiasedResiduals/LocalX/Plane" + plane;
    m_hUnbiasedResLX.push_back(book1D(name, title, low, high, bins));
    name = "UnbiasedResiduals/LocalY/Plane" + plane;
    m_hUnbiasedResLY.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hUnbiasedResGX[i], labelx, "entries");
    setAxisLabels(m_hUnbiasedResGY[i], labely, "entries");
    setAxisLabels(m_hUnbiasedResLX[i], labelx, "entries");
    setAxisLabels(m_hUnbiasedResLY[i], labely, "entries");

    // Unbiased residuals vs. x/y
    name = "UnbiasedResiduals/GlobalResXvsLocalX/Plane" + plane;
    m_hUnbiasedResGXvsLX.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    name = "UnbiasedResiduals/GlobalResYvsLocalY/Plane" + plane;
    m_hUnbiasedResGYvsLY.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    name = "UnbiasedResiduals/GlobalResXvsGlobalX/Plane" + plane;
    m_hUnbiasedResGXvsGX.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    name = "UnbiasedResiduals/GlobalResYvsGlobalY/Plane" + plane;
    m_hUnbiasedResGYvsGY.push_back(
        book2D(name, title, lowXY, highXY, binsXY, low, high, bins));
    setAxisLabels(m_hUnbiasedResGXvsGX[i], "local #it{x} [mm]", labelx);
    setAxisLabels(m_hUnbiasedResGYvsGY[i], "local #it{y} [mm]", labely);
    setAxisLabels(m_hUnbiasedResGXvsGX[i], "global #it{x} [mm]", labelx);
    setAxisLabels(m_hUnbiasedResGYvsGY[i], "global #it{y} [mm]", labely);

    // Biased residuals vs. track probability
    name = "BiasedResiduals/GlobalXvsTrackProb/Plane" + plane;
    m_hBiasedResGXvsTrackProb.push_back(
        book2D(name, title, 0.0, 1.0, 1000, low, high, bins));
    name = "BiasedResiduals/GlobalYvsTrackProb/Plane" + plane;
    m_hBiasedResGYvsTrackProb.push_back(
        book2D(name, title, 0.0, 1.0, 1000, low, high, bins));

    // Pull distributions
    name = "Pulls/ResidualX/Plane" + plane;
    m_hPullX.push_back(book1D(name, title, -5., 5., 100));
    name = "Pulls/ResidualY/Plane" + plane;
    m_hPullY.push_back(book1D(name, title, -5., 5., 100));
    setAxisLabels(m_hPullX[i], "pull " + labelx, "entries");
    setAxisLabels(m_hPullY[i], "pull " + labely, "entries");

    // Residuals with unbiased RMS
    name = "Resolution/GlobalX/Plane" + plane;
    m_hResolutionX.push_back(book1D(name, title, low, high, bins));
    name = "Resolution/GlobalY/Plane" + plane;
    m_hResolutionY.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hResolutionX[i], "#Delta x [mm]", "entries");
    setAxisLabels(m_hResolutionY[i], "#Delta y [mm]", "entries");

    // Time residuals
    bins = m_parResidualsT.bins();
    low = m_parResidualsT.lowEdge();
    high = m_parResidualsT.highEdge();
    name = "BiasedResiduals/Time/Plane" + plane;
    m_hBiasedResT.push_back(book1D(name, title, low, high, bins));
    setAxisLabels(m_hBiasedResT[i], labelt, "entries");

    // Residuals with respect to plane zero
    name = "MisalignmentFrom0/Plane" + plane;
    m_hSyncDifferences.push_back(book1D(name, title, -35.0, 35.0, 2000));
    setAxisLabels(m_hSyncDifferences[i], labelt, "entries");

    name = "MisalignmentFrom0Profile/Plane" + plane;
    m_hSyncInRun.push_back(bookProfile1D(name, title, 0, 120.0, 120));
    setAxisLabels(m_hSyncInRun[i], "#it{t}_{track} [minutes]",
                  "#LT #it{t} - #it{t}_{track} #GT [ns]");

    name = "BiasedResiduals/ResTimeVsGlobalX/Plane" + plane;
    m_hBiasedResTvsGX.push_back(
        book2D(name, title, -12, 3, 256, low, high, bins));
    setAxisLabels(m_hBiasedResTvsGX[i], "#it{x} [mm]", labelt);
    name = "BiasedResiduals/ResHitTimeVsColumn/Plane" + plane;
    m_hBiasedResPixelTvsColumn.push_back(
        book2D(name, title, -0.5, 255.5, 256, low, high, bins));
    setAxisLabels(m_hBiasedResPixelTvsColumn[i], "column",
                  "#it{t}_{hit} - #it{t}_{track} [ns]");

  }
}
