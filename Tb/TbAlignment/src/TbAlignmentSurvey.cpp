// ROOT
#include "TH1.h"

// Gaudi
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiUtils/Aida2ROOT.h"

// Local
#include "TbAlignmentSurvey.h"

DECLARE_TOOL_FACTORY(TbAlignmentSurvey)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbAlignmentSurvey::TbAlignmentSurvey(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent)
    : TbAlignmentBase(type, name, parent) {}

//=============================================================================
// Destructor
//=============================================================================
TbAlignmentSurvey::~TbAlignmentSurvey() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TbAlignmentSurvey::initialize() {

  // Initialise the base class.
  StatusCode sc = TbAlignmentBase::initialize();
  if (sc.isFailure()) return sc;

  return StatusCode::SUCCESS;
}

void TbAlignmentSurvey::align(std::vector<TbAlignmentTrack*>& /*tracks*/) {

  info() << "Survey alignment" << endmsg;
  for (unsigned int i = 0; i < m_nPlanes; ++i) {
    if (masked(i)) continue;
    const std::string plane = std::to_string(i);
    const std::string path = "Tb/TbClusterPlots/Differences/";
    const std::string titlex = path + "x/Plane" + plane;
    const std::string titley = path + "y/Plane" + plane;
    double tx = m_modules[i]->x();
    double ty = m_modules[i]->y();

    if (m_dofs[0]) {
      AIDA::IHistogram1D* hAida = NULL;
      StatusCode sc = GaudiTool::histoSvc()->retrieveObject(titlex, hAida);
      if (sc.isFailure() || !hAida) {
        warning() << "Cannot retrieve histogram " << titlex << endmsg;
      } else {
        auto hRoot = Gaudi::Utils::Aida2ROOT::aida2root(hAida);
        tx -= hRoot->GetBinCenter(hRoot->GetMaximumBin());
      }
    }
    if (m_dofs[1]) {
      AIDA::IHistogram1D* hAida = NULL;
      StatusCode sc = GaudiTool::histoSvc()->retrieveObject(titley, hAida);
      if (sc.isFailure() || !hAida) {
        warning() << "Cannot retrieve histogram " << titley << endmsg;
      } else {
        auto hRoot = Gaudi::Utils::Aida2ROOT::aida2root(hAida);
        ty -= hRoot->GetBinCenter(hRoot->GetMaximumBin());
      }
    }
    const double rx = m_modules[i]->rotX();
    const double ry = m_modules[i]->rotY();
    const double rz = m_modules[i]->rotZ();
    const double tz = m_modules[i]->z();
    m_modules[i]->setAlignment(tx, ty, tz, rx, ry, rz, 0., 0., 0., 0., 0., 0.);
  }
}
