#include <set>
#include <TMath.h>

#include "ChamberGeo.hh"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::set;

vector<double> ChamberGeo::GetPhiRange(double R){
    vector<double> phirange;
    //    cout << "R = " << R << endl;
    if(x0 != x1 && y0 != y1){
        double phiY0 = asin(y0/R)*180./TMath::Pi();
        double phiX1 = acos(x1/R)*180./TMath::Pi();
        double phiY1 = asin(y1/R)*180./TMath::Pi();
        double phiX0 = acos(x0/R)*180./TMath::Pi();

        set<double> phis;
        if(phiY0 == phiY0){
            if(phiY0 > 180) phiY0 -= 360;
            phis.insert(phiY0);
            double phiY02 = 180-phiY0;
            if(phiY02 > 180) phiY02 -= 360;
            phis.insert(phiY02);
        }
        if(phiX1 == phiX1){
            if(phiX1 > 180) phiX1 -= 360;
            phis.insert(-phiX1);
            phis.insert(phiX1);
        }
        if(phiY1 == phiY1){
            if(phiY1 > 180) phiY1 -= 360;
            phis.insert(phiY1);
            double phiY12 = 180-phiY1;
            if(phiY12 > 180) phiY12 -= 360;
            phis.insert(phiY12);
        }
        if(phiX0 == phiX0){
            if(phiX0 > 180) phiX0 -= 360;
            phis.insert(-phiX0);
            phis.insert(phiX0);
        }

        for(set<double>::iterator it = phis.begin(); it != phis.end(); it++){
            double phi1 = *it++;
            double phi2;
            if(it == phis.end()){
                phi2 = *phis.begin();
                it--;
            } else phi2 = *it--;
            double phimed = (phi1 + phi2)/2;
            if(phi2 < phi1) phimed += 180;
            phimed *= TMath::Pi()/180;
            double x = R*cos(phimed);
            double y = R*sin(phimed);
            if(x > x0 && x < x1 && y > y0 && y < y1){
                phirange.push_back(phi1);
                phirange.push_back(phi2);
            }
        }
    }
    if(phirange.size() < 2) phirange = {0, 360};
    if(phirange.size() % 2) cerr << "EE: phirange.size() = " << phirange.size() << endl;
    // for(unsigned int i = 0; i < phirange.size(); i+=2)
    //     cout << phirange[i] << " -> " << phirange[i+1] << endl;

    return phirange;
}

vector<TEllipse*> ChamberGeo::GetCathode2D(){
    vector<TEllipse*> ell;
    vector<double> phirange = GetPhiRange(TPCBase::CathodeRadius/mm);
    for(unsigned int i = 0; i < phirange.size(); i += 2){
        ell.push_back(new TEllipse(0,0,TPCBase::CathodeRadius/mm,TPCBase::CathodeRadius/mm,phirange[i],phirange[i+1]));
        ell.back()->SetNoEdges();
        ell.back()->SetFillStyle(0);
    }
    return ell;
}

vector<TEllipse*> ChamberGeo::GetPadCircle2D(){
    vector<double> phirange = GetPhiRange(TPCBase::ROradius/mm);
    vector<TEllipse*> ell;
    for(unsigned int i = 0; i < phirange.size(); i += 2){
        ell.push_back(new TEllipse(0,0,TPCBase::ROradius/mm,TPCBase::ROradius/mm,phirange[i],phirange[i+1]));
        ell.back()->SetNoEdges();
        ell.back()->SetFillStyle(0);
    }
    return ell;
}

TGraph *ChamberGeo::GetAnodeWires(int i0, int i1){
    TGraph *wires = new TGraph;
    wires->SetMarkerStyle(4);
    for(int i = i0; i < i1; i++){
        double x, y;
        TPCBase::GetAnodePosition(i, x, y, false, phi0);
        x /= mm;
        y /= mm;
        if(x0 != x1 && y0 != y1)
            if(x < x0 || x > x1 || y < y0 || y > y1) continue;
        wires->SetPoint(wires->GetN(), x, y);
    }
    return wires;
}

TGraph *ChamberGeo::GetFieldWires(int i0, int i1){
    TGraph *wires = new TGraph;
    wires->SetMarkerStyle(4);
    for(int i = i0; i < i1; i++){
        double x, y;
        TPCBase::GetWirePosition(i, x, y, false, phi0);
        x /= mm;
        y /= mm;
        if(x0 != x1 && y0 != y1)
            if(x < x0 || x > x1 || y < y0 || y > y1) continue;
        wires->SetPoint(wires->GetN(), x, y);
    }
    return wires;
}

vector<TLine> ChamberGeo::GetPadSectors(){
    vector<TLine> lines;
    double dphi = TMath::TwoPi()/32.;
    for(int i = 0; i < 32; i++){
        double phi = i*dphi;
        double x0 = 10.*TPCBase::CathodeRadius*TMath::Cos(phi);
        double y0 = 10.*TPCBase::CathodeRadius*TMath::Sin(phi);
        double x1 = 10.*TPCBase::ROradius*TMath::Cos(phi);
        double y1 = 10.*TPCBase::ROradius*TMath::Sin(phi);

        lines.push_back(TLine(x0, y0, x1, y1));
        lines.back().SetLineWidth(1);
        lines.back().SetLineStyle(2);
    }
    return lines;
}
