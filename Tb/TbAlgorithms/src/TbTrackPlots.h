#ifndef TB_TRACKPLOTS_H
#define TB_TRACKPLOTS_H 1

// AIDA
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"

#include "AIDA/IProfile2D.h"
#include "AIDA/IProfile1D.h"

// Tb/TbEvent
#include "Event/TbTrack.h"

// Tb/TbKernel
#include "TbKernel/ITbTrackFit.h"
#include "TbKernel/TbAlgorithm.h"

/** @class TbTrackPlots TbTrackPlots.h
 *
 *  Algorithm to produce monitoring histograms for telescope tracks.
 *
 *  @author Dan Saunders
 */

class TbTrackPlots : public TbAlgorithm {
 public:
  /// Constructor
  TbTrackPlots(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbTrackPlots() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  /// TES location of tracks
  std::string m_trackLocation;

  /// Calculate residuals by refitting and calculating the intercept. 
  bool m_refit;
  /// Fit upstream and downstream planes separately and plot the angle.
  bool m_plotAngle;

  /// Track intercepts.
  std::vector<AIDA::IHistogram2D*> m_hIntercepts;
  std::vector<AIDA::IHistogram2D*> m_hInterceptsAssociated;
  std::vector<AIDA::IHistogram2D*> m_hInterceptsNonAssociated;
  /// Biased residuals.
  std::vector<AIDA::IHistogram1D*> m_hBiasedResGX;
  std::vector<AIDA::IHistogram1D*> m_hBiasedResGY;
  std::vector<AIDA::IHistogram1D*> m_hBiasedResLX;
  std::vector<AIDA::IHistogram1D*> m_hBiasedResLY;
  /// Unbiased residuals.
  std::vector<AIDA::IHistogram1D*> m_hUnbiasedResGX;
  std::vector<AIDA::IHistogram1D*> m_hUnbiasedResGY;
  std::vector<AIDA::IHistogram1D*> m_hUnbiasedResLX;
  std::vector<AIDA::IHistogram1D*> m_hUnbiasedResLY;
  /// Residual pulls.
  std::vector<AIDA::IHistogram1D*> m_hPullX;
  std::vector<AIDA::IHistogram1D*> m_hPullY;
  /// Residuals with unbiased RMS.
  std::vector<AIDA::IHistogram1D*> m_hResolutionX;
  std::vector<AIDA::IHistogram1D*> m_hResolutionY;

  /// Biased residuals as functions of x/y.
  std::vector<AIDA::IHistogram2D*> m_hBiasedResGXvsLX;
  std::vector<AIDA::IHistogram2D*> m_hBiasedResGYvsLY;
  std::vector<AIDA::IHistogram2D*> m_hBiasedResGXvsGX;
  std::vector<AIDA::IHistogram2D*> m_hBiasedResGYvsGY;
  /// Unbiased residuals as functions of x/y.
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGXvsLX;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGYvsLY;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGXvsGX;
  std::vector<AIDA::IHistogram2D*> m_hUnbiasedResGYvsGY;
  /// Biased residuals as functions of track probability.
  std::vector<AIDA::IHistogram2D*> m_hBiasedResGXvsTrackProb;
  std::vector<AIDA::IHistogram2D*> m_hBiasedResGYvsTrackProb;

  /// Cluster time residuals.
  std::vector<AIDA::IHistogram1D*> m_hBiasedResT;
  /// Cluster time residuals as function of global x.
  std::vector<AIDA::IHistogram2D*> m_hBiasedResTvsGX;
  /// Pixel time residuals as function of column number.
  std::vector<AIDA::IHistogram2D*> m_hBiasedResPixelTvsColumn;

  /// Other.
  AIDA::IHistogram1D* m_hTimeDifferenceTrackTrigger = nullptr;

  std::vector<AIDA::IHistogram1D*> m_hSyncDifferences;
  std::vector<AIDA::IProfile1D*> m_hSyncInRun;

  AIDA::IHistogram1D* m_hChi2 = nullptr;
  AIDA::IHistogram1D* m_hProb = nullptr;
  AIDA::IHistogram1D* m_hTrackSize = nullptr;
  AIDA::IHistogram1D* m_hSlopeXZ = nullptr;
  AIDA::IHistogram1D* m_hSlopeYZ = nullptr;
  AIDA::IHistogram1D* m_hFirstStateX = nullptr;
  AIDA::IHistogram1D* m_hFirstStateY = nullptr;
  AIDA::IHistogram2D* m_hSlopeXvsX = nullptr;
  AIDA::IHistogram2D* m_hSlopeYvsY = nullptr;

  /// Parameters for chi-squared distribution
  Gaudi::Histo1DDef m_parChi2;
  /// Parameters for x/y residual distributions
  Gaudi::Histo1DDef m_parResidualsXY;
  /// Parameters for time residual distributions
  Gaudi::Histo1DDef m_parResidualsT;
  /// Parameters for x/y histograms (hitmaps)
  Gaudi::Histo1DDef m_parXY;
  /// Parameters for track slope distributions
  Gaudi::Histo1DDef m_parSlope;

  /// Name of the track fit tool
  std::string m_trackFitTool;
  /// Track fit tool
  ITbTrackFit* m_trackFit = nullptr;

  void setupPlots();
  void fillTimingPlots(const LHCb::TbTrack* track);
  void fillResiduals(LHCb::TbTrack* track);
  void fillResidualsFromNodes(LHCb::TbTrack* track);
};
#endif
