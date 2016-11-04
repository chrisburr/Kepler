#ifndef TBGEOMETRYSVC_H
#define TBGEOMETRYSVC_H

#include <map>

// Gaudi
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/Transform3DTypes.h"

// Local
#include "TbKernel/ITbGeometrySvc.h"
#include "TbKernel/TbModule.h"

template <class TYPE>
class SvcFactory;

/** @class TbGeometrySvc TbGeometrySvc.h
 * Implementation of the testbeam geometry service.
 *
 */

class TbGeometrySvc : public extends1<Service, ITbGeometrySvc> {

 public:
  /// Constructor
  TbGeometrySvc(const std::string& name, ISvcLocator* svc);
  /// Destructor
  virtual ~TbGeometrySvc();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  /// Transform a point in the local frame of a plane to global coordinates
  virtual Gaudi::XYZPoint localToGlobal(const Gaudi::XYZPoint& p,
                                        const unsigned int i) {
    return m_modules[i]->transform() * p;
  }
  /// Transform a point in the global frame to the local frame of a plane
  virtual Gaudi::XYZPoint globalToLocal(const Gaudi::XYZPoint& p,
                                        const unsigned int i) {
    return m_modules[i]->inverse() * p;
  }

  /// Calculate the local x, y coordinates of a given pixel
  virtual bool pixelToPoint(const unsigned int scol, const unsigned int row,
                            const unsigned int plane, double& x, double& y);
  /// Calculate the pixel and inter-pixel position of a given local point
  virtual bool pointToPixel(const double x, const double y,
                            const unsigned int plane, unsigned int& scol,
                            unsigned int& row);

  /// Calculate the intercept of a track with a telescope plane
  virtual Gaudi::XYZPoint intercept(const LHCb::TbTrack* t,
                                    const unsigned int i);

  /// Return a pointer to the module object for a given plane index
  virtual TbModule* module(const unsigned int i) { return m_modules[i]; }
  /// Return a pointer to the module object of a given plane identifier
  virtual TbModule* module(const std::string& det);
  /// Return the list of modules
  virtual const std::vector<TbModule*>& modules() { return m_modules; }

  virtual unsigned int deviceIndex(const std::string& det) {
    auto p = m_deviceIndex.find(det);
    return p != m_deviceIndex.end() ? p->second : 999;
  }
  virtual unsigned int plane(const std::string& det) {
    auto p = m_planes.find(det);
    return p != m_planes.end() ? p->second : 999;
  }

  virtual unsigned int nDevices() const { return m_nDevices; }

  bool readConditions(const std::string& filename,
                      std::vector<TbModule*>& modules);
  virtual void printAlignment(const std::vector<TbModule*>& modules);

  virtual void setModule(unsigned int i, TbModule* module) {
    m_modules[i] = module;
  }

 private:
  /// Allow SvcFactory to instantiate the service.
  friend class SvcFactory<TbGeometrySvc>;
  /// Transforms for each plane
  std::vector<TbModule*> m_modules;
  /// Map of plane names and their indices in the list
  std::map<std::string, unsigned int> m_moduleIndex;
  /// Map of device ids to their chip names
  std::map<std::string, unsigned int> m_deviceIndex;
  /// Map of device names to their planes indices
  std::map<std::string, unsigned int> m_planes;
  unsigned int m_nDevices;

  /// Cached x-coordinates of pixels on sensor tiles.
  std::vector<double> m_xTriple;

  /// Calculate the intercept of a straight line with a telescope plane
  Gaudi::XYZPoint intercept(const Gaudi::XYZPoint& p, const Gaudi::XYZVector& t,
                            const unsigned int i) {
    const Gaudi::XYZVector n = m_modules[i]->normal();
    const double s = n.Dot(m_modules[i]->centre() - p) / n.Dot(t);
    return p + s * t;
  }

};

#endif
