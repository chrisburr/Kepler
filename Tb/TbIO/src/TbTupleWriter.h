#ifndef TB_TUPLEWRITER_H
#define TB_TUPLEWRITER_H 1

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbTupleWriter TbTupleWriter.h
 *
 *  Algorithm to write out hits/clusters/tracks/triggers in ntuple format.
 *
 *  @author Heinrich Schindler
 *  @date   2014-06-19
 */

class TbTupleWriter : public TbAlgorithm {
 public:
  /// Standard constructor
  TbTupleWriter(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbTupleWriter();

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  bool m_writeTriggers;
  bool m_writeHits;
  bool m_writeClusters;
  bool m_writeTracks;
  bool m_writeClusterHits;
  /// Flag to produce a stripped nTuple
  bool m_strippedNTuple; 
  
  unsigned int m_maxclustersize;
  unsigned int m_maxTriggers;
  unsigned long long m_evtNo = 0;
  std::string m_triggerLocation;
  std::string m_hitLocation;
  std::string m_clusterLocation;
  std::string m_trackLocation;

  void fillTriggers();
  void fillHits();
  void fillClusters();
  void fillTracks();

  void bookTriggers();
  void bookHits();
  void bookClusters();
  void bookTracks();

  // Trigger nTuple entries
  NTuple::Item<unsigned long long> m_TgID;
  NTuple::Item<unsigned long long> m_TgTime;
  NTuple::Item<double> m_TgHTime;
  NTuple::Item<unsigned int> m_TgEvt;
  NTuple::Item<unsigned int> m_TgPlane;
  NTuple::Item<unsigned int> m_TgCounter;

  // Hit nTuple entries
  NTuple::Item<unsigned long long> m_hID;
  NTuple::Item<unsigned long long> m_hTime;
  NTuple::Item<unsigned int> m_hCol;
  NTuple::Item<unsigned int> m_hRow;
  NTuple::Item<unsigned int> m_hScol;
  NTuple::Item<double> m_hHTime;
  NTuple::Item<unsigned int> m_hToT;
  NTuple::Item<unsigned int> m_hPlane;

  // Track nTuple entries
  NTuple::Item<unsigned long long> m_TkID;
  NTuple::Item<unsigned long long> m_TkTime;
  NTuple::Array<unsigned long long> m_TkClId;
  NTuple::Array<unsigned long long> m_TkTgId;
  NTuple::Array<unsigned int> m_TkTgPlane;
  NTuple::Array<double> m_TkXResidual; 
  NTuple::Array<double> m_TkYResidual;  

  NTuple::Item<double> m_TkX0;
  NTuple::Item<double> m_TkY0;
  NTuple::Item<double> m_TkTx;
  NTuple::Item<double> m_TkTy;
  NTuple::Item<double> m_TkHTime;
  NTuple::Item<double> m_TkChi2ndof;
  NTuple::Item<unsigned int> m_TkNCl;
  NTuple::Item<unsigned int> m_TkNTg;
  NTuple::Item<unsigned int> m_TkNTgPlane;
  
  NTuple::Item<unsigned int> m_TkEvt;

  // Cluster nTuple entries
  NTuple::Item<double> m_clGx, m_clGy, m_clGz;
  NTuple::Item<double> m_clLx, m_clLy, m_clHTime;
  NTuple::Item<unsigned int> m_clSize;
  NTuple::Item<double> m_clCharge;
  NTuple::Item<unsigned int>  m_clPlane, m_clN;
  NTuple::Item<unsigned long long> m_clEvtNo, m_clID, m_clTime;
  NTuple::Item<bool> m_clTracked;
  NTuple::Array<int> m_clhRow, m_clhCol, m_clhToT;
  NTuple::Array<unsigned int> m_clsCol;
  NTuple::Array<double> m_clhHTime;
};

#endif
