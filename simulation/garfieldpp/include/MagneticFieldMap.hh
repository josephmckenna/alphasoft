#ifndef _MAGNETIC_FIELD_MAP_
#define _MAGNETIC_FIELD_MAP_

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <set>

using std::vector;
using std::string;
using std::set;

class MagneticFieldMap
{
protected:
  vector<vector<vector<vector<double> > > > BfieldMap;
  double bmin, bmax;
  double dx, dy, dz;
  double xmin, xmax, ymin, ymax, zmin, zmax;
  int polar;
  set<char> symmetries;

  // Transformation between cartesian and polar coordinates
  void Cartesian2Polar(const double x0, const double y0, 
		       double& r, double& theta) 
  {
    
    if( x0 == 0. && y0 == 0. ) 
      {
	r = theta = 0.;
	return;
      }
    r = sqrt(x0 * x0 + y0 * y0);
    theta = 180. * atan2(y0, x0) / M_PI;
  }
  
  void Polar2Cartesian(const double r, const double theta, 
		       double& x0, double& y0) 
  {
    x0 = r * cos(M_PI * theta / 180.);
    y0 = r * sin(M_PI * theta / 180.);
  }

public:
  MagneticFieldMap();
  MagneticFieldMap(bool);

  void MagneticField(double x, double y, double z,
		     double& bx, double& by, double& bz, int& status);

  bool ReadMap(const string filename, float scale = 1);  // Read in CSV field map   

  bool SetSymmetries(string sym) // string of symmetry letters, e.g. "xy" or "rz"
  {
    symmetries.clear();
    bool OK(true);
    for(string::iterator it = sym.begin(); it != sym.end(); it++)
      {
	switch(*it)
	  {
	  case 'x':
	    break;
	  case 'y': 
	    if(polar)
	      {
		std::cerr << "Coordinate " << *it << " undefined in polar coordinates." << std::endl; 
		OK = false; 
		break;
	      }
	  case 'z': 
	    std::cout << "Mirror symmetry in " << *it << "-direction." << std::endl; 
	    symmetries.insert(*it); 
	    break;
	  case 'p':
	    break;
	  case 'r':
	    if(polar)
	      {
		std::cout << "Rotational symmetry around z-axis." << std::endl; 
		symmetries.insert('p'); 
		break;
	      } 
	    else
	      {
		std::cerr << "Cannot define rotational symmetry in cartesian coordinates. Did you forget to set 'polar' in the constructor?" << std::endl; 
		OK = false; 
		break;
	      }
	  default: 
	    std::cerr << "Undefined symmetry parameter " << *it << std::endl;
      }
    }
    if( OK && symmetries.size() && BfieldMap.size() ) 
      return CheckSymmetries();
    else 
      return OK;
  }

  inline void PrintSymmetries() const
  { 
    for( set<char>::iterator it = symmetries.begin(); it != symmetries.end(); it++ )
      std::cout << *it << std::endl;
  }

  bool CheckSymmetries() const  // see if symmetries and field map match
  {
    bool OK(true);
    bool nonzero(false);
    for(set<char>::iterator it = symmetries.begin(); it != symmetries.end(); it++)
      {
	switch(*it)
	  {
	  case 'x': 
	    OK &= (xmin >= 0); 
	    nonzero |= (xmin != 0);
	    break;
	  case 'y': 
	    OK &= (ymin >= 0); 
	    nonzero |= (ymin != 0); 
	    break;
	  case 'z':
	    OK &= (zmin >= 0); 
	    nonzero |= (zmin != 0); 
	    break;
	  }
      }

    if(!OK)
      std::cerr << "Requested symmetries not applicable (negative lower map boundary)" << std::endl;
    else if( nonzero )
      std::cerr << "Lower boundary for symmetric coordinate above zero. May produce problems if requesting values between xmin and -xmin." << std::endl;
    return OK;
  }

  inline void SetPolar(int pol) { polar=pol; }
  inline int IsPolar() const { return polar; }

  inline void GetMapBoundaries(double &x0, double &x1, 
			       double &y0, double &y1, 
			       double &z0, double &z1) const
  {
    x0 = xmin;
    x1 = xmax;
    y0 = ymin;
    y1 = ymax;
    z0 = zmin;
    z1 = zmax;
  }

  inline void GetBRange(double &b0, double &b1) const
  { 
    b0 = bmin; 
    b1 = bmax;
  }

  inline void SetBRange(double b0, double b1)
  { 
    bmin = b0; 
    bmax = b1;
  }
 
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
