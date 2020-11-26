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
        cout << "a2mcGenerator::Generate ==> Could not generate primary, please check a2MC.ini or a2mcGenerator class" << endl;
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
    Bool_t genOK = false;
    return genOK;
}
//-----------------------------------------------------------------------------

Bool_t a2mcGenerator::GenMuons() 
{
    Bool_t genOK = false;
// ####################### Cosimic ray muons a2mc generator  ###################### --->     INIT
    if(a2mcConf.GetGenMode()==0) {
        ///< Using the a2mcMuonGen class to extract the generation parameters
        a2mcMuonGen gen;
        ///< muGenFlag = 0 [flat], 1 [spherical]
        Int_t muGenFlag = 1; ///< THIS PART NEED TO BE IMPLEMENTED [ALSO FLAT GENERATION, FOR THE MOMENT ONLY SPHERICAL]
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
            Double_t yOffset = -fDetConstruction->GetSilDet_R();
            Double_t zOffset = 0.;
            Double_t fSkyR   = 50.; ///< This has been set accordingly to the Oxford magnet size 

            gen.SetYOffset(yOffset);
            gen.SetZOffset(zOffset);
            gen.SetSphRadius(fSkyR);
            gen.Generate();         ///< ####### THIS IS THE ACTUAL GENERATION OF THE PARAMETERS ####### 
            ///< Getting the origin (generation position)
            std::array<double, 3> origin;
            gen.GetGenPoint(origin);            
            fGenPos[0] = origin[0];
            fGenPos[1] = origin[1];
            fGenPos[2] = origin[2];
            ///< Getting the momentum at generation
            ptot   = gen.GetGenMomentum();
            theta  = gen.GetTheta();
            phi    = gen.GetPhi();
            cx = sin(theta)*cos(phi);	
            cy = sin(theta)*sin(phi);
            cz = cos(theta);
            ///< In the aegisMuonGen the Z axis is the vertical one, here in the VMC the vertical is the Y
            ///< => fixing it here
            //  (y)|          VMC              //  (z)|         MuonGen
            //     |  / (x)                    //     |  / (y)
            //     | /                         //     | /
            //     |/_________ (z)             //     |/_________ (x)
            TVector3 Dir(cy, cz, cx);
            fGenMom[0] = ptot*Dir.X();
            fGenMom[1] = ptot*Dir.Y();
            fGenMom[2] = ptot*Dir.Z();
//            Double_t zenith = M_PI - theta; ///< zenithal angle (0- PI/2)            
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
    } ///< if(a2mcConf.GetGenMode()==0) --- END
// ####################### Cosimic ray muons a2mc generator  ###################### --->      END
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
