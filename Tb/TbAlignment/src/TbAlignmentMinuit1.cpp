#include "TbAlignmentMinuit1.h"

DECLARE_TOOL_FACTORY(TbAlignmentMinuit1)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbAlignmentMinuit1::TbAlignmentMinuit1(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : TbAlignmentMinuitBase(type, name, parent) {

  declareProperty("ReferencePlane", m_referencePlane = 999);

  m_dofsDefault = {true, true, false, false, false, true};
}

//=============================================================================
// Destructor
//=============================================================================
TbAlignmentMinuit1::~TbAlignmentMinuit1() {}

//=============================================================================
// Calculate the chi2.
//=============================================================================
void TbAlignmentMinuit1::chi2(double& f, double* par, double* /*g*/) {

  // Assign new aligment constants
  for (auto im = m_modules.begin(), end = m_modules.end(); im != end; ++im) {
    const unsigned int i = im - m_modules.begin();
    (*im)->setAlignment(par[6 * i + 0], par[6 * i + 1], par[6 * i + 2],
                        par[6 * i + 3], par[6 * i + 4], par[6 * i + 5]);
  }

  // Loop over tracks
  f = 0.;
  for (auto it = m_tracks.begin(), end = m_tracks.end(); it != end; ++it) {
    // Get global position of each cluster relative to the reference cluster
    auto clusters = (*it)->track()->clusters();
    for (auto ic = clusters.cbegin(), end = clusters.cend(); ic != end; ++ic) {
      // Skip reference plane
      const unsigned int plane = (*ic)->plane();
      if (plane == m_referencePlane) continue;
      // Transform local cluster coordinates to global frame
      Gaudi::XYZPoint pLocal((*ic)->xloc(), (*ic)->yloc(), 0.);
      Gaudi::XYZPoint pGlobal = geomSvc()->localToGlobal(pLocal, plane);
      // Calculate residuals wrt reference
      const double xresidual = pGlobal.x() - (*it)->xOnReferencePlane();
      const double yresidual = pGlobal.y() - (*it)->yOnReferencePlane();
      // Add residuals to chi2
      f += xresidual * xresidual + yresidual * yresidual;
    }
  }
}

//=============================================================================
// Main alignment function.
//=============================================================================
void TbAlignmentMinuit1::align(
    std::vector<TbAlignmentTrack*>& alignmentTracks) {

  TbAlignmentMinuitBase::align(alignmentTracks);
  info() << "Minuit technique 1" << endmsg;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    const unsigned int offset = 6 * i;
    // Fix unwanted detectors
    if (i == m_referencePlane || masked(i)) {
      info() << "*** Detector " << i << " will be held fixed" << endmsg;
      m_fitter->FixParameter(offset + 0);  // x
      m_fitter->FixParameter(offset + 1);  // y
      m_fitter->FixParameter(offset + 2);  // z
      m_fitter->FixParameter(offset + 3);  // Rx
      m_fitter->FixParameter(offset + 4);  // Ry
      m_fitter->FixParameter(offset + 5);  // Rz
    } else {
      for (unsigned int j = 0; j < 6; ++j) {
        if (m_dofs[j]) {
          m_fitter->ReleaseParameter(offset + j);
        } else {
          m_fitter->FixParameter(offset + j);
        }
      }
    }
  }

  // Loop over tracks and get global position of the reference cluster.
  for (auto it = m_tracks.begin(), end = m_tracks.end(); it != end; ++it) {
    auto clusters = (*it)->track()->clusters();
    for (auto ic = clusters.cbegin(), end = clusters.cend(); ic != end; ++ic) {
      // Find reference cluster
      const unsigned int plane = (*ic)->plane();
      if (plane != m_referencePlane) continue;
      // Transform local cluster coordinates to global frame
      Gaudi::XYZPoint pLocal((*ic)->xloc(), (*ic)->yloc(), 0.);
      Gaudi::XYZPoint pGlobal = geomSvc()->localToGlobal(pLocal, plane);
      (*it)->setXOnReferencePlane(pGlobal.x());
      (*it)->setYOnReferencePlane(pGlobal.y());
      break;
    }
  }

  // Execute minimization and calculate proper error matrix
  double arglist[2];
  arglist[0] = 10000;
  arglist[1] = 1.e-2;
  m_fitter->ExecuteCommand("MIGRAD", arglist, 2);
  m_fitter->ExecuteCommand("HESSE", arglist, 1);
  if (msgLevel(MSG::INFO)) m_fitter->ExecuteCommand("SHOW PARAMETERS", 0, 0);
  updateGeometry();
}
