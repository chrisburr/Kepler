#include <algorithm>
#include <fstream>
#include <sstream>

// Local
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbCondFile.h"
#include "TbKernel/ITbDataSvc.h"
#include "TbPixelSvc.h"

DECLARE_SERVICE_FACTORY(TbPixelSvc)

//============================================================================
// Constructor
//============================================================================
TbPixelSvc::TbPixelSvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc) {
  declareProperty("WriteTrimDACs", m_writeTrimDACs = false);
  declareProperty("IgnoreMask", m_ignoreMask = false);
}

//============================================================================
// Destructor
//============================================================================
TbPixelSvc::~TbPixelSvc() {}

//============================================================================
// Initialisation
//============================================================================
StatusCode TbPixelSvc::initialize() {
  StatusCode sc = Service::initialize();
  if (!sc.isSuccess()) return sc;
  // Get the number of chips.
  const unsigned int nDevices = geomSvc()->nDevices();
  m_pixelConfiguration.resize(nDevices);

  m_protectPhase = std::vector<bool>(nDevices, 0);
  for (unsigned int i = 0; i < nDevices; ++i) {
    m_pixelConfiguration[i].resize(Tb::NPixels, PixelConfig());
  }
  // Retrieve the names of the configuration files to be parsed.
  ITbDataSvc* dataSvc = Gaudi::svcLocator()->service<ITbDataSvc>("TbDataSvc");
  std::vector<std::string> files = dataSvc->getPixelConfig();
  files.push_back(dataSvc->getTimingConfig());
  for (auto& f : files) {
    if (f == "") continue;
    info() << "Importing pixel configuration from " << f << endmsg;
    // Import conditions from configuration file.
    if (!readConditions(f)) {
      error() << "Cannot import pixel configuration" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//============================================================================
// Add a constant offset in time to a single super column
//============================================================================
void TbPixelSvc::addOffset(const int offset, const unsigned int device,
                           const unsigned int dcol) {
  for (unsigned int pix = 0; pix < 512; ++pix) {
    const unsigned int address = pix + (0xFE00 & (dcol << 9));
    m_pixelConfiguration[device][address].tOffset += offset;
  }
}

//============================================================================
// Export trim DACs and masked pixel flags to a text file
//============================================================================
void TbPixelSvc::writeTrimDAC(const unsigned int device) {
  std::ofstream trimdac;
  trimdac.open(("plane_" + std::to_string(device) + "_trimdac.txt").c_str());
  trimdac << "#col  row trim mask tp_ena " << std::endl;
  for (unsigned int col = 0; col < Tb::NCols; ++col) {
    for (unsigned int row = 0; row < Tb::NRows; ++row) {
      const unsigned int pixel = address(col, row);
      trimdac << std::setw(3) << col << std::setw(4) << row << std::setw(4)
              << (int)(m_pixelConfiguration[device][pixel].trim) << std::setw(4)
              << m_pixelConfiguration[device][pixel].trimDac_isMasked
              << std::setw(4) << m_pixelConfiguration[device][pixel].tp_ena
              << std::endl;
    }
  }
  trimdac.close();
}

//============================================================================
// Set trim DACs for the entire pixel matrix of a chip
//============================================================================
void TbPixelSvc::setTrim(const unsigned int device, const char* data) {
  unsigned int hash = 2166136261;
  for (unsigned int col = 0; col < Tb::NCols; ++col) {
    for (unsigned int row = 0; row < Tb::NRows; ++row) {
      const unsigned int pixel = address(col, row);
      const uint8_t word = *(data + col + Tb::NRows * row);
      hash ^= word;
      hash *= 16777619;
      m_pixelConfiguration[device][pixel].trim = 0xF & (word >> 1);
      m_pixelConfiguration[device][pixel].tp_ena = 0x1 & (word >> 5);
      m_pixelConfiguration[device][pixel].trimDac_isMasked = 0x1 & word;
    }
  }
  info() << "Hash for device " << device << ": " << hash << endmsg;
  if (m_writeTrimDACs) writeTrimDAC(device);
}

void TbPixelSvc::resetClockPhase(const unsigned int device) {
  for (unsigned int pixel = 0; pixel < Tb::NPixels; ++pixel) {
    m_pixelConfiguration[device][pixel].tOffset = 0;
  }
}

//============================================================================
// Calculate the clock phase correction.
//============================================================================
void TbPixelSvc::setPhase(const unsigned int device,
                          const unsigned int pll_config = 1024,
                          const int amplitude = 1) {
  if (m_protectPhase[device]) return;
  const unsigned int phase = pow(2, (pll_config & 0x1C0) >> 6);
  if (phase == 1) return;

  for (unsigned int pixel = 0; pixel < Tb::NPixels; ++pixel) {
    // Decode the pixel address, first the double column.
    const unsigned int dcol = (0xFE00 & pixel) >> 9;
    // Get the pixel within the super pixel.
    const unsigned int pix = (0x0007 & pixel);
    // Calculate the column number.
    const unsigned int col = 2 * dcol + pix / 4;
    // col = col % 128;
    int dt = 0;
    if (phase == 16) {
      dt = (-((16 - ((col / 2) % 16)) % 16)) << 8;
    } else if (phase == 4) {
      dt = (-4 * ((4 - ((col / 2) % 4)) % 4)) << 8;
    } else if (phase == 2) {
      dt = (((col % 4) < 2) ? 8 : 0) << 8;
    }
    m_pixelConfiguration[device][pixel].tOffset += amplitude * dt;
  }
  m_protectPhase[device] = true;
}

//============================================================================
// Import pixel conditions from configuration file
//============================================================================
bool TbPixelSvc::readConditions(const std::string& filename) {
  TbCondFile f(filename);
  if (!f.is_open()) {
    warning() << "Cannot open " << filename << endmsg;
    return false;
  }
  std::string mode = "";
  std::string chip = "";
  int deviceIndex = -1;
  unsigned int col(0), row(0);
  std::vector<unsigned int> badPixels;
  while (!f.eof()) {
    std::string line = "";
    if (!f.getLine(line)) continue;
    if (line.find("Charge") != std::string::npos) {
      f.split(line, ' ', mode, chip);
      deviceIndex = geomSvc()->deviceIndex(chip);
      if (deviceIndex == 999) {
        error() << "Device " << chip << " is not in the alignment file"
                << endmsg;
        error() << "Stop parsing " << filename << endmsg;
        break;
      }
    } else if (line.find("Mask") != std::string::npos ||
               line.find("Offset") != std::string::npos ||
               line.find("Timing") != std::string::npos ||
               line.find("GlobalCharge") != std::string::npos ||
               line.find("PLL") != std::string::npos) {
      mode = line;
    } else if (mode == "Charge") {
      float a(0.), b(0.), c(0.), t(0.);
      f.split(line, ' ', col, row, a, b, c, t);
      const unsigned int pixel = address(col, row);

      if (isnan(a) || isinf(a) || isnan(b) || isnan(c) || isnan(t) ||
          isinf(b) || isinf(c) || isinf(t)) {
        badPixels.push_back(pixel);
      }
      m_pixelConfiguration[deviceIndex][pixel].p0 = a;
      m_pixelConfiguration[deviceIndex][pixel].p1 = b;
      m_pixelConfiguration[deviceIndex][pixel].c = c;
      m_pixelConfiguration[deviceIndex][pixel].t = t;
    } else if (mode == "Mask") {
      f.split(line, ' ', chip, col, row);
      deviceIndex = geomSvc()->deviceIndex(chip);
      if (deviceIndex == 999) {
        error() << "Device " << chip << " is not in the alignment file"
                << endmsg;
        continue;
      }
      const unsigned int pixel = address(col, row);
      info() << "Masking pixel " << format("0x%04x", pixel)
             << format(" (column %3d", col) << format(", row %3d)", row)
             << " on device " << chip << endmsg;
      m_pixelConfiguration[deviceIndex][pixel].isMasked = true;
    } else if (mode == "GlobalCharge") {
      double a(0.), b(0.), c(0.), t(0.);
      f.split(line, ' ', chip, a, b, c, t);
      //     if( isnan(a) || isnan(b) || isnan(c) || isnan(d) )

      deviceIndex = geomSvc()->deviceIndex(chip);
      if (deviceIndex == 999) {
        error() << "Device " << chip << " is not in the alignment file"
                << endmsg;
        continue;
      }
      for (auto& pixel : m_pixelConfiguration[deviceIndex]) {
        pixel.p0 = a;
        pixel.p1 = b;
        pixel.c = c;
        pixel.t = t;
      }
      info() << "Applying global charge calibration on device " << chip
             << "(a = " << a << ", b = " << b << ", c = " << c << ", t = " << t
             << ")" << endmsg;
    } else if (mode == "Offset") {
      int offset = 0;
      f.split(line, ' ', chip, col, offset);
      deviceIndex = geomSvc()->deviceIndex(chip);
      if (deviceIndex == 999) {
        error() << "Device " << chip << " is not in the alignment file"
                << endmsg;
        continue;
      }
      info() << "Adding time offset " << offset * 25 << " ns "
             << "to double column " << col << " on chip " << chip << endmsg;
      addOffset(offset * Tb::ToA, deviceIndex, col);
    } else if (mode == "Timing") {
      double dt = 0.;
      f.split(line, ' ', chip, dt);
      deviceIndex = geomSvc()->deviceIndex(chip);
      if (deviceIndex == 999) {
        error() << "Device " << chip << " is not in the alignment file"
                << endmsg;
        continue;
      }
      const int offset = static_cast<int>(std::round(dt * Tb::nanosecond));
      info() << "Adding time offset " << dt << " ns (" << offset
             << " time units) to chip " << chip << endmsg;
      const unsigned int nDCols = Tb::NCols / 2;
      for (unsigned int col = 0; col < nDCols; ++col) {
        addOffset(offset, deviceIndex, col);
      }
    } else if (mode == "PLL") {
      int amplitude = 0.;
      unsigned int pll_mode;
      f.split(line, ' ', chip, pll_mode, amplitude);
      deviceIndex = geomSvc()->deviceIndex(chip);
      if (deviceIndex == 999) {
        error() << "Device " << chip << " is not in the alignment file"
                << endmsg;
        continue;
      }
      info() << "Overwriting PLL CONFIG for chip " << chip << endmsg;
      m_protectPhase[deviceIndex] = false;
      resetClockPhase(deviceIndex);
      setPhase(deviceIndex, pll_mode, amplitude);
    }
  }
  for (auto& pixel : badPixels) {
    auto pos = posFromAddress(pixel);
    float avgt(0.), avgc(0.), avgp0(0.), avgp1(0.);
    unsigned int nPixels = 0;
    if (pos.second != 255) {
      unsigned int neighbour = address(pos.first, pos.second + 1);
      if (std::find(badPixels.begin(), badPixels.end(), neighbour) ==
          badPixels.end()) {
        avgt += m_pixelConfiguration[deviceIndex][neighbour].t;
        avgc += m_pixelConfiguration[deviceIndex][neighbour].c;
        avgp0 += m_pixelConfiguration[deviceIndex][neighbour].p0;
        avgp1 += m_pixelConfiguration[deviceIndex][neighbour].p1;
        ++nPixels;
      }
    }
    if (pos.second != 0) {
      unsigned int neighbour = address(pos.first, pos.second - 1);
      if (std::find(badPixels.begin(), badPixels.end(), neighbour) ==
          badPixels.end()) {
        avgt += m_pixelConfiguration[deviceIndex][neighbour].t;
        avgc += m_pixelConfiguration[deviceIndex][neighbour].c;
        avgp0 += m_pixelConfiguration[deviceIndex][neighbour].p0;
        avgp1 += m_pixelConfiguration[deviceIndex][neighbour].p1;
        ++nPixels;
      }
    }
    if (nPixels == 0) {
      m_pixelConfiguration[deviceIndex][pixel].isMasked = true;
      // printConfig(deviceIndex, pixel);
      continue;
    }
    m_pixelConfiguration[deviceIndex][pixel].t = avgt / (float)nPixels;
    m_pixelConfiguration[deviceIndex][pixel].p0 = avgp0 / (float)nPixels;
    m_pixelConfiguration[deviceIndex][pixel].p1 = avgp1 / (float)nPixels;
    m_pixelConfiguration[deviceIndex][pixel].c = avgc / (float)nPixels;
    // printConfig(deviceIndex, pixel);
  }
  f.close();
  return true;
}
