#include "TbAlignmentMinuitBase.h"

namespace Tb {
TbAlignmentMinuitBase* gTbAlignmentMinuitBase(nullptr);

//=============================================================================
// Function to be minimised
//=============================================================================
void fcn(int&, double* g, double& f, double* par, int) {
  gTbAlignmentMinuitBase->chi2(f, par, g);
}
}

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbAlignmentMinuitBase::TbAlignmentMinuitBase(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
    : TbAlignmentBase(type, name, parent) {

  declareProperty("NIterations", m_nIterations = 3);
  declareProperty("FitStrategy", m_fitStrategy = 2);
}

//=============================================================================
// Destructor
//=============================================================================
TbAlignmentMinuitBase::~TbAlignmentMinuitBase() {}

//=============================================================================
// Update the module positions and orientations
//=============================================================================
void TbAlignmentMinuitBase::updateGeometry() {

  info() << "Updating alignment constants:" << endmsg;
  for (auto im = m_modules.begin(), end = m_modules.end(); im != end; ++im) {
    const unsigned int i = im - m_modules.begin();
    const double x = (*im)->x() + m_fitter->GetParameter(6 * i + 0);
    const double y = (*im)->y() + m_fitter->GetParameter(6 * i + 1);
    const double z = (*im)->z() + m_fitter->GetParameter(6 * i + 2);
    const double rotx = (*im)->rotX() + m_fitter->GetParameter(6 * i + 3);
    const double roty = (*im)->rotY() + m_fitter->GetParameter(6 * i + 4);
    const double rotz = (*im)->rotZ() + m_fitter->GetParameter(6 * i + 5);
    info() << format("%2i", i) << format(" %-15s", (*im)->id().c_str())
           << format(" %10.3f", (*im)->x()) << format(" %10.3f", (*im)->y())
           << format(" %10.3f", (*im)->z()) << format(" %10.3f", (*im)->rotX())
           << format(" %10.3f", (*im)->rotY())
           << format(" %10.3f", (*im)->rotZ()) << endmsg;
    (*im)->setAlignment(x, y, z, rotx, roty, rotz, 0., 0., 0., 0., 0., 0.);
  }
}

//=============================================================================
// Main alignment function
//=============================================================================
void TbAlignmentMinuitBase::align(
    std::vector<TbAlignmentTrack*>& alignmentTracks) {

  m_tracks = alignmentTracks;
  // Set up the fitter.
  m_fitter = new TFitter(60);
  Tb::gTbAlignmentMinuitBase = this;
  m_fitter->SetFCN(Tb::fcn);
  // Set the output level (-1: none, 0: minimum: 1: normal).
  double arglist = -1.;
  if (msgLevel(MSG::DEBUG)) arglist = 1.;
  m_fitter->ExecuteCommand("SET PRINTOUT", &arglist, 1);
  // Set the value defining parameter errors.
  arglist = 1.;
  m_fitter->ExecuteCommand("SET ERR", &arglist, 1);
  // Set the strategy for calculating derivatives. 
  arglist = m_fitStrategy;
  m_fitter->ExecuteCommand("SET STRATEGY", &arglist, 1);
  // Set the floating point accuracy.
  arglist = 1.e-12;
  m_fitter->ExecuteCommand("SET EPS", &arglist, 1);
  setParameters();
}

//=============================================================================
// Set the initial values of the fit parameters.
//=============================================================================
void TbAlignmentMinuitBase::setParameters() {

  info() << "Setting initial values of alignment parameters" << endmsg;
  for (auto im = m_modules.begin(), end = m_modules.end(); im != end; ++im) {
    const unsigned int i = im - m_modules.begin();
    const double dx = (*im)->dX();
    const double dy = (*im)->dY();
    const double dz = (*im)->dZ();
    const double drx = (*im)->dRotX();
    const double dry = (*im)->dRotY();
    const double drz = (*im)->dRotZ();
    const std::string id = (*im)->id();
    m_fitter->SetParameter(6 * i + 0, (id + " dx ").c_str(), dx, 0.001, 0., 0.);
    m_fitter->SetParameter(6 * i + 1, (id + " dy ").c_str(), dy, 0.001, 0., 0.);
    m_fitter->SetParameter(6 * i + 2, (id + " dz ").c_str(), dz, 1., 0., 0.);
    m_fitter->SetParameter(6 * i + 3, (id + " dRx").c_str(), drx, 0.001, 0.,
                           0.);
    m_fitter->SetParameter(6 * i + 4, (id + " dRy").c_str(), dry, 0.001, 0.,
                           0.);
    m_fitter->SetParameter(6 * i + 5, (id + " dRz").c_str(), drz, 0.001, 0.,
                           0.);
  }
}
