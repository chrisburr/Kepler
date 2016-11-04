#include "TbAlignmentMinuit0.h"

DECLARE_TOOL_FACTORY(TbAlignmentMinuit0)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbAlignmentMinuit0::TbAlignmentMinuit0(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : TbAlignmentMinuitBase(type, name, parent) {

  declareProperty("ReferencePlane", m_referencePlane = 999);

  m_dofsDefault = {true, true, false, true, true, true};
}

//=============================================================================
// Destructor
//=============================================================================
TbAlignmentMinuit0::~TbAlignmentMinuit0() {}

//=============================================================================
// Calculate the chi2.
//=============================================================================
void TbAlignmentMinuit0::chi2(double& f, double* par, double* /*g*/) {

  // Assign new aligment constants
  for (auto im = m_modules.begin(), end = m_modules.end(); im != end; ++im) {
    const unsigned int i = im - m_modules.begin();
    (*im)->setAlignment(par[6 * i + 0], par[6 * i + 1], par[6 * i + 2],
                        par[6 * i + 3], par[6 * i + 4], par[6 * i + 5]);
  }

  // Loop over tracks
  f = 0.;
  for (auto it = m_tracks.begin(), end = m_tracks.end(); it != end; ++it) {
    // Update global coordinates of the clusters
    SmartRefVector<LHCb::TbCluster> clusters = (*it)->track()->clusters();
    for (auto ic = clusters.begin(), end = clusters.end(); ic != end; ++ic) {
      Gaudi::XYZPoint pLocal((*ic)->xloc(), (*ic)->yloc(), 0.);
      const unsigned int plane = (*ic)->plane();
      Gaudi::XYZPoint pGlobal = geomSvc()->localToGlobal(pLocal, plane);
      (*ic)->setX(pGlobal.x());
      (*ic)->setY(pGlobal.y());
      (*ic)->setZ(pGlobal.z());
    }
    // Refit track with new cluster positions
    m_trackFit->fit((*it)->track());
    // Add the new track chi2 to the overall chi2
    f += (*it)->track()->chi2();
  }
}
//=============================================================================
// Main alignment function.
//=============================================================================
void TbAlignmentMinuit0::align(
    std::vector<TbAlignmentTrack*>& alignmentTracks) {

  TbAlignmentMinuitBase::align(alignmentTracks);
  info() << "Minuit technique 0" << endmsg;
  double arglist[2];
  arglist[0] = 10000;
  arglist[1] = 1.e-2;

  for (unsigned int iteration = 0; iteration < m_nIterations; ++iteration) {
    info() << "Iteration " << iteration + 1 << "/" << m_nIterations << endmsg;
    // Align detector modules one at a time
    for (unsigned int i = 0; i < m_nPlanes; ++i) {
      // Skip reference plane and masked planes.
      if (i == m_referencePlane || masked(i)) continue;
      // Wobble this plane and fix the others.
      for (unsigned int j = 0; j < m_nPlanes; ++j) {
        if (i != j) {
          m_fitter->FixParameter(6 * j + 0);  // x
          m_fitter->FixParameter(6 * j + 1);  // y
          m_fitter->FixParameter(6 * j + 2);  // z
          m_fitter->FixParameter(6 * j + 3);  // Rx
          m_fitter->FixParameter(6 * j + 4);  // Ry
          m_fitter->FixParameter(6 * j + 5);  // Rz
        } else {
          info() << "*** Wobbling detector " << j << endmsg;
          for (unsigned int k = 0; k < 6; ++k) {
            if (m_dofs[k]) {
              m_fitter->ReleaseParameter(6 * j + k);
            } else {
              m_fitter->FixParameter(6 * j + k);
            }
          }
        }
      }
      // Execute minimization and calculate proper error matrix
      m_fitter->ExecuteCommand("MIGRAD", arglist, 2);
      m_fitter->ExecuteCommand("HESSE", arglist, 1);
      if (msgLevel(MSG::INFO))
        m_fitter->ExecuteCommand("SHOW PARAMETERS", 0, 0);
    }
  }
  updateGeometry();
}
