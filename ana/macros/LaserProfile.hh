#include "../../reco/include/TPCconstants.hh"
#include <TMath.h>

using namespace TMath;

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

Double_t laser_profile(Double_t *x, Double_t *par)
{
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


    double phi = ASin((c*x[0]+d)/gamma)-delta;
    return phi*RadToDeg();
}

Double_t laser_profile_pad(Double_t *x, Double_t *par)
{
    double phi_offset = par[5];
    bool top = phi_offset < 0.;
    phi_offset = abs(phi_offset);

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
    if(top) phi = -1.*(ASin((c*(_padrow - pad2mm(x[0])) + d)/gamma)-delta);
    else phi = ASin((c*pad2mm(x[0])+d)/gamma)-delta;
    phi = phi*RadToDeg()+phi_offset;
    // if(phi < 0.) phi += 360.;
    // else if(phi > 360.) phi -= 360.;
    return deg2col(phi);
}

vector<TF1*> GetPadProfile(double phi_offset = 0.){
    TString pn = TString::Format("lasProf_phi%.2f",phi_offset);
    TF1 *lasProf = new TF1(pn,laser_profile_pad,0,_padrow,6);
    lasProf->SetParameters(lasProf_defaultPar().data());
    lasProf->SetParNames("z0","Rc","Rr","theta","beta","phi_offset");
    lasProf->SetParameter(5, phi_offset);
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
