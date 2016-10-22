#include "SpacePoints.hh"

#include "TPCBase.hh"
#include "TLookUpTable.hh"
#include "TSpacePoint.hh"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::set;
using std::ofstream;

bool SpacePoints::SetGas(double CO2frac, double p, double T){
    std::cout << "SpacePoints::SetGas(" << CO2frac << ')' << std::endl;
    return lookup.SetGas("arco2", CO2frac);
}

vector<SpacePoints::Point3D> & SpacePoints::GetRPhiPoints(double thresh, double tfudge){
    points.clear();
    tspacepoints.Clear();

    if(!signals){
        cerr << "Signals not connected before calling GetRPhiPoints()" << endl;
        return points;
    }
    vector<Signals::signal> sig = signals->sanode;
    if(! sig.size()){
        cerr << "No anode times available to reconstruct r-phi-points." << endl;
        return points;
    }

    double tmin = 1e12;

    if(t0 >= 0){
        tmin = t0;
    } else{
        for(unsigned int i = 0; i < sig.size(); i++)
            if(sig[i].t < tmin) tmin = sig[i].t;
    }

    for(unsigned int i = 0; i < sig.size(); i++){
        if(sig[i].height < thresh) continue;
        int w = sig[i].i;
        double t = sig[i].t - tmin + tfudge;

        points.emplace_back(t, w, lookup, phi0);
	if( (points[i].x*points[i].x+points[i].y*points[i].y) == 0. ) continue;
	tspacepoints.AddLast(new TSpacePoint(w, -1, t,
					     points[i].x, points[i].y, 0.,
					     points[i].ex, points[i].ey, 1.e-8,
					     sig[i].height));
	//	std::cout<<i<<" ) R = "<<((TSpacePoint*)tspacepoints.Last())->GetR()<<" phi = "<<((TSpacePoint*)tspacepoints.Last())->GetPhi()<<std::endl;
    }
    tspacepoints.SetOwner();

    return points;
}

vector<SpacePoints::Point3D> & SpacePoints::GetPoints(double anodeThres, double padThres, double tfudge){
    tspacepoints.SetOwner();
    tspacepoints.Clear();

    if(!points.size()) GetRPhiPoints(anodeThres, tfudge);

    double ez = 4./TMath::Sqrt(12.);

    vector<set<Signals::signal, Signals::signal::heightorder> > pads = signals->MatchPads(anodeThres, padThres);
    double pt(padThres);
    // cout << "+++++ pad matches (" << pads.size() << " / " << points.size() << ')' << endl;
    while(pads.size() < 0.8*points.size() && pt >= 0.2*padThres){
        pt -= 0.1*padThres;
        cerr << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
        cerr << "+++++ not enough pad matches (" << pads.size() << " / " << points.size() << ')' << endl;
        cerr << "+++++ lowering pad threshold to " << pt << endl;
        cerr << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
        pads = signals->MatchPads(anodeThres, pt);
    }
    if(! pads.size()){
        cerr << "No pad times available to reconstruct points." << endl;
        return points;
    }

    if(pads.size() != points.size()){
        cerr << "Size mismatch! " << pads.size() << " != " << points.size() << endl;
        return points;
    }
    for(unsigned int i = 0; i < pads.size(); i++){
        double totHeight;
        static double zMean;

        Point3D &point = points[i];
        if((!pads[i].size()) || sqrt(point.x*point.x + point.y*point.y) > rcut || point.x*point.y == 0) continue;

        auto p = pads[i].begin();     // fancy C++11
        while(p != pads[i].end()){    // We need to look for multiple matches
            double z, phi;
            int index = p->i;
            TPCBase::GetPadPosition(p->i, z, phi);
            zMean = z * p->height;
            totHeight = p->height;
            p = pads[i].erase(p);
            while(p != pads[i].end()){
                if(abs(p->i - index) == 1){    // FIXME: This might introduce bias, because p->i == index+2 gets a neighbour removed. Investigate this.
                    TPCBase::GetPadPosition(p->i, z, phi);
                    zMean += z * p->height;
                    totHeight += p->height;
                    p = pads[i].erase(p);
                } else p++;
            }
            zMean /= mm;
            if(totHeight){
                zMean /= totHeight;
                point.SetZ(zMean, ez);
                tspacepoints.AddLast(new TSpacePoint(point.x, point.y, zMean, point.ex, point.ey, point.ez));
            }
            p = pads[i].begin();
        }
    }
    tspacepoints.SetOwner();
    return points;
}

SpacePoints::Point3D::Point3D(double rx, double phiy, double zz, bool polar){
    if(polar){
        r = rx;
        phi = phiy;
        z = zz;
        x = r*cos(phi);
        y = r*sin(phi);
    } else {
        x = rx;
        y = phiy;
        z = zz;
        r = sqrt(x*x+y*y);
        phi = atan(y/x);
    }
    t = unknown;
}

void SpacePoints::Point3D::SetErrors(double erx, double ephiy, double ezz, bool polar){
    if(polar){
        er = erx;
        ephi = ephiy;
        ez = ezz;
        ex = sqrt((x/r * er)*(x/r * er) + (-y * ephi)*(-y * ephi));
        ey = sqrt((y/r * er)*(y/r * er) + (x * ephi)*(x * ephi));
    } else {
        ex = erx;
        ey = ephiy;
        ez = ezz;
    }
}

SpacePoints::Point3D::Point3D(double t_, int anode_, TLookUpTable &lookup, double phi0){
    t = t_;
    anode = anode_;
    r = lookup.GetRadius(t);
    double phiWire, rWire;
    TPCBase::GetAnodePosition(anode, rWire, phiWire, true);
    phi = lookup.GetAzimuth(t) + phiWire + phi0;
    x = r*cos(phi);
    y = r*sin(phi);
    z = unknown;
    SetErrors(lookup.GetdR(t), lookup.GetdPhi(t));
}
