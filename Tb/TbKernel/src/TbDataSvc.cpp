#include <algorithm>
#include <fstream>
#include <sstream>

// Boost
#include <boost/filesystem.hpp>

// Local
#include "TbKernel/TbConstants.h"
#include "TbKernel/TbCondFile.h"
#include "TbDataSvc.h"

DECLARE_SERVICE_FACTORY(TbDataSvc)

namespace fs = boost::filesystem;

//============================================================================
// Constructor
//============================================================================
TbDataSvc::TbDataSvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc) {

  declareProperty("AlignmentFile", m_alignmentFile = "");
  declareProperty("TimingConfigFile", m_timingFile = "");
  declareProperty("PixelConfigFile", m_pixelFiles);
  declareProperty("EtaConfigFiles", m_etaFiles);
  declareProperty("Input", m_inputs);
}

//============================================================================
// Destructor
//============================================================================
TbDataSvc::~TbDataSvc() {}

//============================================================================
// Finalisation
//============================================================================
StatusCode TbDataSvc::finalize() { return Service::finalize(); }

//============================================================================
// Add the prefix for reading files from EOS/CASTOR
//============================================================================
std::string TbDataSvc::expandPath(const std::string& file) {

  if (file.find("eos") == 0) {
    return "root://eoslhcb.cern.ch//" + file;
  }
  return file;
}

//============================================================================
// Initialisation
//============================================================================
StatusCode TbDataSvc::initialize() {

  StatusCode sc = Service::initialize();
  if (!sc.isSuccess()) return sc;

  if (m_inputs.empty()) {
    error() << "No input files specified." << endmsg;
    return StatusCode::FAILURE;
  }
  for (auto its = m_inputs.cbegin(), end = m_inputs.cend(); its != end; ++its) {
    if (its->find("eos") == 0) {
      // EOS path.
      if (its->find(".dat") != std::string::npos) {
        // Specified path is a raw data file. Add it to the list.
        m_inputFiles.push_back(expandPath(*its));
        continue;
      }
      // Assume the specified path is a directory and list its contents.
      const std::string eospath =
          "/afs/cern.ch/project/eos/installation/0.3.15/";
      const std::string cmd = eospath + "bin/eos.select ls " + *its;
      FILE* proc = popen(cmd.c_str(), "r");
      char buf[4096];
      // Loop over the entries.
      while (!feof(proc) && fgets(buf, sizeof(buf), proc)) {
        std::string fname = buf;
        // Skip non-.dat files.
        const size_t p = fname.find(".dat");
        if (p == std::string::npos) continue;
        // Add the file to the list.
        fname = fname.substr(0, p + 4);
        m_inputFiles.push_back(expandPath(*its + fname));
      }
      pclose(proc);
    } else {
      // Local file/directory.
      fs::path path(*its);
      if (!fs::exists(path)) {
        warning() << "File or directory " << *its << " not found." << endmsg;
        continue;
      }
      if (fs::is_directory(path)) {
        fs::directory_iterator itdEnd;
        for (fs::directory_iterator itd(path); itd != itdEnd; ++itd) {
          if (is_regular_file(itd->status()))
            m_inputFiles.push_back((*itd).path().string());
        }
      } else
        m_inputFiles.push_back(*its);
    }
  }

  if (m_inputFiles.empty()) {
    error() << "No input files specified." << endmsg;
    return StatusCode::FAILURE;
  }
  for (auto& f : m_inputFiles) info() << "Reading File: " << f << endmsg;

  m_alignmentFile = expandPath(m_alignmentFile);
  m_timingFile = expandPath(m_timingFile);
  for (auto& f : m_pixelFiles) f = expandPath(f);
  for (auto& f : m_etaFiles) f = expandPath(f);
  return StatusCode::SUCCESS;
}
