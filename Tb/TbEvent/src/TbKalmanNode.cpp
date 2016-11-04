#include "Event/TbKalmanNode.h"

namespace {
//=============================================================================
// Helper function to filter one hit
//=============================================================================
inline double filterX(double& x, double& tx, double& covXX, double& covXTx,
                      double& covTxTx, const double xhit,
                      const double xhitcov) {
  const double predcovXX = covXX;
  const double predcovXTx = covXTx;
  const double predcovTxTx = covTxTx;
  const double predx = x;
  const double predtx = tx;

  // compute the gain matrix
  const double R = xhitcov + predcovXX;
  const double Kx = predcovXX / R;
  const double KTx = predcovXTx / R;
  // update the state vector
  double r = xhit - predx;
  x = predx + Kx * r;
  tx = predtx + KTx * r;
  // update the covariance matrix. we can write it in many ways ...
  covXX /*= predcovXX  - Kx * predcovXX */ = (1 - Kx) * predcovXX;
  covXTx /*= predcovXTx - predcovXX * predcovXTx / R */ = (1 - Kx) * predcovXTx;
  covTxTx = predcovTxTx - KTx * predcovXTx;
  // return the chi2
  return r * r / R;
}
}

namespace LHCb {

//=========================================================================
// Standard constructor, initializes variables
//=========================================================================
TbKalmanNode::TbKalmanNode()
    : m_parent(nullptr) {
}

//=========================================================================
// Constructor from a z position
//=========================================================================
TbKalmanNode::TbKalmanNode(TbKalmanTrack& parent, const double z)
    : m_parent(&parent) {

  m_active = false;
  m_state.setZ(z);
}

//=========================================================================
// Constructor from a cluster
//=========================================================================
TbKalmanNode::TbKalmanNode(TbKalmanTrack& parent, const TbCluster& cluster)
    : TbNode(cluster), m_parent(&parent) {

  m_state.setZ(cluster.z());
}

//=========================================================================
// Destructor
//=========================================================================
TbKalmanNode::~TbKalmanNode() {
  if (m_prevNode && m_prevNode->m_nextNode == this) 
    m_prevNode->m_nextNode = nullptr;
  if (m_nextNode && m_nextNode->m_prevNode == this) 
    m_nextNode->m_prevNode = nullptr;
}

//=============================================================================
// Filter X and Y independently. Could easily be complicated if needed.
//=============================================================================
LHCb::ChiSquare TbKalmanNode::filter(LHCb::TbState& state) const {

  double chi2(0.);
  Gaudi::SymMatrix4x4& cov = state.covariance();
  Gaudi::Vector4& vec = state.parameters();
  // First X
  chi2 += filterX(vec(0), vec(2), cov(0, 0), cov(0, 2), cov(2, 2),
                  m_x, m_covx);
  // Then Y
  chi2 += filterX(vec(1), vec(3), cov(1, 1), cov(1, 3), cov(3, 3),
                  m_y, m_covy);
  // chi2 has 2 dofs
  return LHCb::ChiSquare(chi2, 2);
}

//=============================================================================
// Turn measurement into an outlier.
//=============================================================================
void TbKalmanNode::deactivate(const bool flag) {
  // Only do something if this is actually an active hit
  if ((flag && m_active) || (!flag && !m_active)) {
    // Set type to outlier
    m_active = !flag;
    // This will take care of upstream and downstream nodes as well:
    // they will be reset to initialized. we need to check this carefully.
    resetFilterStatus(Predicted);
    // Now make sure others do not rely on this one anymore.
    if (!hasInfoUpstream(Forward)) resetHasInfoUpstream(Forward);
    if (!hasInfoUpstream(Backward)) resetHasInfoUpstream(Backward);
  }
}

//=========================================================================
// Helper function to decide if we need to use the upstream filtered state
//=========================================================================
bool TbKalmanNode::hasInfoUpstream(const int direction) const {
  if (m_hasInfoUpstream[direction] == Unknown) {
    bool rc = false;
    const TbKalmanNode* prev = prevNode(direction);
    if (prev) {
      if (prev->active())
        rc = true;
      else
        rc = prev->hasInfoUpstream(direction);
    }
    unConst().m_hasInfoUpstream[direction] = rc ? True : False;
  }
  return (m_hasInfoUpstream[direction] == True);
}

void TbKalmanNode::resetHasInfoUpstream(int direction) {
  m_hasInfoUpstream[direction] = False;
  if (!active()) {
    TbKalmanNode* next = const_cast<TbKalmanNode*>(nextNode(direction));
    if (next) next->resetHasInfoUpstream(direction);
  }
}

//=========================================================================
// Reset the status of this node
//=========================================================================
void TbKalmanNode::resetFilterStatus(const int dir, FilterStatus s) {
  // The logic here is tedious, because of the smoothed states having
  // a strange depence, which depends on the type of smoother.
  if (m_filterStatus[dir] <= s) return;
  m_filterStatus[dir] = s;

  if (s < Filtered) {
    // If the backward filter is in 'Smoothed' state, it needs to be
    // reset to filtered, because the bi-directional smoother relies
    // on both filtered states.
    // Note: Backward=Smoothed means 'bi-directional smoother'.
    if (m_filterStatus[Backward] == Smoothed)  
      m_filterStatus[Backward] = Filtered;

    // reset the status of any node that depends on this one. now
    // be careful: if this node has been copied it may be pointing
    // to a wrong node.
    const TbKalmanNode* next = nextNode(dir);
    if (next && next->m_filterStatus[dir] > s && next->prevNode(dir) == this)
      const_cast<TbKalmanNode*>(next)->resetFilterStatus(dir, std::min(s, Initialized));
  }

  if (dir != Forward) return;
  // for the classical filter, we actually need to put the
  // upstream node back to filtered, if it is in a classicaly
  // smoothed status
  // Note: Forward=Smoothed means 'classical smoother'
  const TbKalmanNode* prev = prevNode(Forward);
  if (prev && prev->m_filterStatus[Forward] == Smoothed &&
      prev->nextNode(Forward) == this)  
    const_cast<TbKalmanNode*>(prev)->resetFilterStatus(Forward, Filtered);
}

//=========================================================================
// Predict the state to this node
//=========================================================================
void TbKalmanNode::computePredictedState(const int direction) {

  // Get the filtered state from the previous node. 
  // If there wasn't any, we will want to copy the reference vector 
  // and leave the covariance the way it is.
  m_predictedState[direction].setZ(z());
  auto& stateVec = m_predictedState[direction].parameters();
  auto& stateCov = m_predictedState[direction].covariance();

  const auto prevnode = prevNode(direction);
  if (prevnode) {
    const auto& prevState = prevnode->filteredState(direction);
    if (!hasInfoUpstream(direction)) {
      // Just _copy_ the covariance matrix from upstream, assuming
      // that this is the seed matrix. That saves us from copying
      // the seed matrix to every state from the start.
      stateCov = prevState.covariance();
      // Start the backward filter from the forward filter.
      if (direction == Backward) stateVec = filteredState(Forward).parameters();
    } else {
      // For the testbeam, the transport is really trivial, 
      // assuming x and y are uncorrelated.
      const double dz = z() - prevnode->z();
      stateVec = prevState.parameters();
      stateVec[0] += dz * stateVec[2];
      stateVec[1] += dz * stateVec[3];

      // Compute the predicted covariance
      stateCov = prevState.covariance();
      stateCov(0, 0) += 2 * dz * stateCov(0, 2) + dz * dz * stateCov(2, 2);
      stateCov(0, 2) += dz * stateCov(2, 2);
      stateCov(1, 1) += 2 * dz * stateCov(1, 3) + dz * dz * stateCov(3, 3);
      stateCov(1, 3) += dz * stateCov(3, 3);

      // Finally add the noise.
      const double q = direction == Forward ? prevnode->m_Q : m_Q;
      stateCov(2, 2) += q;
      stateCov(3, 3) += q;
    }
  }
  // Update the status flag.
  m_filterStatus[direction] = Predicted;
}

//=========================================================================
// Filter this node
//=========================================================================
void TbKalmanNode::computeFilteredState(const int direction) {

  // Copy the predicted state
  auto& state = m_filteredState[direction];
  state = predictedState(direction);

  // Apply the filter if needed
  m_deltaChi2[direction] = filter(state);
  // Update the status flag.
  m_filterStatus[direction] = Filtered;
}

//=========================================================================
// Bi-directional smoother
//=========================================================================
void TbKalmanNode::computeBiSmoothedState() {

  auto& state = m_state;
  if (!hasInfoUpstream(Forward)) {
    // Last node in backward direction
    state = filteredState(Backward);
  } else if (!hasInfoUpstream(Backward)) {
    // Last node in forward direction
    state = filteredState(Forward);
  } else {
    // Take the weighted average of two states. We now need to
    // choose for which one we take the filtered state. AFAIU the
    // weighted average behaves better if the weights are more
    // equal. So, we filter the 'worst' prediction. In the end, it
    // all doesn't seem to make much difference.

    const LHCb::TbState* s1;
    const LHCb::TbState* s2;
    if (predictedState(Backward).covariance()(0, 0) >
        predictedState(Forward).covariance()(0, 0)) {
      s1 = &(filteredState(Backward));
      s2 = &(predictedState(Forward));
    } else {
      s1 = &(filteredState(Forward));
      s2 = &(predictedState(Backward));
    }

    const auto& X1 = s1->parameters();
    const auto& C1 = s1->covariance();
    const auto& X2 = s2->parameters();
    const auto& C2 = s2->covariance();

    auto& X = state.parameters();
    auto& C = state.covariance();

    // Compute the inverse of the covariance in the difference: R=(C1+C2)
    static Gaudi::SymMatrix4x4 invR;
    invR = C1 + C2;
    const bool success = invR.InvertChol();
    if (!success) {
      std::cerr << "Error inverting cov matrix in smoother" << std::endl;
    }
    // Compute the gain matrix.
    static ROOT::Math::SMatrix<double, 4, 4> K;
    K = C1 * invR;
    X = X1 + K * (X2 - X1);
    ROOT::Math::AssignSym::Evaluate(C, K * C2);
    // The following used to be more stable, but isn't any longer, it seems:
    //ROOT::Math::AssignSym::Evaluate(C, -2 * K * C1);
    //C += C1 + ROOT::Math::Similarity(K,R);
    // std::cout << "smoothing two states with errors on slope: "
    //           << std::sqrt(C1(2,2)) << " " << std::sqrt(C2(2,2)) << " "
    //           << std::sqrt(C(2,2)) << std::endl ;
  }
  // updateResidual(state);

  // Bug fix: we cannot set backward to state 'Smoothed', unless we have passed
  // its filter step!
  filteredState(Backward);
  m_filterStatus[Backward] = Smoothed;
}

int TbKalmanNode::index() const {
  int rc = 0;
  if (m_prevNode) rc = m_prevNode->index() + 1;
  return rc;
}
}
