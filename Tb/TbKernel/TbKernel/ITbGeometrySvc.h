#ifndef TBKERNEL_ITBGEOMETRYSVC_H
#define TBKERNEL_ITBGEOMETRYSVC_H 1

#include "GaudiKernel/IService.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"

/** @class ITbGeometrySvc ITbGeometrySvc.h
 *
 * Interface for the testbeam geometry service which holds
 * the geometric transformations between local and global coordinates
 * of the individual planes.
 *
 */

class TbModule;
namespace LHCb {
class TbTrack;
}

class GAUDI_API ITbGeometrySvc : virtual public IService {

 public:
  /// InterfaceID
  DeclareInterfaceID(ITbGeometrySvc, 1, 0);

  /// Transform a point in the local frame of a plane to global coordinates
  virtual Gaudi::XYZPoint localToGlobal(const Gaudi::XYZPoint& p,
                                        const unsigned int i) = 0;
  /// Transform a point in the global frame to the local frame of a plane
  virtual Gaudi::XYZPoint globalToLocal(const Gaudi::XYZPoint& p,
                                        const unsigned int i) = 0;

  /// Calculate the local x, y coordinates of a given pixel
  virtual bool pixelToPoint(const unsigned int scol, const unsigned int row,
                            const unsigned int plane, double& x,
                            double& y) = 0;
  /// Calculate the pixel and inter-pixel position of a given local point
  virtual bool pointToPixel(const double x, const double y,
                            const unsigned int plane, unsigned int& scol,
                            unsigned int& row) = 0;

  /// Calculate the intercept of a track with a telescope plane
  virtual Gaudi::XYZPoint intercept(const LHCb::TbTrack* t,
                                    const unsigned int i) = 0;

  virtual Gaudi::XYZPoint intercept(const Gaudi::XYZPoint& p,
                                    const Gaudi::XYZVector& t,
                                    const unsigned int i) = 0;

  /// Return a pointer to the module object for a given plane index
  virtual TbModule* module(const unsigned int i) = 0;
  /// Return a pointer to the module object of a given plane identifier
  virtual TbModule* module(const std::string& det) = 0;
  /// Return the list of modules
  virtual const std::vector<TbModule*>& modules() = 0;

  virtual unsigned int nDevices() const = 0;

  virtual unsigned int deviceIndex(const std::string& det) = 0;

  virtual unsigned int plane(const std::string& det) = 0;

  virtual bool readConditions(const std::string& file,
                              std::vector<TbModule*>& modules) = 0;

  virtual void printAlignment(const std::vector<TbModule*>& modules) = 0;

  virtual void setModule(unsigned int i, TbModule* module) = 0;
};

#endif
