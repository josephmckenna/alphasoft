///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################


#include "a2mcGenerator.h"
#include "a2mcMuonGen.h"

ClassImp(a2mcGenerator);

a2mcGenerator* a2mcGenerator::fgInstance = 0;

using namespace std;

//-----------------------------------------------------------------------------
a2mcGenerator::a2mcGenerator(TVirtualMCStack* stack, a2mcApparatus* detConstruction)
   :TObject(),
    fStack(stack),
    fDetConstruction(detConstruction)
{
// Default constructor
    nGens = 0;
    nTrks = 0;
    if (fgInstance) {
        Fatal("muBlastGenerator", "Singleton instance already exists.");
        return;
    }
    if (!a2mcConf.isValid()) {
        cout << "Error reading configuration " << endl;
        return;
    } 
    fgInstance = this;

}
//-----------------------------------------------------------------------------
void a2mcGenerator::Generate()
{
    Bool_t genOK = false;
    if( 0<=a2mcConf.GetGenType()&&a2mcConf.GetGenType()<= 9)   genOK = GenPbars();
    if(10<=a2mcConf.GetGenType()&&a2mcConf.GetGenType()<=19)   genOK = GenMuons();
    if(!genOK) {
        cout << "a2mcGenerator::Generate ==> Could not generate primary with gen_type = " << a2mcConf.GetGenType() << endl;
        return;
    }
///< Add the primary particle to the stack
    Int_t    track_number; ///< it is filled by the stack
    fStack->PushTrack(1, -1, fPDG, 
        fGenMom[0],fGenMom[1],fGenMom[2], fTotE,
        fGenPos[0],fGenPos[1],fGenPos[2], 0., 
        0., 0., 0., kPPrimary, track_number, 1., 0);
///< ##########################
///< Input legend
///< TVirtualMCStack->PushTrack 
// PushTrack (Int_t toBeDone, Int_t parent, Int_t pdg, Double_t px, Double_t py, Double_t pz, Double_t e, Double_t vx, Double_t vy,
// Double_t vz, Double_t tof, Double_t polx, Double_t poly, Double_t polz, TMCProcess mech, Int_t &ntr, Double_t weight, Int_t is)
//    toBeDone - 1 if particles should go to tracking, 0 otherwise
//    parent - number of the parent track, -1 if track is primary
//    pdg - PDG encoding
//    px, py, pz - particle momentum [GeV/c]
//    e - total energy [GeV]
//    vx, vy, vz - position [cm]
//    tof - time of flight [s]
//    polx, poly, polz - polarization
//    mech - creator process VMC code
//    ntr - track number (is filled by the stack
//    weight - particle weight
//    is - generation status code
///< ##########################
    return;
}

//-----------------------------------------------------------------------------

Bool_t a2mcGenerator::GenPbars() 
{
//#   gen_type   [from a2MC.ini]
//#   0 => antiprotons 
//#       gen_mode 
//#       0 -> center of the silicon detector [pointlike source] 
//#       1 -> center of the silicon detector [Gaussian in Z, cylinder in radius]
//#       2 -> center of the silicon detector [Uniform  in Z, cylinder in radius]
    UInt_t gen_mode = a2mcConf.GetGenMode();
    if(gen_mode>2) {
        cout << "a2mcGenerator::GenPbars ==> Could not generate pbars with gen_mode = " << gen_mode << endl;
        return false;
    }
    fPDG = kProtonBar;
    TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(fPDG);
    Double_t mass = particlePDG->Mass(); // Mass
    ///<---------------------
    //|< 1) Setting variables 
    //\<---------------------
    Double_t zmin, zmax, zgen; 
    Double_t xySig, zSig;
    ///< Setting the paramenters [TEMPORARY - THEY NEED TO BE CHECKED]
    xySig   = 0.1;  // gaussian generation
    zSig    = 0.6;   // gaussian generation 
    zmin    = -1.;   // uniform generation
    zmax    = +1.;   // uniform generation
    zgen    = fDetConstruction->GetSilDet_Z(); ///< Centering on the silicon detector
    ///<---------------------
    //|< 2) Vertex generation 
    //\<---------------------
    if(gen_mode==0) xySig = 0.;
    fGenPos[0] = gRandom->Gaus(0.,xySig);
    fGenPos[1] = gRandom->Gaus(0.,xySig);
    fGenPos[2] = zgen;
    if(gen_mode==1) {
        fGenPos[2] += gRandom->Gaus(0.,zSig);
    }
    if(gen_mode==2) {
        fGenPos[2] += gRandom->Uniform(zmin,zmax);
    }
    ///<-----------------------
    //|< 3) Momentum generation 
    //\<-----------------------
    Double_t vmin, vmax, ptot;
    Double_t a;
    Double_t b;
    Double_t T      = 15.; // in Kelvin
    Double_t maxv   = 900.; // m/s
    Double_t m      = 1.672e-27; // Masso of the antiproton in kg
    Double_t k      = 1.380e-23; // Boltzmann constant
    Double_t theta  = -999.;
    Double_t phi    = -999.;
    Double_t v=0., v1=0., v2=0., v3=0.;  
    vmin = 200.;
    vmax = 600.;
    ///< Gaussian distribution
    v1      =   gRandom->Gaus(0.,sqrt(k*T/m));
    v2      =   gRandom->Gaus(0.,sqrt(k*T/m));
    v3      =   gRandom->Gaus(0.,sqrt(k*T/m));
    v       =   sqrt(v1*v1+v2*v2+v3*v3);
    theta   =   acos(v3/v);
    phi     =   atan2(v2,v1);
//    ///< Alternative way to generate velocity (Maxwellian) [TO BE TESTED]
//    while(1) {
//        a = gRandom->Uniform(0.,maxv); // m/s
//        b = gRandom->Uniform(0.,0.01);
//        if(b<Maxwell(a,T)) break;
//    }
//    v     = a;
//    phi   = gRandom->Uniform(0.,360.) * 3.141592/180.;
//    theta = acos(1.-2.*gRandom->Uniform(0.,1.)) ;
// ################### Momentum #########################
    UInt_t mag_field = a2mcConf.GetMagField();
    Double_t c, beta, gamma;
    c       = 299792458.; // Speed of light in m/s
    beta    = v/c;
    gamma   = 1/sqrt(1-beta*beta);
    ptot    = gamma*mass*beta; // relativistic formula, ptot is expressed in GeV
    ///< -----------------------------------------------------------------------
    ///< ----------------------------- WARNING ---------------------------------
    ///< -----------------------------------------------------------------------
    ///< It may be needed (according the the Geant4 version and settings) to put
    ///< a lower limit to ptot, to avoid that the antiproton is stuck in the 
    ///< magnetic field. This means that for mag_field !=0 annihilations may be 
    ///< in flight while for mag_field == 0 annihilations are at rest.
    ///< WARNING: overwriting the momentum if it is too low (only for mag_field != 0)
    Double_t plim = 5.0e-3; // in GeV
    if(mag_field!=0&&ptot<plim) ptot=plim; // Otherwise the antiproton doesn't leave the traps due to magnetic field (5 MeV/c)
    fTotE = sqrt(ptot*ptot + mass*mass);
    fKinE = fTotE - mass;
    Double_t px, py, pz;
    px = ptot*sin(theta)*cos(phi);
    py = ptot*sin(theta)*sin(phi);
    pz = ptot*cos(theta);

    fGenMom[0] = px;
    fGenMom[1] = py;
    fGenMom[2] = pz;

// REMOVE ME ... FORCING pz = 0;
//    fGenMom[0] = (px/fabs(px))*sqrt(pow(px,2.)+pow(pz/sqrt(2.),2.)); 
//    fGenMom[1] = (py/fabs(py))*sqrt(pow(py,2.)+pow(pz/sqrt(2.),2.)); 
//    fGenMom[2] = 0.;
// REMOVE ME --> END
// REMOVE ME ... FORCING pz = 0, py=ptot/sqrt(2.), px = ptot/sqrt(2.);
//            p[0] = ptot/sqrt(2.); // REMOVE ME
//            p[1] = ptot/sqrt(2.); // REMOVE ME
//            p[2] = 0.; // REMOVE ME 
// REMOVE ME --> END
    return true;
}

//-----------------------------------------------------------------------------

Bool_t a2mcGenerator::GenMuons() 
{
    if(a2mcConf.GetGenMode()==0) return GenMuSphere();
    if(a2mcConf.GetGenMode()==1) return GenMuFlat();
    cout << "a2mcGenerator::GenMuons ==> Could not generate primary with gen_type = " << a2mcConf.GetGenType() 
         << " and gen_mode " << a2mcConf.GetGenMode() << endl;
    return false;

}
//-----------------------------------------------------------------------------

Bool_t a2mcGenerator::GenMuSphere() 
{
///< In the a2mcMuonGen the Z axis is the vertical one, 
///<    here in the VMC the vertical one is the Y axis
///< => fixing it here
//  (y)|          VMC              //  (z)|         MuonGen
//     |  / (x)                    //     |  / (y)
//     | /                         //     | /
//     |/_________ (z)             //     |/_________ (x)
///< WARNING: x -> y, y -> z, z -> x (both in position and direction of the primary)
    Bool_t genOK = false;
// ####################### Cosimic ray muons a2mc generator  ###################### --->     INIT
    ///< Using the a2mcMuonGen class to extract the generation parameters
    a2mcMuonGen gen;
    gen.SetSphericalGeneration(true);
//        gen.SetHorizontalGeneration(false);
        
    ///< Variable used in the while
    Double_t mu_sign, mass, e;
    Double_t ptot, phi, theta, cx, cy, cz;
    Bool_t evtOK = false;
    while(!evtOK) {  ///< Filtering to get a "realistic" cosmic ray muons flux
        ///< Select mu+ or mu-
        mu_sign=gRandom->Uniform();
        if (mu_sign<0.444445) { ///< Experimental mu-/mu+ ratio [actually to be checked on the latest PDG review]
            fPDG = kMuonMinus;
        } else { 
            fPDG = kMuonPlus;
        }
        TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(fPDG);
            
        ///< All these values should be checked and set automatically getting parameters from a2mcApparatus
        Double_t fSkyOffset = -fDetConstruction->GetSilDet_R(); ///< Vertical offset
        Double_t fSkyRadius = 50.; ///< This has been set accordingly to the Oxford magnet size 

        gen.SetVertOffset(fSkyOffset);
        gen.SetHemiSphereRadius(fSkyRadius);
        gen.Generate();         ///< ####### THIS IS THE ACTUAL GENERATION OF THE PARAMETERS ####### 
        ///< Getting the origin (generation position)
        std::array<double, 3> origin;
        gen.GetGenPoint(origin);
        ///< WARNING: TAKING CARE OF DIFFERENT REFERENCE SYSTEMS (see above) 
        fGenPos[0] = origin[1];
        fGenPos[1] = origin[2];
        fGenPos[2] = origin[0];
        ///< Getting the momentum at generation
        ptot   = gen.GetGenMomentum();
        theta  = gen.GetTheta();
        phi    = gen.GetPhi();
        cx = sin(theta)*cos(phi);	
        cy = sin(theta)*sin(phi);
        cz = cos(theta);
        ///< WARNING: TAKING CARE OF DIFFERENT REFERENCE SYSTEMS (see above) 
        TVector3 Dir(cy, cz, cx);
        fGenMom[0] = ptot*Dir.X();
        fGenMom[1] = ptot*Dir.Y();
        fGenMom[2] = ptot*Dir.Z();
//        Double_t zenith = M_PI - theta; ///< zenithal angle (0- PI/2)            
        mass = particlePDG->Mass();
        fTotE = sqrt(mass*mass + ptot*ptot); ///< Total energy
        fKinE = fTotE - mass;
        // Filtering
        nGens++;
        evtOK = true;
            ///< Checking if the line "crosses" the SilDet cylinder (indeed a "double" SilDet  cylinder)
//            if(pbar_dump==10||pbar_dump==11) evtOK = CheckSilDetCrossing(TVector3(fXOrigin, fYOrigin, fZOrigin), Dir);
            ///< To measure the equivalent muon flux in minutes, check the interception of the generated muon with the 
            ///< base of the hemisphere
//            if(pbar_dump==1 && CheckHemisphereBase(TVector3(fXOrigin, fYOrigin, fZOrigin), Dir, fSkyR, yOffset)) nMuBase++;
//            evtOK = true; // NO filtering
//            //   if(s<0) evtOK = true; // opposite x,y filtering on generated muons: all out
//            //if(s>0) evtOK = true; // x,y filtering on generated muons
//            //   if(s>0 && zeta2> lGround) evtOK = true; // x,y,z filtering on generated muons;  n.b. sostituire con un get da Apparatus
////            if(s>0 && zeta2> lGround && zenith>PI/3.) evtOK = true; // x,y,z and zenith (>60 degrees) filtering on generated muons
//            //   if(s>0 && zeta1< -500. && zeta1> -1400. && zeta2> lGround && zenith>PI/2.4) evtOK = true; // x,y,z and zgen aroung crog and above refr and zenith (>75 degrees) filtering on generated muons
    } ///< while(!evtOK) --- END
    genOK = true;
    nTrks++;
    return genOK;
}
//-----------------------------------------------------------------------------

Bool_t a2mcGenerator::GenMuFlat() 
{
///< In the a2mcMuonGen the Z axis is the vertical one, 
///<    here in the VMC the vertical one is the Y axis
///< => fixing it here
//  (y)|          VMC              //  (z)|         MuonGen
//     |  / (x)                    //     |  / (y)
//     | /                         //     | /
//     |/_________ (z)             //     |/_________ (x)
///< WARNING: => x -> y, y -> z, z -> x (both in position and direction of the primary)
    Bool_t genOK = false;
// ####################### Cosimic ray muons a2mc generator  ###################### --->     INIT
    ///< Using the a2mcMuonGen class to extract the generation parameters
    a2mcMuonGen gen;
    gen.SetHorizontalGeneration(true);
        
    ///< Variable used in the while
    Double_t mu_sign, mass, e;
    Double_t ptot, phi, theta, cx, cy, cz;
    Bool_t evtOK = false;
    while(!evtOK) {  ///< Filtering to get a "realistic" cosmic ray muons flux
        ///< Select mu+ or mu-
        mu_sign=gRandom->Uniform();
        if (mu_sign<0.444445) { ///< Experimental mu-/mu+ ratio [actually to be checked on the latest PDG review]
            fPDG = kMuonMinus;
        } else { 
            fPDG = kMuonPlus;
        }
        TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(fPDG);

        ///< All these values should be checked and set automatically getting parameters from a2mcApparatus
        Double_t fSkySideX  = +fDetConstruction->GetWorldDx();
        Double_t fSkySideZ  = +fDetConstruction->GetWorldDz();
        Double_t fSkyOffset = +fDetConstruction->GetWorldDy()/2.-0.1; ///< Vertical offset

        gen.SetFlatSkySurface(fSkySideX,fSkySideZ);
        gen.SetVertOffset(fSkyOffset);
        gen.Generate();         ///< ####### THIS IS THE ACTUAL GENERATION OF THE PARAMETERS ####### 
        ///< Getting the origin (generation position)
        std::array<double, 3> origin;
        gen.GetGenPoint(origin);
        ///< WARNING: TAKING CARE OF DIFFERENT REFERENCE SYSTEMS (see above) 
        fGenPos[0] = origin[1];
        fGenPos[1] = origin[2];
        fGenPos[2] = origin[0];
        ///< Getting the momentum at generation
        ptot   = gen.GetGenMomentum();
        theta  = gen.GetTheta();
        phi    = gen.GetPhi();
        cx = sin(theta)*cos(phi);	
        cy = sin(theta)*sin(phi);
        cz = cos(theta);
        ///< WARNING: TAKING CARE OF DIFFERENT REFERENCE SYSTEMS (see above) 
        TVector3 Dir(cy, cz, cx);
        fGenMom[0] = ptot*Dir.X();
        fGenMom[1] = ptot*Dir.Y();
        fGenMom[2] = ptot*Dir.Z();
//        Double_t zenith = M_PI - theta; ///< zenithal angle (0- PI/2)            
        mass = particlePDG->Mass();
        fTotE = sqrt(mass*mass + ptot*ptot); ///< Total energy
        fKinE = fTotE - mass;
        // Filtering
        nGens++;
        evtOK = true;
    } ///< while(!evtOK) --- END
    genOK = true;
    nTrks++;
    return genOK;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void a2mcGenerator::DumpGenInfo()
{
    cout << "\t Primary particle information" << endl;
    a2mcMessenger::Instance()->printLeft(20,"\t X  = "); cout << fGenPos[0] << " cm " << endl;
    a2mcMessenger::Instance()->printLeft(20,"\t Y  = "); cout << fGenPos[1] << " cm " << endl;
    a2mcMessenger::Instance()->printLeft(20,"\t Z  = "); cout << fGenPos[2] << " cm " << endl;
    cout << "\t ----------------" << endl;
    Double_t pt = sqrt(fGenMom[0]*fGenMom[0]+fGenMom[2]*fGenMom[2]);
    Double_t ptot = sqrt(fGenMom[0]*fGenMom[0]+fGenMom[1]*fGenMom[1]+fGenMom[2]*fGenMom[2]);
    Double_t theta = atan2(fGenMom[1], pt);
    theta *= TMath::RadToDeg(); theta += 90.;
    a2mcMessenger::Instance()->printLeft(20,"\t Pt = "); cout << pt << " GeV/c " << endl;
    a2mcMessenger::Instance()->printLeft(20,"\t Py = "); cout << fGenMom[1] << " GeV/c " << endl;
    a2mcMessenger::Instance()->printLeft(20,"\t Ptot = "); cout << ptot << " GeV/c " << endl;        
    a2mcMessenger::Instance()->printLeft(20,"\t theta = "); cout << theta << " deg " << endl;
///< REMOVE ME
//        Double_t cx, cy, cz;
//        cx = fGenMom[0]/ptot; cy = fGenMom[1]/ptot; cz = fGenMom[2]/ptot;
//        a2mcMessenger::Instance()->printLeft(20,"\t cx = "); cout << cx << endl;
//        a2mcMessenger::Instance()->printLeft(20,"\t cy = "); cout << cy << endl;
//        a2mcMessenger::Instance()->printLeft(20,"\t cz = "); cout << cz << endl;
//        TVector3 PExtr = LineExtrapolation(TVector3(fGenPos[0], fGenPos[1], fGenPos[2]), TVector3(cx, cy, cz), fDetConstruction->GetLowerY());
//        a2mcMessenger::Instance()->printLeft(20,"\t XP = "); cout << PExtr.X() << endl;
//        a2mcMessenger::Instance()->printLeft(20,"\t YP = "); cout << PExtr.Y() << endl;
//        a2mcMessenger::Instance()->printLeft(20,"\t ZP = "); cout << PExtr.Z() << endl;
//        a2mcMessenger::Instance()->printLeft(20,"\t DR = "); cout << (fDetConstruction->GetLowerY()-fGenPos[1])*pt/fGenMom[1] << endl;
    return;
}

//_____________________________________________________________________________
Double_t a2mcGenerator::EquivalentTimeMin() {
    ///< Assuming a rate of 1 u/(cm^2 min)
    Double_t area_cm2 = fSkyDx*fSkyDz;
    Double_t time_in_min = area_cm2 == 0. ? 0. : nGens/area_cm2;
    
    Int_t runNumber = fDetConstruction->GetRunNumber();
    return time_in_min;
}
//_____________________________________________________________________________
Bool_t a2mcGenerator::CheckHemisphereBase(TVector3 P0, TVector3 Dir, Double_t R, Double_t Y) {
    ///< This method extrapolates the generated muon (line in 3D) to a specific Y plane
    ///< to check if the generated muon "crosses" the BASE of the hemisphere  
    ///< Line P = P0 + Dir*t [P = (X, Y, Z), P0 = (X0, Y0, Z0), Dir = (cx, cy, cz)]
    ///< Finding intersection with a circle of radius R and vertical position Y
    // Y coordinate is the vertical one
    //  (y)| 
    //     |  / (x)
    //     | /
    //     |/_________ (z)
    ///<   Line P = P0 + Dir*t [P = (X, Y, Z), P0 = (X0, Y0, Z0), Dir = (cx, cy, cz)]
    ///< 
    if(Dir.Y()==0.) return false; ///< The line is parallel to the XZ plane => no extrapolation 
    ///< Find free parameter t
    Double_t t = (Y-P0.Y())/Dir.Y();
    ///< Find the extrapolation at Y
    TVector3 P1 = P0 + t*Dir;
    ///< Assuming here that the center of the hemisphere in X and Z is in 0    
    if(sqrt(P1.X()*P1.X()+P1.Z()*P1.Z())<R) return true; ///< The extrapolation fits in the hemisphere base
    return false;
}
//_____________________________________________________________________________
Bool_t a2mcGenerator::CheckSilDetCrossing(TVector3 P0, TVector3 Dir) {
    ///< This method check if the generated muon "crosses" the SilDet cylinder
    ///< Line P = P0 + Dir*t [P = (X, Y, Z), P0 = (X0, Y0, Z0), Dir = (cx, cy, cz)]
    ///< Finding intersection with a cylinder of radius SilDet_R*2 
//    cout << "CheckSilDetCrossing ________________________________________________________________" << endl;
    Double_t SilDet_Z = fDetConstruction->GetSilDet_Z();
    Double_t SilDet_L = fDetConstruction->GetSilDet_L();
    Double_t SilDet_R = fDetConstruction->GetSilDet_R();
//    cout << "SilDet_Z " << SilDet_Z << ", SilDet_L " << SilDet_L << ", SilDet_R " << SilDet_R << endl;
//    cout << "P0.X() " << P0.X() << " P0.Y() " << P0.Y() << " P0.Z() " << P0.Z()<< endl;
    if(Dir.Y()==0.) return false; ///< The line is parallel to the XZ plane => no extrapolation 
    ///< Find free parameter t, if any (interection of a line with a cylinder) - checking in the  X, Y plane
    Double_t a = Dir.X()*Dir.X()+ Dir.Y()*Dir.Y();
    Double_t b = 2*(Dir.X()*P0.X()+Dir.Y()*P0.Y());
    Double_t c = P0.X()*P0.X() + P0.Y()*P0.Y() - (SilDet_R*2.)*(SilDet_R*2.); ///< Checking for a cylinder of a radius = 2.*SilDet_R
    
    Double_t det = b*b-4*a*c;
//    cout << "det " << det << endl;
    if(det<0) return false;
    Double_t t1 = (-b - sqrt(det))/(2.*a);
    Double_t t2 = (-b + sqrt(det))/(2.*a);
    Double_t Z1 = P0.Z() + t1*Dir.Z();
    Double_t Z2 = P0.Z() + t2*Dir.Z();
//    cout << "Z1 " << Z1 << " Z2 " << Z2 << endl;
    TVector3 P1 = P0 + t1*Dir;
    TVector3 P2 = P0 + t2*Dir;
//    cout << "R1 " << sqrt(P1.X()*P1.X()+P1.Y()*P1.Y()) << endl;
//    cout << "R2 " << sqrt(P2.X()*P2.X()+P2.Y()*P2.Y()) << endl;
//    if(fabs(Z1-SilDet_Z)<SilDet_L||fabs(Z2-SilDet_Z)<SilDet_L) {
//        cout << "------------ YES ------------ " << endl;
//    }
    if(fabs(Z1-SilDet_Z)<SilDet_L) return true;///< Checking for a cylinder of DZ = 2.SilDet_L
    if(fabs(Z2-SilDet_Z)<SilDet_L) return true;///< Checking for a cylinder of DZ = 2.SilDet_L
    return false;
}
//_____________________________________________________________________________
TVector3 a2mcGenerator::LineExtrapolation(TVector3 P0, TVector3 Dir, Double_t YP) {
    ///< This method extrapolates the generated muon (line in 3D) to a specific Y plane
    // Y coordinate is the vertical one
    //  (y)| 
    //     |  / (x)
    //     | /
    //     |/_________ (z)
    ///<   Line P = P0 + Dir*t [P = (X, Y, Z), P0 = (X0, Y0, Z0), Dir = (cx, cy, cz)]
    ///< 
    TVector3 P(0.,0.,0.);
    ///< Find free parameter t
    if(Dir.Y()==0.) return P; ///< The line is parallel to the XZ plane => no extrapolation 

    P.SetY(YP);
    Double_t t = (P.Y()-P0.Y())/Dir.Y();
    P.SetX(P0.X() + Dir.X()*t);
    P.SetZ(P0.Z() + Dir.Z()*t);
//    cout << "a2mcGenerator::LineExtrapolator ===> DX = " << P.X()- P0.X() << ", DZ = " << P.Z() - P0.Z() << " DR = ";
//    cout << sqrt((P.X()- P0.X())*(P.X()- P0.X())+ (P.Z()- P0.Z())*(P.Z()- P0.Z())) << endl;
    return P;
}

//-----------------------------------------------------------------------------
Double_t a2mcGenerator::Maxwell(Double_t v, Double_t T)
{

  Double_t m = 1.672e-27;
  Double_t k = 1.380e-23;
  Double_t maxww;
  
  maxww = (4/sqrt(3.141592))*pow(m/(2*k*T),1.5)*v*v*exp(-m*v*v/(2*k*T));
  return maxww;
}

//_____________________________________________________________________________
a2mcGenerator::~a2mcGenerator()
{
    /// Destructor
    fgInstance = 0;    
}
//_____________________________________________________________________________
a2mcGenerator* a2mcGenerator::Instance()
{
    /// \return The singleton instance.
    return fgInstance;
}

//-----------------------------------------------------------------------------
