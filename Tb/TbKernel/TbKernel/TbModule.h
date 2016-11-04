#ifndef TB_MODULE_H
#define TB_MODULE_H 1

#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/Transform3DTypes.h"
#include "Math/Translation3D.h"
#include "Math/RotationZYX.h"
#include "TbKernel/TbConstants.h"
#include "TMatrixD.h"
#include "TMatrixDEigen.h"
#include "TVectorD.h"

struct TbChip {
  TbChip(const std::string& _id, const unsigned int& _col,
         const unsigned int& _num)
      : col(_col), num(_num), id(_id) {};

  unsigned int col;
  unsigned int num;
  std::string id;
};

class TbModule {

 public:
  /// Module types
  enum Type {
    Tpx3,        ///< Tpx3 single chip assembly
    Tpx3Triple,  ///< Tpx3 3x1 tile
    UT           ///< UT sensor
  };

  /// Constructor
  TbModule() : m_chips(), m_centre(), m_normal() {}

  /// Destructor
  virtual ~TbModule() {
    if (m_transform) delete m_transform;
    if (m_inverse) delete m_inverse;
  }

  /// Return the module type.
  TbModule::Type type() const { return m_type; }
  /// Set the module type.
  void setType(const TbModule::Type& type) { m_type = type; }

  /// Return the module ID.
  std::string id() const { return m_id; }
  /// Set the module ID.
  void setId(const std::string& id) { m_id = id; }
  /// Return the chip ID (for modules with multiple chips).
  std::string id(const unsigned int index) { return m_chips[index].id; }

  /// Set the nominal position and orientation and the misalignment.
  void setAlignment(const double x, const double y, const double z,
                    const double rx, const double ry, const double rz,
                    const double dx, const double dy, const double dz,
                    const double drx, const double dry, const double drz) {
    m_x = x;
    m_y = y;
    m_z = z;
    m_rx = rx;
    m_ry = ry;
    m_rz = rz;
    m_dx = dx;
    m_dy = dy;
    m_dz = dz;
    m_drx = drx;
    m_dry = dry;
    m_drz = drz;
    setTransform();
  }
  /// Set the misalignment.
  void setAlignment(const double dx, const double dy, const double dz,
                    const double drx, const double dry, const double drz) {
    m_dx = dx;
    m_dy = dy;
    m_dz = dz;
    m_drx = drx;
    m_dry = dry;
    m_drz = drz;
    setTransform();
  }
  /// Return the centre position of the module.
  const Gaudi::XYZPoint& centre() { return m_centre; }
  /// Return the normal vector to the module plane.
  const Gaudi::XYZVector& normal() { return m_normal; }
  /// Return the transformation matrix from local to global coordinates.
  Gaudi::Transform3D transform() { return *m_transform; }
  /// Return the transformation matrix from global to local coordinates.
  Gaudi::Transform3D inverse() { return *m_inverse; }

  unsigned int nChips() const { return m_chips.size(); }

  void addChip(const std::string& id) {
    m_chips.push_back(TbChip(id, nChips() * Tb::NCols, nChips()));
  }
  unsigned int cols() const {
    return m_type == TbModule::Tpx3Triple ? 3 * Tb::NCols : Tb::NCols;
  }

  const std::vector<TbChip>& chips() const { return m_chips; }

  double x() const { return m_x; }
  double y() const { return m_y; }
  double z() const { return m_z; }
  double rotX() const { return m_rx; }
  double rotY() const { return m_ry; }
  double rotZ() const { return m_rz; }
  double dX() const { return m_dx; }
  double dY() const { return m_dy; }
  double dZ() const { return m_dz; }
  double dRotX() const { return m_drx; }
  double dRotY() const { return m_dry; }
  double dRotZ() const { return m_drz; }

  void setTransform() {
    ROOT::Math::Translation3D t(m_x + m_dx, m_y + m_dy, m_z + m_dz);
    ROOT::Math::RotationZYX r(m_rz + m_drz, m_ry + m_dry, m_rx + m_drx);
    if (m_transform) delete m_transform;
    if (m_inverse) delete m_inverse;

    m_transform = new Gaudi::Transform3D(r, t);
    m_inverse = new Gaudi::Transform3D(m_transform->Inverse());
    // Calculate the coordinates of the module centre in the global frame.
    m_centre = transform() * Gaudi::XYZPoint(0., 0., 0.);
    // Calculate the normal vector in the global frame.
    Gaudi::XYZPoint p = transform() * Gaudi::XYZPoint(0., 0., 1.);
    m_normal = p - m_centre;
  }

 private:
  /// Module type
  TbModule::Type m_type = TbModule::Tpx3;
  /// Chips constituting the module
  std::vector<TbChip> m_chips;
  /// Nominal positions and rotations
  double m_x = 0.;
  double m_y = 0.;
  double m_z = 0.;
  double m_rx = 0.;
  double m_ry = 0.;
  double m_rz = 0.;
  /// Alignment
  double m_dx = 0.;
  double m_dy = 0.;
  double m_dz = 0.;
  double m_drx = 0.;
  double m_dry = 0.;
  double m_drz = 0.;
  /// Module identifier
  std::string m_id = "";
  /// Centre position
  Gaudi::XYZPoint m_centre;
  /// Normal vector to the module plane
  Gaudi::XYZVector m_normal;

  /// Transformation matrix from local to global coordinates.
  Gaudi::Transform3D* m_transform = NULL;
  /// Transformation matrix from global to local coordinates.
  Gaudi::Transform3D* m_inverse = NULL;

  /// Calculate the transformation matrix and its inverse.
};

#endif
