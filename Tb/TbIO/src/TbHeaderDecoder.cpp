// Boost
#include "boost/format.hpp"

// Local
#include "TbHeaderDecoder.h"

DECLARE_TOOL_FACTORY(TbHeaderDecoder)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TbHeaderDecoder::TbHeaderDecoder(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
    : GaudiTool(type, name, parent), 
      m_print(true), m_geomSvc(nullptr), m_pixelSvc(nullptr) {

  declareInterface<TbHeaderDecoder>(this);
}

//=============================================================================
// Destructor
//=============================================================================
TbHeaderDecoder::~TbHeaderDecoder() {}

//=============================================================================
// Initialisation
//=============================================================================
StatusCode TbHeaderDecoder::initialize() {

  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Read the first fields of the Spidr header from a given file.
//=============================================================================
bool TbHeaderDecoder::readCommon(const char* data, unsigned int& size,
                                 unsigned int& fmt) {

  struct CommonHeader {
    uint32_t headerId;
    uint32_t headerSizeTotal;
    uint32_t headerSize;
    uint32_t format;
  };
  CommonHeader* hdr = (CommonHeader*)data;
  size = hdr->headerSizeTotal;
  if (size > 66304) size = 66304;
  fmt = hdr->format;
  return true;
}

//=============================================================================
// Read the Spidr header from a given file.
//=============================================================================
bool TbHeaderDecoder::read(const char* data, const unsigned int& fmt,
                           std::string& deviceId) {

  if (fmt == 0x30 || fmt == 0x1) {
    return readV1(data, deviceId);
  }
  error() << format("Unknown format (0x%x)", fmt) << endmsg;
  return false;
}


//=============================================================================
// Read version 1 of the Spidr header.
//=============================================================================
bool TbHeaderDecoder::readV1(const char* data, std::string& deviceId) {

  struct Tpx3Header {
    uint32_t headerId;
    uint32_t headerSizeTotal;
    uint32_t headerSize;
    uint32_t format;
    uint32_t deviceId;
    uint32_t genConfig;
    uint32_t outblockConfig;
    uint32_t pllConfig;
    uint32_t testPulseConfig;
    uint32_t slvsConfig;
    uint32_t pwrPulseConfig;
    uint32_t dac[32];
    uint8_t ctpr[256 / 8];
    uint32_t unused[64 - 11 - 32 - (256 / 8) / 4];
  };

  struct SpidrTpx3Header {
    uint32_t headerId;
    uint32_t headerSizeTotal;
    uint32_t headerSize;
    uint32_t format;
    uint32_t spidrId;
    uint32_t libVersion;
    uint32_t softwVersion;
    uint32_t firmwVersion;
    uint32_t ipAddress;
    uint32_t ipPort;
    uint32_t yyyyMmDd;
    uint32_t hhMmSsMs;
    uint32_t runNr;
    uint32_t seqNr;
    uint32_t spidrConfig;  // Trigger mode, decoder on/off
    uint32_t spidrFilter;
    uint32_t spidrBiasVoltage;
    // Spare
    uint32_t unused[128 - 17 - 128 / 4];
    // Description string
    char descr[128];
    // Device header
    Tpx3Header devHeader;
  };

  SpidrTpx3Header* hdr = (SpidrTpx3Header*)data;
  Tpx3Header devHdr = hdr->devHeader;
  // Get the chip id.
  const int devid0 = (hdr->devHeader.deviceId >> 8) & 0xFFF;
  const int devid1 = (hdr->devHeader.deviceId >> 4) & 0xF;
  const int devid2 = (hdr->devHeader.deviceId >> 0) & 0xF;
  const char devidchar = (char)(devid2 + 64);
  deviceId = str(boost::format("W%04d_%d%02d") % devid0 % devidchar % devid1);

  // Transfer the header information to the pixel configuration service.
  const unsigned int deviceIndex = geomSvc()->deviceIndex(deviceId);
  if (deviceIndex == 999) {
    error() << deviceId << " is not listed in the alignment file." << endmsg;
    return false;
  } 
  pixelSvc()->setPhase(deviceIndex, devHdr.pllConfig);
  if (devHdr.headerSizeTotal - devHdr.headerSize == 65536) {
    const char* trimDac = data + (hdr->headerSize + devHdr.headerSize);
    pixelSvc()->setTrim(deviceIndex, trimDac);
  }
  if (!m_print) return true;

  boost::format fmts(" %|-30.30s|%|32t| %s ");
  boost::format fmtx(" %|-30.30s|%|32t| 0x%x ");
  boost::format fmt8x(" %|-30.30s|%|32t| 0x%08x ");
  boost::format fmtd(" %|-30.30s|%|32t| %d ");
  info() << std::string(70, '-') << endmsg;
  info() << "                        File header " << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << fmtx % "ID" % hdr->headerId << endmsg;
  info() << fmtd % "Total size" % hdr->headerSizeTotal << endmsg;
  info() << fmtd % "Size" % hdr->headerSize << endmsg;
  info() << fmt8x % "Format" % hdr->format << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << fmtx % "SPIDR ID" % hdr->spidrId << endmsg;
  info() << fmtx % "Library version" % hdr->libVersion << endmsg;
  info() << fmtx % "Software version" % hdr->softwVersion << endmsg;
  info() << fmtx % "Firmware version" % hdr->firmwVersion << endmsg;
  info() << std::string(70, '-') << endmsg;
  const int ip0 = (hdr->ipAddress >> 24) & 0xFF;
  const int ip1 = (hdr->ipAddress >> 16) & 0xFF;
  const int ip2 = (hdr->ipAddress >> 8) & 0xFF;
  const int ip3 = (hdr->ipAddress >> 0) & 0xFF;
  const std::string ip = std::to_string(ip0) + "." + std::to_string(ip1) + "." +
                         std::to_string(ip2) + "." + std::to_string(ip3);
  info() << fmts % "IP address" % ip << endmsg;
  info() << fmtd % "IP port" % hdr->ipPort << endmsg;
  info() << std::string(70, '-') << endmsg;
  const int date0 = (hdr->yyyyMmDd >> 16) & 0xFFFF;
  const int date1 = (hdr->yyyyMmDd >> 8) & 0xFF;
  const int date2 = (hdr->yyyyMmDd >> 0) & 0xFF;
  const int time0 = (hdr->hhMmSsMs >> 24) & 0xFF;
  const int time1 = (hdr->hhMmSsMs >> 16) & 0xFF;
  const int time2 = (hdr->hhMmSsMs >> 8) & 0xFF;
  const int time3 = (hdr->hhMmSsMs >> 0) & 0xFF;
  info() << boost::format(" %|-30.30s|%|32t| %04x-%02x-%02x") % "Date" % date0 %
                date1 % date2 << endmsg;
  info() << boost::format(" %|-30.30s|%|32t| %02x:%02x:%02x.%02x") % "Time" %
                time0 % time1 % time2 % time3 << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << fmtd % "Run number" % hdr->runNr << endmsg;
  info() << fmtd % "Sequence number" % hdr->seqNr << endmsg;
  info() << fmts % "Run info" % hdr->descr << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << fmt8x % "Spidr configuration" % hdr->spidrConfig << endmsg;
  info() << fmt8x % "Data filter" % hdr->spidrFilter << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << "                       Device header" << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << fmtx % "Type ID" % devHdr.headerId << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << fmtx % "Device ID" % devHdr.deviceId << endmsg;
  info() << fmts % "" % deviceId << endmsg;
  info() << std::string(70, '-') << endmsg;
  info() << fmt8x % "General config" % devHdr.genConfig << endmsg;
  info() << fmt8x % "Output block config" % devHdr.outblockConfig << endmsg;
  info() << fmt8x % "PLL config" % devHdr.pllConfig << endmsg;
  info() << fmt8x % "Testpulse config" % devHdr.testPulseConfig << endmsg;
  info() << fmt8x % "SLVS config" % devHdr.slvsConfig << endmsg;
  info() << fmt8x % "Power pulsing config" % devHdr.pwrPulseConfig << endmsg;
  info() << std::string(70, '-') << endmsg;
  boost::format fmt(" %|-30.30s|%|32t| %03d %03d %03d %03d");
  std::string title = "DAC values";
  for (unsigned int i = 0; i < 5; ++i) {
    const unsigned int offset = 4 * i;
    info() << fmt % title % devHdr.dac[offset] % devHdr.dac[offset + 1] %
                  devHdr.dac[offset + 2] % devHdr.dac[offset + 3] << endmsg;
    title = "";
  }
  info() << std::string(70, '-') << endmsg; 
  return true;
}
