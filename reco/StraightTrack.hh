#ifndef STRAIGHTTRACK_H
#define STRAIGHTTRACK_H
class StraightTrack{
public:
    StraightTrack(double a0, double a1, double phi_rot = 0);
    StraightTrack(double x0, double y0, double x1, double y1, double phi_rot = 0);
    double GetR(int anode);
    double GetD(){
        return d;
    };
    double phi0, phi1;
private:
    double d, phiT, phiRot;
};
#endif
