#ifndef a2mc_muonGEN_H
#define a2mc_muonGEN_H

#include <array>
#include <iostream>
#include <TRandom.h>
#include <TMath.h>

class a2mcMuonGen : public TObject
{
private:
    void Init();
    double mYOffset;
    double mZOffset;
    double mCylRadius;
    double mCylHeight;
    double mGenMomentum;
    double mTheta;
    double mPhi;
    std::array<double, 3> mGenPoint;
    std::vector<double> p_min;
    //double mzWorld;
    double mxSky;
    double mySky;
    double mzSky;
    double sphere_radius;
    int iSky;
    bool   mDoHorGen;
    bool   mDoSphGen;
    bool   mDoCylGen;

public:
    a2mcMuonGen();
    virtual ~a2mcMuonGen();
    virtual void Generate();


// Setters //////////////////////////////////////////////
    void SetCylinderRadius(double);
    void SetCylinderHeight(double);
    void SetCylinderRadiusAndHeight(double, double);
    void SetZOffset(double);
    void SetYOffset(double);
    void SetSkyXYZ(double, double, double, int);
    void SetSphRadius(double);
    void SetHorizontalGeneration(bool);
    void SetSphericalGeneration(bool);
    void SetCylindricalGeneration(bool);
    
// Getters ////////////////////////////////////////////// 
    double GetCylinderRadius() {return mCylRadius;};
    double GetCylinderHeight() {return mCylHeight;};
    void GetCylinderRadiusAndHeight(double &radius, double &height) {
        radius = mCylRadius;
        height = mCylHeight;
    };
    double GetZOffset() {return mZOffset;};

    void GetGenPoint(std::array<double, 3>& genPoint) {
        genPoint = mGenPoint;
    };

    void GetLineCircleIntercepts(double &x0, double &y0, double &x1, double &y1) {
        double m = tan(mPhi);
        double q = mGenPoint[1]-m*mGenPoint[0];

        double a = 1 + pow(m,2);
        double b = 2*m*q;
        double c = pow(q,2) - pow(mCylRadius,2);

        if (pow(b,2)-4*a*c < 0) return;
    
        x0 = (-b + sqrt(pow(b,2)-4*a*c))/(2*a);
//    std::cout << "x = " << x0 << std::endl;
	
        y0 = tan(mPhi)*x0 + q;
        x1 = (-b - sqrt(pow(b,2)-4*a*c))/(2*a);
        y1 = tan(mPhi)*x1 + q;
    };

    double GetGenMomentum() {return mGenMomentum;};

    double GetTheta() {return mTheta;};
    double GetPhi() {return mPhi;};
  ////////////////////////////////////////////////////////

};
#endif


