#ifndef SpacePoints_h
#define SpacePoints_h

#include "TLookUpTable.hh"
#include "Signals.hh"
#include <iostream>
#include <TObjArray.h>

class SpacePoints{
public:
    class Point3D{
    public:
        Point3D(double rx, double phiy, double zz = unknown, bool polar = true);
        Point3D(double t, int anode, TLookUpTable &lookup, double phi0 = 0);
        void SetErrors(double erx = 0, double ephiy = 0, double ezz = 0, bool polar = true);
        void SetZ(double zz, double ezz = 0){ z = zz; ez = ezz; };
        double r, phi, z, x, y, er, ex, ephi, ey, ez, t;
        int anode;
    };

    SpacePoints(const Signals *sig=nullptr): lookup("arco2"){
        Reset(sig);
    };
    void Reset(const Signals *sig){
        signals = sig;
        points.clear();
    }
    bool SetGas(double CO2frac = 0.1, double p = 760, double T = 293.15);
    bool SetB(double b){
        B = b;
        return lookup.SetB(B);
    }
    void SetT0(double t){ // Set to negative for self-determined trigger time
        t0 = t;
    }
    void SetPhi0(double phi_0){ // Rotation of TPC in radians with respect to nominal position
        phi0 = phi_0;
    }
    vector<Point3D> & GetRPhiPoints(double thresh = 0, double tfudge = 0);
    vector<Point3D> & GetPoints(double anodeThres = 0, double padThres = 0, double tfudge = 0);

    TObjArray *GetSpacePoints(){
        return &tspacepoints;
    };

    TLookUpTable lookup;

private:
    double DeltaPhi_total(){ // degrees total Lorentz offset between start point at cathode and end point at anode wire
        if(B == 1) return 26.33;
        else if(B == 0) return 0;
        else {
            std::cerr << "Parameters for magnetic fields other than 0 T and 1 T unknown." << std::endl;
            return 0;
        }
    }
    double t0 = -1.;
    double phi0 = 0.;
    double B = 1.;
    const double rcut = 170;
    const double mm = 0.1;

    const Signals * signals;

    vector<Point3D> points;

    TObjArray tspacepoints;
};

#endif
