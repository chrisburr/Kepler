#ifndef TB_CLUSTERING_H
#define TB_CLUSTERING_H 1

// Tb/TbEvent
#include "Event/TbHit.h"
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbClustering TbClustering.h
 *
 *  Algorithm to group together touching Timepix3 pixel hits.
 *
 *  @author Dan Saunders
 */

class TbClustering : public TbAlgorithm {
 public:
  /// Constructor
  TbClustering(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbClustering() {}

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  /// TES location prefix of hits
  std::string m_hitLocation;
  /// TES location prefix of clusters
  std::string m_clusterLocation;
  /// Time window (in ns)
  double m_twindow;
  /// Max. distance between two pixels to be grouped together
  int m_searchDist;
  /// Cluster error model
  int m_clusterErrorMethod;
  /// Eta-correction parameterisation.
  struct EtaCorrection {
    double xmin, xmax;
    std::vector<double> coefficients;
  };
  /// Set of eta-correction parameters per plane, direction, and cluster width.
  std::vector<std::array<std::vector<std::vector<EtaCorrection> >, 2> > m_eta;

  /// Add pixels to a given seed pixel.
  void addNeighbouringHits(std::vector<const LHCb::TbHit*>& pixels,
                           LHCb::TbHits::const_iterator begin,
                           LHCb::TbHits::const_iterator end,
                           std::vector<bool>& used);
  /// Check if a hit touches any of the pixels in a given cluster.
  bool hitTouchesCluster(const int scol, const int row,
                         const std::vector<const LHCb::TbHit*>& pixels) const {

    bool result = false;
    for (auto it = pixels.cbegin(), end = pixels.cend(); it != end; ++it) {
      if ((abs(int((*it)->scol()) - scol) <= m_searchDist) &&
          (abs(int((*it)->row()) - row) <= m_searchDist)) {
        result = true;
        break;
      }
    }
    return result;
  }

  /// Set cluster attributes (position, time, ADC).
  void completeCluster(const unsigned int plane,
                       const std::vector<const LHCb::TbHit*>& pixels,
                       LHCb::TbClusters* clusters);
  /// Calculate the cluster errors.
  void setClusterError(LHCb::TbCluster* cluster) const;
  /// Calculate the eta-correction.
  void etaCorrection(double& xLocal, double& yLocal,
                     const unsigned int nCols, const unsigned int nRows,
                     const unsigned int plane) const;
  void readEta(const std::string& filename);

};
#endif
