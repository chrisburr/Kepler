#include "Event/TbState.h"

using namespace LHCb;

//=============================================================================
// Update the state vector
//=============================================================================
void TbState::setState(double x, double y, double tx, double ty, double z) {
  m_parameters[0] = x;
  m_parameters[1] = y;
  m_parameters[2] = tx;
  m_parameters[3] = ty;
  m_z = z;
}

//=============================================================================
// Clone the state
//=============================================================================
TbState* TbState::clone() const { return new TbState(*this); }
