#include "TVector3.h"

class ScintPadProto
{
private:
  TVector3 r0;
  TVector3 U;

  double Zlen;
  double Xlen;

  double topZshift;
  double topXshift;
  double botZshift;
  double botXshift;
  
  double topZmin;
  double topZmax;
  double topXmin;
  double topXmax;
  
  double botZmin;
  double botZmax;
  double botXmin;
  double botXmax;

  double topYpos;
  double botYpos;

  TVector3 n0;
  TVector3 p1;
  TVector3 p2;

  void Setup()
  {
    topZmin = -0.5*Zlen + topZshift, topZmax = 0.5*Zlen + topZshift;
    topXmin = -0.5*Xlen + topXshift, topXmax = 0.5*Xlen + topXshift;
    botZmin = -0.5*Zlen + botZshift, botZmax = 0.5*Zlen + botZshift;
    botXmin = -0.5*Xlen + botXshift, botXmax = 0.5*Xlen + botXshift;

    p1.SetXYZ( (topXmin + topXmax)*0.5, topYpos, (topZmin + topZmax)*0.5 );
    p2.SetXYZ( (botXmin + botXmax)*0.5, botYpos, (botZmin + botZmax)*0.5 );
  }

public:
  ScintPadProto( TVector3 point, TVector3 direction ): r0(point), U(direction),
						       Zlen(25.), Xlen(30.), // cm
						       topZshift(0.), topXshift(0.),
						       botZshift(0.), botXshift(0.),
						       topYpos(30.), botYpos(-27.),
						       n0(0.,1.,0.)
  {
    Setup();
  }

  ScintPadProto( TVector3 point, TVector3 direction, 
		 double tzs, double txs,
		 double bzs, double bxs ): r0(point), U(direction),
					   Zlen(25.), Xlen(30.), // cm
					   topZshift( tzs ), topXshift( txs ),
					   botZshift( bzs ), botXshift( bxs ),
					   topYpos(30.), botYpos(-27.),
					   n0(0.,1.,0.)
  {
    Setup();
  }
  
  ScintPadProto( TVector3 point, TVector3 direction, 
		 double typ, double byp): r0(point), U(direction),
					  Zlen(25.), Xlen(30.), // cm
					  topZshift(0.), topXshift(0.),
					  botZshift(0.), botXshift(0.),
					  topYpos( typ ), botYpos( byp ),
					  n0(0.,1.,0.)
  {
    Setup();
  }
  
  ScintPadProto( TVector3 point, TVector3 direction, 
		 double tzs, double txs,
		 double bzs, double bxs,
		 double typ, double byp): r0(point), U(direction),
					  Zlen(25.), Xlen(30.), // cm
					  topZshift( tzs ), topXshift( txs ),
					  botZshift( bzs ), botXshift( bxs ),
					  topYpos( typ ), botYpos( byp ),
					  n0(0.,1.,0.)
  {
    Setup();
  }


  bool Intersection()
  {
    if( U.Dot( n0 ) == 0.)
      {
	if( r0.X() > topXmin && r0.X() < topXmax && 
	    r0.Z() > topZmin && r0.Z() < topZmax &&
	    r0.X() > botXmin && r0.X() < botXmax && 
	    r0.Z() > botZmin && r0.Z() < botZmax )
	  return true;
	else
	  return false;
      }
	
    
    double t1 = (p1 - r0).Dot( n0 ) / U.Dot( n0 );
    TVector3 C1 = U * t1 + r0;
    bool i1 = false;
    if( C1.X() > topXmin && C1.X() < topXmax && 
	C1.Z() > topZmin && C1.Z() < topZmax )
      i1 = true;
    else
      return i1;

    double t2 = (p2 - r0).Dot( n0 ) / U.Dot( n0 );
    TVector3 C2 = U * t2 + r0;
    bool i2 = false;
    if( C2.X() > botXmin && C2.X() < botXmax && 
	C2.Z() > botZmin && C2.Z() < botZmax )
      i2 = true;
    
    return i1 && i2;
  }
};
