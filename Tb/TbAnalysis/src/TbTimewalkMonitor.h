#pragma once

// AIDA
#include "AIDA/IProfile1D.h"
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbTimewalkMonitor TbTimewalkMonitor.h
 *
 */

class TbTimewalkMonitor : public TbAlgorithm {
 public:
  /// Constructor
  TbTimewalkMonitor(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbTimewalkMonitor() {}
  virtual StatusCode finalize(); 
  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  std::string m_trackLocation;
  unsigned int m_widthMax, m_widthMin;
  std::vector<AIDA::IProfile1D*> m_timewalk;
  std::vector<AIDA::IProfile1D*> m_timewalkOneHit;
  std::vector<AIDA::IProfile1D*> m_timewalkTwoHit;
  std::vector<AIDA::IProfile1D*> m_timewalkQ;
  std::vector<AIDA::IHistogram1D*> m_dtDist;
  std::vector<AIDA::IHistogram1D*> m_cDist;
  
  std::vector<AIDA::IProfile1D*> m_space;
  std::vector<AIDA::IHistogram1D*> m_dt;
  std::vector<AIDA::IProfile1D*> m_LRSYNC; 
 
  std::vector<AIDA::IProfile1D*> m_UDSYNC;
  std::vector<AIDA::IProfile1D*> m_quad;

  std::vector<AIDA::IProfile1D*> m_inscol;
  std::vector<AIDA::IProfile1D*> m_interscol;

  std::vector<AIDA::IHistogram2D*> m_twd;
  std::vector<AIDA::IHistogram2D*> m_twdQ;
  std::vector<AIDA::IHistogram2D*> m_twdQL;
  std::vector<AIDA::IHistogram2D*> m_twdQR;
  std::vector<AIDA::IHistogram2D*> m_spd2D;
  
  std::vector<AIDA::IHistogram2D*> m_twdL;
  std::vector<AIDA::IHistogram2D*> m_twdR;
  
};
