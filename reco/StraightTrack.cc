#include "StraightTrack.hh"
#include "TPCBase.hh"

#include <iostream>
#include <cmath>

using std::cout;
using std::endl;

StraightTrack::StraightTrack(double a0, double a1, double phi_rot){
    double R;
    phiRot = phi_rot;
    int a00 = a0;
    int a10 = a1;
    int a01 = a00 + 1;
    int a11 = a10 + 1;

    double ratio0 = a0 - a00;
    double ratio1 = a1 - a10;

    double phiA, phiB;
    TPCBase::GetAnodePosition(a00, R, phiA, true, phi_rot);
    TPCBase::GetAnodePosition(a01, R, phiB, true, phi_rot);

    phi0 = ratio0*phiB + (1-ratio0)*phiA;

    TPCBase::GetAnodePosition(a10, R, phiA, true, phi_rot);
    TPCBase::GetAnodePosition(a11, R, phiB, true, phi_rot);

    phi1 = ratio1*phiB + (1-ratio1)*phiA;
    //    cout << "anode phi positions: " << phi0*180./TMath::Pi() << ", " << phi1*180./TMath::Pi() << endl;
    R *= 10.;
    d = R*cos(0.5*(phi1-phi0));
    phiT = (phi0+phi1)/2.;
    if(d < 0){
        phiT -= M_PI;
        d *= -1.;
    }
    //    cout << "d = " << d << ", phiT = " << phiT*180./TMath::Pi() << endl;
}

StraightTrack::StraightTrack(double x0, double y0, double x1, double y1, double phi_rot){
    double phi0 = atan(y0/x0);
    if(x0 < 0) phi0 += M_PI;
    if(x0 > 0 && y0 < 0) phi0 += 2.*M_PI;
    double phi1 = atan(y1/x1);
    if(x1 < 0) phi1 += M_PI;
    if(x1 > 0 && y1 < 0) phi1 += 2.*M_PI;
    double r0 = sqrt(x0*x0+y0*y0);
    double r1 = sqrt(x1*x1+y1*y1);
    double R = sqrt(r0*r0+r1*r1-2*r0*r1*cos(phi1-phi0));
    double s = sin(phi1-phi0)>0 ? 1 : -1;
    d = s*r0*r1*sin(phi1-phi0)/R;
    phiT = acos(s*(r1*sin(phi1)-r0*sin(phi0))/R);
    double phiT2 = asin(-s*(r1*cos(phi1)-r0*cos(phi0))/R);
    if(abs(phiT2+phiT) < 1e-10) phiT = phiT2;
    if(phiT != phiT)
        cout << "XXXXXXXX " << x0 << '\t' << x1 << '\t' << r0 << '\t' << r1 << '\t' << R << '\t' << phi0 << '\t' << phi1 << endl;
    // cout << "phiT  = " << phiT*180./M_PI << " d = " << d << endl;
}

double StraightTrack::GetR(int anode){
    double R, phi;
    TPCBase::GetAnodePosition(anode, R, phi, true, phiRot);
    return d/cos(phi-phiT);
}
