#ifndef TPCBASE_H
#define TPCBASE_H

#include <iostream>
#include <vector>
#include <string>

#include <cmath>

static const double unknown = -1e99;

class TPCBase{
public:
    static void GetAnodePosition(int i, double &x_R, double &y_phi, bool polar=false, double phi0 = 0);
    static void GetWirePosition(int i, double &x_R, double &y_phi, bool polar=false, double phi0 = 0);
    static void GetPadPosition(int i, double &z, double &phi);
    static int FindPad(const double z, const double phi = 0);
    static unsigned int FindAnode(const double phi, double phi0 = 0);
    static double GetAnodePitch(){
        return 2.*M_PI / double(NanodeWires);
    };



    static constexpr double CathodeRadius = 10.925f;  // correct value, use when ready.
  //    static constexpr double CathodeRadius = 10.3f;  // FIXME: legacy value, should not be used for real calculations, but stays in for comparability with previous simulations
    static constexpr double FieldWiresR = 17.4f; // cm
    static constexpr double AnodeWiresR = 18.2f; // cm
    static constexpr double ROradius = 19.f; // cm
    static const int NanodeWires = 256;
    static const int NfieldWires = 256;
    static constexpr double HalfWidthZ = 115.2f;
    static constexpr double diamFieldWires = 0.0075f;
    static constexpr double tensionFieldWires = 300.f; // g
    static constexpr double diamAnodeWires = 0.0030f;  // correct value, use when ready.
  //    static constexpr double diamAnodeWires = 0.0025f;  // FIXME: legacy value, should not be used for real calculations, but stays in for comparability with previous simulations
    static constexpr double tensionAnodeWires = 60.f; // g
    static constexpr double AnodeWiresDensity = 19.25f; // g/cm^3 tungsten
    int trap_radius = 5; // default value = 5 wire radii
    static constexpr double PadSideZ = 0.4f; //cm
    static constexpr double PadWidthPhi = 2.0f; // in units of pi
    //    int npads = int(2.*HalfWidthZ/PadSideZ);
    static const int npads = 576;
    static const int npadsec = 32;
    static constexpr double TrapR = 2.2275f; // cm
};

#endif
