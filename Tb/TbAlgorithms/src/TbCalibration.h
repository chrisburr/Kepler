#pragma once

// Tb/TbEvent
#include "Event/TbCluster.h"

// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"

/** @class TbCalibration TbCalibration.h
 *
 *  Algorithm to produce timing and pixel configurations
 *  Try to make as independent as possible from the rest of Kepler
 *  (Relies on clustering and EventBuilder only)
 *
 *  @author T. Evans
 *
 */

class TbCalibration : public TbAlgorithm {
 public:
  /// Constructor
  TbCalibration(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbCalibration() {};

  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution
  virtual StatusCode finalize();

 private:
  // profile structure
  struct POINT {
    POINT() : total(0), nData(0) {};
    double total;
    double nData;
    double avg() const { return nData == 0 ? 0 : total / nData; }
    double val() const { return total; }
    double n() const { return nData; }
    void add(double data) {
      total += data;
      nData++;
    }
  };

  struct PROFILE1D : public std::vector<POINT> {
    PROFILE1D(unsigned int size = 0) : std::vector<POINT>(size, POINT()) {}
  };

  struct PROFILE2D : public std::vector<std::vector<POINT>> {
    PROFILE2D(unsigned int size_x = 0, unsigned int size_y = 0)
        : std::vector<std::vector<POINT>>(
              size_x, std::vector<POINT>(size_y, POINT())) {};

    inline POINT element(const unsigned int x, const unsigned int y) const {
      std::vector<POINT> tmp = *(begin() + x);
      return tmp[y];
    }

    std::vector<double> neighbours(const unsigned int x,
                                   const unsigned int y) const {
      std::vector<double> return_value;
      return_value.clear();
      unsigned int ix_begin = x > 0 ? x - 1 : 0;
      unsigned int ix_end = x < size() - 1 ? x + 1 : size() - 1;
      unsigned int iy_begin = y > 0 ? y - 1 : 0;
      unsigned int iy_end = y < size() - 1 ? y + 1 : size() - 1;
      for (unsigned int ix = ix_begin; ix <= ix_end; ++ix) {
        for (unsigned int iy = iy_begin; iy <= iy_end; ++iy)
          if (!(ix == x && iy == y))
            return_value.push_back(element(ix, iy).n());
      }
      return return_value;
    }
  };

  /// TES location of tracks
  std::string m_trackLocation;
  /// TES location of clusters
  std::string m_clusterLocation;
  std::string m_hitLocation;
  std::string m_pixelSvcConfig;
  std::string m_timingSvcConfig;

  bool m_checkHotPixels;
  bool m_checkSyncronisation;
  bool m_checkColumnOffsets;
  unsigned int m_syncMethod;
  unsigned int m_dut;  /// synchronisation of the DUT is kept separate and
                       /// doesn't affect the telescope timing
  std::vector<PROFILE1D> m_offsets;
  std::vector<PROFILE2D> m_hitMaps;
  PROFILE1D m_sync;
  // hot pixel identification, analyses the work done by execute
  void hotPixelAnalysis(const PROFILE2D& hitMap, const std::string& plane,
                        std::ostream& os = std::cout);
  void hotPixel_execute(const LHCb::TbHits* hits);

  void syncAnalysis(const double& sync, const std::string& plane,
                    std::ostream& os = std::cout);
  void sync_execute(const std::vector<LHCb::TbClusters*>& clusters);

  double nearestHit(const LHCb::TbCluster* cluster,
                    const LHCb::TbClusters* clusters);

  void columnOffsetAnalysis(const PROFILE1D& avg_difference,
                            const std::string& plane,
                            std::ostream& os = std::cout);
  void columnOffset_execute(const LHCb::TbClusters* clusters);

  void syncOffset2(std::ostream& os = std::cout);
  class lowerBound {
   public:
    bool operator()(const LHCb::TbCluster* lhs, const double t) const {
      return lhs->htime() < t;
    }
  };
};
