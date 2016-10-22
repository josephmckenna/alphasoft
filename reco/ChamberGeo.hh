#ifndef ChamberGeo_h
#define ChamberGeo_h
#include <vector>

#include <TEllipse.h>
#include <TLine.h>
#include <TGraph.h>
#include "TPCBase.hh"

using std::vector;

class ChamberGeo {
public:
    ChamberGeo() : x0(0), y0(0), x1(0), y1(0){};
    void SetWindow(double xmin, double ymin, double xmax, double ymax){
        x0 = xmin; y0 = ymin; x1 = xmax; y1 = ymax;
    };
    void SetPhi0(double phi_0){ // Rotation of TPC in degrees with respect to nominal position
        phi0 = phi_0*M_PI/180.;
    }

    vector<TEllipse*> GetCathode2D();
    vector<TEllipse*> GetPadCircle2D();
    TGraph *GetAnodeWires(int i0 = 0, int i1 = TPCBase::NanodeWires);
    TGraph *GetFieldWires(int i0 = 0, int i1 = TPCBase::NfieldWires);
    vector<TLine> GetPadSectors();
private:
    vector<double> GetPhiRange(double R);
    double x0, y0, x1, y1;
    double phi0 = 0.;
    const double mm = 0.1;
};

#endif
