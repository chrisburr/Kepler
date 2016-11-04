#include <fstream>

// ROOT
#include "Math/Translation3D.h"
#include "Math/RotationZYX.h"

// Tb/TbEvent
#include "Event/TbTrack.h"

// Local
#include "TbKernel/TbCondFile.h"
#include "TbKernel/ITbDataSvc.h"
#include "TbKernel/TbFunctors.h"
#include "TbKernel/TbConstants.h"
#include "TbGeometrySvc.h"

DECLARE_SERVICE_FACTORY(TbGeometrySvc)

//============================================================================
// Constructor
//============================================================================
TbGeometrySvc::TbGeometrySvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc),
      m_modules(),
      m_moduleIndex(),
      m_deviceIndex(),
      m_planes(),
      m_nDevices(0) {}

//============================================================================
// Destructor
//============================================================================
TbGeometrySvc::~TbGeometrySvc() {

  // Delete the modules.
  for (auto it = m_modules.begin(), end = m_modules.end(); it != end; ++it) {
    if (*it) delete (*it);
  }
}

//============================================================================
// Initialisation
//============================================================================
StatusCode TbGeometrySvc::initialize() {

  StatusCode sc = Service::initialize();
  if (!sc.isSuccess()) return sc;

  const std::string& filename =
      Gaudi::svcLocator()->service<ITbDataSvc>("TbDataSvc")->getAlignmentFile();
  info() << "Importing alignment conditions from " << filename << endmsg;
  // Import geometry conditions from alignment file.
  if (!readConditions(filename, m_modules)) {
    error() << "Cannot import alignment conditions" << endmsg;
    return StatusCode::FAILURE;
  }
  // Sort the modules by z-position.
  std::sort(m_modules.begin(), m_modules.end(),
            TbFunctors::LessByZ<const TbModule*>());
  // Loop over the modules.
  for (auto it = m_modules.begin(), end = m_modules.end(); it != end; ++it) {
    // Map the name of the module to its index in the list.
    const unsigned index = it - m_modules.begin();
    const std::string name = (*it)->id();
    m_moduleIndex[name] = index;
    for (auto ic = (*it)->chips().begin(); ic != (*it)->chips().end(); ++ic)
      m_planes[ic->id] = index;
  }
  // Cache the x-coordinates of the pixel centres for sensor tiles.
  m_xTriple.resize(3 * Tb::NCols);
  for (unsigned int chip = 0; chip < 3; ++chip) {
    const double x0 = chip * (Tb::NCols + 2) * Tb::PixelPitch;
    const unsigned int offset = chip * Tb::NCols;
    for (unsigned int col = 0; col < Tb::NCols; ++col) {
      const unsigned int scol = offset + col;
      double x = x0 + (0.5 + col) * Tb::PixelPitch;
      if (scol == 256 || col == 512) {
        x -= 0.5 * Tb::PixelPitch;
      } else if (scol == 255 || col == 511) {
        x += 0.5 * Tb::PixelPitch;
      }
      m_xTriple[scol] = x;
    }
  }
  printAlignment(m_modules);
  return StatusCode::SUCCESS;
}

//============================================================================
// Finalisation
//============================================================================
StatusCode TbGeometrySvc::finalize() { return Service::finalize(); }

//============================================================================
// Calculate the local coordinates (pixel centre) of a given pixel
//============================================================================
bool TbGeometrySvc::pixelToPoint(const unsigned int scol,
                                 const unsigned int row,
                                 const unsigned int plane, double& x,
                                 double& y) {

  if (m_modules[plane]->type() == TbModule::Tpx3) {
    x = (scol + 0.5) * Tb::PixelPitch;
    y = (row + 0.5) * Tb::PixelPitch;
    return true;
  } else if (m_modules[plane]->type() == TbModule::Tpx3Triple) {
    x = m_xTriple[scol];
    y = (row + 0.5) * Tb::PixelPitch;
    return true;
  }
  return false;
}

//============================================================================
// Calculate the pixel and inter-pixel position of a given local point
//============================================================================
bool TbGeometrySvc::pointToPixel(const double x, const double y,
                                 const unsigned int plane, unsigned int& scol,
                                 unsigned int& row) {

  if (x < 0. || y < 0.) return false;
  if (m_modules[plane]->type() == TbModule::Tpx3) {
    const double fcol = x / Tb::PixelPitch;
    const double frow = y / Tb::PixelPitch;
    scol = int(fcol);
    row = int(frow);
    if (scol > 255 || row > 255) return false;
    return true;
  } else if (m_modules[plane]->type() == TbModule::Tpx3Triple) {
    const double chipSize = Tb::NCols * Tb::PixelPitch;
    const double interChipDistance = 2 * Tb::PixelPitch;
    double x0 = 0.;
    for (unsigned int i = 0; i < 3; ++i) {
      const double xl = x - x0;
      if (xl < chipSize + 0.5 * interChipDistance) {
        unsigned int col = 0;
        if (xl > 0.) {
          col = int(xl / Tb::PixelPitch);
          if (col >= Tb::NCols) col = Tb::NCols - 1;
        }
        scol = col + i * Tb::NCols;
        row = int(y / Tb::PixelPitch);
        if (row >= Tb::NRows) row = Tb::NRows - 1;
        return true;
      }
      x0 += chipSize + interChipDistance;
    }
  }
  return false;
}

//============================================================================
// Calculate intercept of track with detector plane
//============================================================================
Gaudi::XYZPoint TbGeometrySvc::intercept(const LHCb::TbTrack* track,
                                         const unsigned int i) {
  auto state = track->firstState();
  if (track->fitStatus() == LHCb::TbTrack::Kalman) {
    auto nodes = track->nodes();
    const double zPlane = m_modules[i]->centre().z();
    if (zPlane < nodes.front().z()) {
      state = nodes.front().state();
    } else if (zPlane > nodes.back().z()) {
      state = nodes.back().state();
    } else {
      for (auto it = nodes.cbegin(), end = nodes.cend(); it != end; ++it) {
        if (zPlane < (*it).z()) {
          const auto p = std::prev(it);
          state = (*p).state();
          break;
        }
      }
    }
  }
  return intercept(state.position(), state.slopes(), i);
}

//============================================================================
// Import geometry conditions from alignment file
//============================================================================
bool TbGeometrySvc::readConditions(const std::string& filename,
                                   std::vector<TbModule*>& modules) {
  if (filename == "") return false;
  TbCondFile f(filename);
  if (!f.is_open()) return false;
  while (!f.eof()) {
    std::string line = "";
    if (!f.getLine(line)) continue;
    std::string id = "";
    double dx(0.), dy(0.), dz(0.);
    double rx(0.), ry(0.), rz(0.);
    f.split(line, ' ', id, dx, dy, dz, rx, ry, rz);

    TbModule* m = new TbModule();
    std::string c1, c2, c3;
    f.split(id, ',', c1, c2, c3);
    m->setId(id);
    m->setAlignment(dx, dy, dz, rx, ry, rz, 0., 0., 0., 0., 0., 0.);
    m_deviceIndex[c1] = m_nDevices++;
    if (c1 != "" && c2 != "" && c3 != "") {
      m->setType(TbModule::Tpx3Triple);
      m->addChip(c1);
      m->addChip(c2);
      m->addChip(c3);
      m_deviceIndex[c2] = m_nDevices++;
      m_deviceIndex[c3] = m_nDevices++;
    } else {
      m->setType(TbModule::Tpx3);
      m->addChip(id);
    }
    modules.push_back(m);
  }
  f.close();
  return true;
}

//============================================================================
// Return the module for a given chip name
//============================================================================
TbModule* TbGeometrySvc::module(const std::string& id) {

  if (m_moduleIndex.count(id) < 1) {
    error() << "Module " << id << " not found" << endmsg;
    return nullptr;
  }

  return m_modules[m_moduleIndex[id]];
}

//============================================================================
// Print the geometry conditions for each module
//============================================================================
void TbGeometrySvc::printAlignment(const std::vector<TbModule*>& modules) {

  for (auto it = modules.begin(), end = modules.end(); it != end; ++it) {
    const std::string name = (*it)->id();
    const unsigned index = m_moduleIndex[name];
    // Print out the geometry conditions.
    info() << format("%2i", index) << format(" %-15s", name.c_str())
          << format(" %10.3f", (*it)->x()) << format(" %10.3f", (*it)->y())
          << format(" %10.3f", (*it)->z()) << format(" %10.3f", (*it)->rotX())
          << format(" %10.3f", (*it)->rotY())
          << format(" %10.3f", (*it)->rotZ()) << endmsg;
  }
}
