#include "Event/TbNode.h"

namespace LHCb {

//=========================================================================
// Compute the residual with respect to a given state
//=========================================================================
void TbNode::updateResidual(const TbState& state) {
  m_residualX = m_x - state.x();
  m_residualY = m_y - state.y();
  const int sign = m_active ? -1 : +1;
  m_residualCovX = m_covx + sign * state.covariance()(0, 0);
  m_residualCovY = m_covy + sign * state.covariance()(1, 1);
}

}
