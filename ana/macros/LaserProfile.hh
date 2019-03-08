#include "../../reco/include/TPCconstants.hh"
#include <TMath.h>

using namespace TMath;

const double strippitch = 265.;
const double stripwidth = 6.;

set<double> strips;

double mm2pad(double mm){
    return (mm + _halflength)/_padpitch;
}

double pad2mm(double pad){
    return pad*_padpitch - _halflength;
}

double deg2col(double deg){
    return deg/360.*double(_padcol);
}

double col2deg(double col){
    return col/double(_padcol)*360.;
}

double deg2aw(double deg){
    double aw = deg/360.*double(_anodes)-0.5;
    if(aw < 0) aw += double(_anodes);
    return aw;
}

double aw2deg(double aw){
    aw += 0.5;
    aw = fmod(aw,double(_anodes));
    return aw/double(_anodes)*360.;
}

const vector<Double_t> lasProf_defaultPar()
{
    const double z0 = -1172.5,        // rod z position
        Rc = _cathradius,       // cathode radius
        Rr = Rc + 27.35,        // rod r position
        theta = 3.0, beta = -50.,
        phi_offset = 0.;
    const vector<Double_t> dpar = { z0,Rc,Rr,theta,beta,phi_offset };
    return dpar;
}

set<double> GetStrips(){
    strips.clear();
    for(double x = 0; x < _halflength; x += strippitch){
        strips.insert(x);
        strips.insert(-x);
    }
    return strips;
}

double FindNearestStrip(double z){
    double strip(9999);
    if(strips.size() == 0) strips = GetStrips();
    for(auto s: strips){
        if(abs(s - z) < abs(strip - z))
            strip = s;
    }
    return strip;
}

Double_t laser_profile(Double_t *x, Double_t *par)
{
    double phi_offset = par[5];
    bool top = phi_offset < 0.;
    phi_offset = abs(phi_offset)+par[6];

    double z0 = par[0],
                            Rc = par[1],
                            Rr = par[2],
                            theta = par[3]*DegToRad(),
                            beta = par[4]*DegToRad();

    double a = Cos(beta)*Cos(theta),
                            b = Sin(beta),
                            c = Cos(beta)*Sin(theta)/Rc,
                            d = (-Cos(beta)*Sin(theta)*z0 + Rr*Sin(beta))/Rc;

    double gamma = Sqrt(a*a+b*b),
                            delta = ATan2(b,a);


    double phi;
    if(top) phi = -1.*(ASin((-c*x[0] + d)/gamma)-delta);
    else phi = ASin((c*x[0]+d)/gamma)-delta;
    phi = phi*RadToDeg()+phi_offset;
    return phi;
}

Double_t laser_profile_pad(Double_t *x, Double_t *par)
{
    Double_t xx = pad2mm(x[0]);
    return deg2col(laser_profile(&xx,par));
}

vector<TF1*> GetPadProfile(double phi_offset = 0., double phi_lorentz_and_shift = 0.){
    TString pn = TString::Format("lasProf_phi%.2f",phi_offset);
    TF1 *lasProf = new TF1(pn,laser_profile_pad,0,_padrow,7);
    lasProf->SetParameters(lasProf_defaultPar().data());
    lasProf->SetParNames("z0","Rc","Rr","theta","beta","phi_offset");
    lasProf->SetParameter(5, phi_offset);
    lasProf->SetParameter(6, phi_lorentz_and_shift);
    vector<TF1*> profVec;
    profVec.push_back(lasProf);
    TF1 *fclone = nullptr;
    if(lasProf->Eval(lasProf->GetXmin()) > _padcol || lasProf->Eval(lasProf->GetXmax()) > _padcol){
        fclone = new TF1(*lasProf);
        fclone->SetParameter(5,phi_offset - 360.);
    } else if(lasProf->Eval(lasProf->GetXmin()) < 0 || lasProf->Eval(lasProf->GetXmax()) < 0){
        fclone = new TF1(*lasProf);
        fclone->SetParameter(5,phi_offset + 360.);
    }
    if(fclone) profVec.push_back(fclone);
    return profVec;
}

pair<double, double> GetPhiRange(double phi_offset = 0., double phi_lorentz_and_shift = 0.)
{
    TF1 ftmp("ftmp",laser_profile,-_halflength,_halflength,7);
    ftmp.SetParameters(lasProf_defaultPar().data());
    ftmp.SetParameter(5, phi_offset);
    ftmp.SetParameter(6, phi_lorentz_and_shift);
    double phimin = ftmp.GetMinimum();
    if(phimin < 0) phimin += 360.;
    double phimax = fmod(ftmp.GetMaximum(),360.);
    return pair<double, double>(phimin,phimax);
}

vector<pair<double, double> > GetLightPoints(double phi_offset = 0., double phi_lorentz_and_shift = 0.)
{
    vector<pair<double, double> > points;
    TF1 ftmp("ftmp",laser_profile,-_halflength,_halflength,7);
    ftmp.SetParameters(lasProf_defaultPar().data());
    ftmp.SetParameter(5, phi_offset);
    ftmp.SetParameter(6, phi_lorentz_and_shift);
    if(strips.size() == 0) strips = GetStrips();
    for(auto s: strips){
        double phi = fmod(ftmp.Eval(s),360.);
        if(phi < 0.) phi += 360.;
        points.emplace_back(s, phi);
    }
    return points;
}

TH2D *GetLightStrips(double phi_offset = 0., double phi_lorentz_and_shift = 0., short deg_aw_pads = 0) // Set pads to true to output in row/col coordinates instead of mm/deg
{
    TH2D *h;
    TF1 *f;
    switch(deg_aw_pads){
    case 0:
        h = new TH2D("hlight","light on Al strips;z[mm];phi[deg]",2*_halflength,-_halflength,_halflength,3600,-0.05,359.95);
        f = new TF1("ftmp",laser_profile,-_halflength,_halflength,7);
        break;
    case 1:
        h = new TH2D("hlight","light on Al strips;z[mm];anode",2*_halflength,-_halflength,_halflength,_anodes,-0.5,_anodes-0.5);
        f = new TF1("ftmp",laser_profile,-_halflength,_halflength,7);
        break;
    case 2:
        h = new TH2D("hlight","light on Al strips;row;col",_padrow,-0.5,_padrow-0.5,10*_padcol,-0.05,_padcol-0.05);
        f = new TF1("ftmp",laser_profile_pad,0,_padrow,7);
        break;
    default:
        return nullptr;
    }

    f->SetParameters(lasProf_defaultPar().data());
    f->SetParameter(5, phi_offset);
    f->SetParameter(6, phi_lorentz_and_shift);
    for(auto s: strips){
        double zmin = s-0.5*stripwidth;
        double zmax = s+0.5*stripwidth;
        double phimax = 360.;
        switch(deg_aw_pads){
        case 0:
        case 1: break;
        case 2:
            zmin = mm2pad(zmin);
            zmax = mm2pad(zmax);
            phimax = _padcol;
            break;
        }
        for(double z = zmin; z <= zmax; z++){
            double phi = fmod(f->Eval(z),phimax);
            if(phi < 0.) phi += phimax;
            if(deg_aw_pads == 1) phi = deg2aw(phi);
            h->Fill(z,phi);
        }
    }
    return h;
}

Double_t expected_intensity(Double_t *x, Double_t *par){
    bool top = (par[2] != 0);
    int offset = par[1];

    double xx = x[0] + _halflength;
    if(top) xx = -x[0] + _halflength;

    double A(0), x0(0), k(0);
    switch(offset){
    case 200: A = 215.47; x0 = -544.89; k = 0.457; break;
    case 300: A = 154.62; x0 = -392.23; k = 0.421; break;
    case 400: A = 140.00; x0 = -338.77; k = 0.412; break;
    case 500: A = 106.45; x0 = -234.08; k = 0.381; break;
    }

    return par[0]*exp(A/pow((xx-x0),k));
}

TF1 *GetExpInt(int offset, bool top){
    int itop = top;
    TString fn = TString::Format("lasInt_%c_%d",top?'T':'B',offset);
    TF1 *fint = new TF1(fn,expected_intensity,-_halflength,_halflength,3);
    fint->SetParameter(0,0.1);
    fint->FixParameter(1,offset);
    fint->FixParameter(2,(top?1:0));
    return fint;
}
