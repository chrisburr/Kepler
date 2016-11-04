#ifndef TBPIXELSVC_H
#define TBPIXELSVC_H

// Gaudi
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Bootstrap.h"

// Local
#include "TbKernel/ITbPixelSvc.h"
#include "TbKernel/ITbGeometrySvc.h"

#include <stdint.h>

template <class TYPE>
class SvcFactory;

/** @class TbPixelSvc TbPixelSvc.h
 *
 * Implementation of the Timepix3 pixel configuration service.
 *
 */

class TbPixelSvc : public extends1<Service, ITbPixelSvc> {
 public:
  /// Constructor
  TbPixelSvc(const std::string& name, ISvcLocator* svc);
  /// Destructor
  virtual ~TbPixelSvc();

  virtual StatusCode initialize();

  /// Get the pixel address for a given column and row.
  virtual unsigned int address(const unsigned int col,
                               const unsigned int row) const {
    // Get the double-column and super-pixel.
    const unsigned int dcol = col / 2;
    const unsigned int spix = row / 4;
    // Get the pixel number within the superpixel.
    unsigned int pix = row % 4;
    if (1 == col % 2) pix += 4;
    return (dcol << 9) + (spix << 3) + pix;
  }
  /// Get the column and row for a given pixel address.
  std::pair<unsigned int, unsigned int> posFromAddress(
      const unsigned int address) const {
    // Decode the pixel address, first get the double column.
    const unsigned int dcol = (0xFE00 & address) >> 8;
    // Get the super pixel address.
    const unsigned int spix = (0x01F8 & address) >> 1;
    // Get the address of the pixel within the super pixel.
    const unsigned int pix = (0x0007 & address);
    // Calculate the row and column numbers.
    return std::make_pair(dcol + pix / 4, spix + (pix & 0x3));
  }

  /// Return whether the pixel has been masked.
  virtual bool isMasked(const unsigned int address,
                        const unsigned int device) const {
    return m_pixelConfiguration[device][address].isMasked ||
           (!m_ignoreMask &&
            m_pixelConfiguration[device][address].trimDac_isMasked);
  }

  /// Correct the timestamp of a given hit.
  virtual void applyPhaseCorrection(LHCb::TbHit* h) {
    h->setTime(h->time() +
               m_pixelConfiguration[h->device()][h->pixelAddress()].tOffset);
  }

  /// Set the clock phase correction.
  virtual void setPhase(const unsigned int device,
                        const unsigned int pll_config, const int amplitude);

  /// Set trim DACs.
  virtual void setTrim(const unsigned int device, const char* data);

  /// Convert time-over-threshold to charge.
  virtual double charge(const unsigned int tot, const unsigned int address,
                        const unsigned int device) const {
    const PixelConfig& conf = m_pixelConfiguration[device][address];
    double value = inverseSurrogate(tot, conf.p1, conf.p0, conf.c, conf.t);
    if (isnan(value)) {
      const auto pos = posFromAddress(address);
      warning() << "Pixel " << device << "/0x" << std::hex << address
                << std::dec << " tot = " << tot << " (" << pos.first << ", "
                << pos.second << ")" << conf.p1 << "  " << conf.p0 << "  "
                << conf.c << "  " << conf.t << endmsg;
      return 1;
    }
    return value;
  }

 private:
  bool m_ignoreMask;
  /// Allow SvcFactory to instantiate the service.
  friend class SvcFactory<TbPixelSvc>;

  struct PixelConfig {
    PixelConfig()
        : isMasked(false),
          trimDac_isMasked(false),
          isDead(false),
          tp_ena(0),
          trim(4),
          tOffset(0),
          p0(0),
          p1(1),
          c(0),
          t(0){};
    bool isMasked;
    bool trimDac_isMasked;
    bool isDead;
    bool tp_ena;
    uint8_t trim;
    int tOffset;
    /// Parameters of surrogate function
    float p0;
    float p1;
    float c;
    float t;
  };

  void resetClockPhase(const unsigned int device);
  /// Add a constant offset in time to a single super column.
  void addOffset(const int offset, const unsigned int device,
                 const unsigned int dcol);

  /// Pixel configuration for each chip
  std::vector<std::vector<PixelConfig> > m_pixelConfiguration;

  void printConfig(const unsigned int plane, const unsigned int address) {
    auto pos = posFromAddress(address);
    PixelConfig& conf = m_pixelConfiguration[plane][address];
    info() << std::hex << "0x" << address << std::dec << " = (" << pos.first
           << ", " << pos.second << ") : " << conf.p0 << " " << conf.p1 << " "
           << conf.c << "  " << conf.t << endmsg;
  }
  /// Functionality to write out the trim dacs written into the header
  bool m_writeTrimDACs;
  void writeTrimDAC(const unsigned int device);

  /// Ignore PLL config
  std::vector<bool> m_protectPhase;
  /// Surrogate function, taken from http://arxiv.org/pdf/1103.2739v3.pdf
  double surrogate(const double charge, const double a, const double b,
                   const double c, const double t) const {
    double result = 0.;
    const double r = (b + a * t);
    const double d = r * r + 4 * a * c;
    if (d > 0.) {
      const double itcpt = ((t * a - b) + sqrt(d)) / (2 * a);
      if (charge > itcpt) {
        result = a * charge + b - c / (charge - t);
      }
    }
    return result;
  }

  double inverseSurrogate(const uint32_t tot, const double a, const double b,
                          const double c, const double t) const {
    const double r = (b + a * t - tot);
    const double d = r * r + 4. * a * c;
    return d > 0. ? (a * t + tot - b + sqrt(d)) / (2. * a) : 1.;
  }

  bool readConditions(const std::string& file);

  /// Pointer to geometry service
  mutable ITbGeometrySvc* m_geomSvc = nullptr;
  /// Access geometry service on-demand
  ITbGeometrySvc* geomSvc() const {
    if (!m_geomSvc) {
      m_geomSvc = Gaudi::svcLocator()->service<ITbGeometrySvc>("TbGeometrySvc");
    }
    return m_geomSvc;
  }
};

#endif
