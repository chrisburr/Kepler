#ifndef TB_TRACKFITTER_H
#define TB_TRACKFITTER_H 1

#include "TH1.h"
#include "TH2.h"

#include "GaudiAlg/GaudiTupleAlg.h"
#include "GaudiKernel/RndmGenerators.h"

#include "TbKernel/ITbGeometrySvc.h"
#include "TbKernel/ITbTrackFit.h"


/** @class TbTrackFitter TbTrackFitter.h
 *  Author: Panagiotis Tsopelas
 */

class TbTrackFitter : public GaudiTupleAlg {
 public:
  TbTrackFitter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TbTrackFitter() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  Rndm::Numbers m_gauss;

  bool m_onOff_hists;
  unsigned int m_nPlanes;

  ITbTrackFit* m_trackFit;
  
  double            m_scat2;
  double            m_hiterror2;
  double            m_theta0;
  int               m_direction;
  
  double sigmax;
  double sigmay;
  
  double scatRx0[8];
  double scatRy0[8];
  
  mutable ITbGeometrySvc* m_geomSvc;  /// Access geometry service on-demand
  ITbGeometrySvc* geomSvc() const {
    if (!m_geomSvc) m_geomSvc = svc<ITbGeometrySvc>("TbGeometrySvc", true);
    return m_geomSvc;
  }

  // Histograms
  
  TH1D* m_clustersX;
  TH1D* m_clustersY;
  
  // straight line fit
  TH1D* m_Sfit_chi2;
  TH1D* m_Sfit_prob;

  TH1D* m_slopesX;
  TH1D* m_slopesY;

  std::vector<TH1D*> m_resSfit_X;
  std::vector<TH1D*> m_resSfit_Y;
  
  std::vector<TH1D*> m_trueSfit_X;
  std::vector<TH1D*> m_trueSfit_Y;
  
  std::vector<TH1D*> m_respullSfit_X;
  std::vector<TH1D*> m_respullSfit_Y;
  
  std::vector<TH1D*> m_trackpullSfit_X;
  std::vector<TH1D*> m_trackpullSfit_Y;


  // Kalman filter
  TH1D* m_Kfit_chi2;
  TH1D* m_Kfit_prob;
  
  // unbiased residuals
  std::vector<TH1D*> m_XunresKfit;
  std::vector<TH1D*> m_YunresKfit;
  
  std::vector<TH1D*> m_resKfit_X;
  std::vector<TH1D*> m_resKfit_Y;

  std::vector<TH1D*> m_reserrKfit_X;
  std::vector<TH1D*> m_reserrKfit_Y;

  std::vector<TH1D*> m_respullKfit_X;
  std::vector<TH1D*> m_respullKfit_Y;

  std::vector<TH1D*> m_trueKfit_X;
  std::vector<TH1D*> m_trueKfit_Y;

  std::vector<TH1D*> m_trueerrKfit_X;
  std::vector<TH1D*> m_trueerrKfit_Y;

  std::vector<TH1D*> m_trackpullKfit_X;
  std::vector<TH1D*> m_trackpullKfit_Y;

  

  // Histograms functions
  void setup_hists();
  void fill_hists(std::vector<LHCb::TbTrack*>&);
  void fill_khists(std::vector<LHCb::TbKalmanTrack*>&);

  // Modifiers and Accessors

  void setNPlanes(unsigned int nplanes) { m_nPlanes = nplanes; }
  int nPlanes() { return m_nPlanes; }

  bool onOff_hists() { return m_onOff_hists; }
};
#endif
