#ifndef G_COMPONENT_B_MAP
#define G_COMPONENT_B_MAP

#include <cmath>
#include <string>
#include <iostream>

#include "ComponentBase.hh"
#include "ComponentAnalyticField.hh"
#include "FundamentalConstants.hh"

#include "MagneticFieldMap.hh"

using std::vector;
using std::string;

namespace Garfield {
    class ComponentBmap : public ComponentAnalyticField {
    public:
        ComponentBmap(bool pol=false);   // Construct field in cylindrical or cartesian coordinates
        void MagneticField(double x, double y, double z,
                           double& bx, double& by, double& bz, int& status);
        bool ReadMagneticFieldMap(const string filename, float scale = 1);  // Read in CSV field map
        void SetMagneticField(double bx, double by, double bz){
            double blim = sqrt(bx*bx + by*by + bz*bz);
	    Bmap.SetBRange( blim, blim );
            ComponentAnalyticField::SetMagneticField(bx, by, bz);
        }

        bool SetSymmetries(string sym);  // string of symmetry letters, e.g. "xy" or "rz"
        void PrintSymmetries(){ Bmap.PrintSymmetries(); };
        bool CheckSymmetries();          // see if symmetries and field map match
        void GetMapBoundaries(double &x0, double &x1, double &y0, double &y1, double &z0, double &z1);
      int IsPolar(){ return Bmap.IsPolar(); }   // polar > 0 for polar coordinates, polar == 2 means rotational symmetry

      void GetBRange(double &b0, double &b1){ Bmap.GetBRange( b0, b1); };
    private:
      MagneticFieldMap Bmap;
    };
}

#endif
