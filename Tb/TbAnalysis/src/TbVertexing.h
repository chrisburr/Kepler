#pragma once

// AIDA
#include "AIDA/IProfile1D.h"
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
// Tb/TbKernel
#include "TbKernel/TbAlgorithm.h"
#include "TbKernel/ITbTrackFit.h"
#include "Event/TbTrack.h"

#include "TVector3.h"
#include "TMatrixD.h"
#include "TVectorD.h"

class TbVertexing : public TbAlgorithm {
 public:
  /// Constructor
  TbVertexing(const std::string& name, ISvcLocator* pSvcLocator);
  /// Destructor
  virtual ~TbVertexing() {}
  virtual StatusCode finalize(); 
  virtual StatusCode initialize();  ///< Algorithm initialization
  virtual StatusCode execute();     ///< Algorithm execution

 private:
  double m_timeWindow;
  double m_maxDoca2;
  NTuple::Item<unsigned int> m_index;
  NTuple::Item<unsigned int> m_matched;
  NTuple::Item<unsigned int> m_common;
  NTuple::Array<double> m_vtxX;
  NTuple::Array<double> m_vtxY;
  NTuple::Array<double> m_vtxZ;
  ITbTrackFit* m_trackFit;

  std::string m_trackLocation;

  double doca2( const LHCb::TbTrack* track1, const LHCb::TbTrack* track2 ) const {
    const TVector3 r1( track1->firstState().x(), track1->firstState().y(), 0. );
    const TVector3 r2( track2->firstState().y(), track2->firstState().y(), 0. );
    const TVector3 t1( track1->firstState().tx(), track1->firstState().ty(), 1.);
    const TVector3 t2( track2->firstState().tx(), track2->firstState().ty(), 1.);
    
    const double z0 = - (t1 -t2).Dot( r1 - r2 ) / (t1-t2).Mag2();
    return (r1-r2).Mag2() + 2*z0*(t1-t2).Dot(r1-r2) + z0*z0*(t1-t2).Mag2();
  };

  /// checks if has > 3 vertices in the forward direction, with unmatched clusters /// 
  int matched( const LHCb::TbTrack* track1, const LHCb::TbTrack* track2 ) const {
    auto clusters1 = track1->clusters();
    auto clusters2 = track2->clusters();
    int nCommon = 0;
    int nMatched = 0;
    for( auto& cluster1 : clusters1 ){
      for( auto& cluster2 : clusters2 ){
        if( cluster1->plane() == cluster2->plane() ){
          if( cluster1 != cluster2 ) nMatched++;
          else nCommon++;
          break;
        }
      }
    }
    return nMatched;
  }

  TVectorD vertexPosition( const std::vector<const LHCb::TbTrack*>& tracks ) const {
    
    TMatrixD cov(3,3);
    TVectorD res(3);
    for( auto& track : tracks ){
      TVectorD t(3);
      t[0] = track->firstState().tx();
      t[1] = track->firstState().ty();
      t[2] = 1;
      const double norm = t.Norm2Sqr();
      TVectorD r(3);
      r[0] = track->firstState().x();
      r[1] = track->firstState().y();
      r[2] = 0;
      double ip = t[0]*r[0] + t[1]*r[1] + t[2]*r[2];

      res += r - (ip /norm ) * t;

      for( unsigned int i=0;i!=3;++i){  
        for( unsigned int j=0; j !=3 ;++j){
          if(i==j) cov[i][j] += 1 - t[i]*t[j] / norm;
          else cov[i][j] += - t[i]*t[j] / norm;
        }
      } 
    }
    TMatrixD inv = cov.Invert();
    return inv * res ; 
  }

};
