#pragma once

// Tb/TbEvent
#include "Event/TbTrack.h"

// Local
#include "TbAlignmentBase.h"

/** @class TbMillepede TbMillepede.h
 *
 *  Implementation of the Millepede algorithm.
 *
 *  @author Christoph Hombach
 *  @date   2012-06-19
 */

class TbMillepede : public TbAlignmentBase {
 public:
  /// Constructor
  TbMillepede(const std::string& type, const std::string& name,
              const IInterface* parent);
  /// Destructor
  virtual ~TbMillepede();

  virtual StatusCode initialize();

  void align(std::vector<TbAlignmentTrack*>& alignmentTracks);

  virtual void updateGeometry();

 private:
  struct Equation {
    double rmeas;
    double weight;
    std::vector<int> indG;
    std::vector<double> derG;
    std::vector<int> indL;
    std::vector<double> derL;
    std::vector<int> indNL;
    std::vector<double> derNL;
    std::vector<double> slopes;
  };
  struct Constraint {
    /// Right-hand side (Lagrange multiplier)
    double rhs;
    /// Coefficients
    std::vector<double> coefficients;
  };

  /// (Re-)initialise matrices and vectors.
  bool reset(const unsigned int nPlanes, const double startfact);
  /// Setup the constraint equations.
  void setConstraints(const unsigned int nPlanes);
  /// Define a single constraint equation
  void addConstraint(const std::vector<double>& dercs, const double rhs);
  /// Add the equations for one track and do the local fit.
  bool putTrack(LHCb::TbTrack* track, const unsigned int nPlanes);
  /// Store the parameters for one measurement.
  void addEquation(std::vector<Equation>& equations,
                   const std::vector<double>& derlc,
                   const std::vector<double>& dergb,
                   const std::vector<double>& dernl,
                   const std::vector<int>& dernli,
                   const std::vector<double>& dernls, const double rmeas,
                   const double sigma);
  /// Perform local parameters fit using the equations for one track.
  bool fitTrack(const std::vector<Equation>& equations,
                std::vector<double>& trackParams, const bool singlefit,
                const unsigned int iteration);
  /// Perform global parameters fit.
  bool fitGlobal();
  /// Print the results of the global parameters fit.
  bool printResults();

  /// Matrix inversion and solution for global fit.
  int invertMatrix(std::vector<std::vector<double> >& v, std::vector<double>& b,
                   const int n);
  /// Matrix inversion and solution for local fit.
  int invertMatrixLocal(std::vector<std::vector<double> >& v,
                        std::vector<double>& b, const int n);

  /// Return the limit in chi2 / ndof for n sigmas.
  double chi2Limit(const int n, const int nd) const;
  /// Multiply matrix and vector
  bool multiplyAX(const std::vector<std::vector<double> >& a,
                  const std::vector<double>& x, std::vector<double>& y,
                  const unsigned int n, const unsigned int m);
  /// Multiply matrices
  bool multiplyAVAt(const std::vector<std::vector<double> >& v,
                    const std::vector<std::vector<double> >& a,
                    std::vector<std::vector<double> >& w, const unsigned int n,
                    const unsigned int m);

  /// Number of global derivatives
  unsigned int m_nagb;
  /// Number of local derivatives
  unsigned int m_nalc;

  /// Equations for each track
  std::vector<std::vector<Equation> > m_equations;
  /// Constraint equations
  std::vector<Constraint> m_constraints;

  ///  Flag for each global parameter whether it is fixed or not.
  std::vector<bool> m_fixed;
  /// Sigmas for each global parameter.
  std::vector<double> m_psigm;

  std::vector<std::vector<double> > m_cgmat;
  std::vector<std::vector<double> > m_corrm;
  std::vector<std::vector<double> > m_clcmat;

  std::vector<double> m_bgvec;
  std::vector<double> m_corrv;
  std::vector<double> m_diag;

  /// Difference in misalignment parameters with respect to initial values.
  std::vector<double> m_dparm;

  /// Mapping of internal numbering to geometry service planes.
  std::vector<unsigned int> m_millePlanes;

  bool m_debug;
  /// Flag to switch on/off iterations in the global fit.
  bool m_iterate;
  /// Residual cut after the first iteration.
  double m_rescut;
  /// Residual cut in the first iteration.
  double m_rescut_init;
  /// Factor in chisquare / ndof cut.
  double m_cfactr;
  /// Reference value for chisquare / ndof cut factor.
  double m_cfactref;
  // Number of standard deviations for chisquare / ndof cut.
  int m_nstdev;
  /// Number of "full" iterations (with geometry updates).
  unsigned int m_nIterations;
  /// Sigmas for each degree of freedom
  std::vector<double> m_sigmas;
  /// Planes to be kept fixed
  std::vector<unsigned int> m_fixedPlanes;
  /// Flag to fix all degrees of freedom or only the translations.
  bool m_fix_all;
};
