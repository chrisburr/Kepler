#ifndef TB_GEOMFUNC
#define TB_GEOMFUNC 1

namespace Tb {
/*
inline static void getTaitBryanAngles(const ROOT::Math::Quaternion& Q, double& alpha,
                               double& beta, double& gamma) {

  double a, b, c, d;
  Q.GetComponents(a, b, c, d);
  alpha = atan2(-2 * (c * d - a * b), a * a - b * b - c * c + d * d);
  beta = atan2(
      -2 * (b * d + c * a),
      sqrt(4 * (a * b - c * d) * (a * b - c * d) +
           (a * a - b * b - c * c + d * d) * (a * a - b * b - c * c + d * d)));
  gamma = atan2(-2 * (b * c - d * a), a * a + b * b - c * c - d * d);
}

inline static ROOT::Math::Quaternion getQuaternionRepresentation(
    const ROOT::Math::Rotation3D& rot) {

  double xx, xy, xz, yx, yy, yz, zx, zy, zz;
  rot.GetComponents(xx, yx, zx, xy, yy, zy, xz, yz, zz);

  const double a = 0.5 * sqrt(1 + xx + yy + zz);
  const double b = (0.25 / a) * (yz - zy);
  const double c = (0.25 / a) * (zx - xz);
  const double d = (0.25 / a) * (xy - yx);

  std::cout << "============= TAIT-BRYAN REPRESENTATION ===============" <<
  std::endl;
  std::cout << xx << "  " << yx << "  " << zx << std::endl;
  std::cout << xy << "  " << yy << "  " << zy << std::endl;
  std::cout << xz << "  " << yz << "  " << zz << std::endl;
  std::cout << "=======================================================" <<
  std::endl;
  std::cout << a << ", " << b << ", " << c << ", " << d << std::endl;

  std::cout << "============= HAMILTONIAN REPRESENTATION ==============" <<
  std::endl;
  std::cout << "| " << a*a + b*b - c*c - d*d << "  " << 2*(b*c - a*d) << "   "
  << 2*(b*d + a*c) << " |" << std::endl;
  std::cout << "| " << 2*(b*c + d*a) << "  " << 1 - 2*(b*b + d*d ) << "   " <<
  2*(c*d - a*b ) << " |" << std::endl;
  std::cout << "| " << 2*(b*d - c*a) << "  " << 2*(a*b + c*d ) << "   " << a*a -
  b*b - c*c + d*d << " |" << std::endl;
  std::cout << "=======================================================" <<
  std::endl;

  ROOT::Math::Quaternion Q(a, b, c, d);
  return Q;
}
*/
static void SmallRotation(const double& x0, const double& dx0, const double& y0,
                          const double& dy0, const double& dz0, double& xr,
                          double& yr, double& zr) {
  xr = dx0 + tan(y0) * (dz0 * cos(x0) + dy0 * cos(x0));
  yr = dy0 * cos(x0) + dz0 * sin(x0);
  zr = (-cos(x0) * dz0 + sin(x0) * dy0) / cos(y0);
}
}

#endif
