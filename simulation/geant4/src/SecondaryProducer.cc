// -------------------------------------------------------------------
//
// Class   SecondaryProducer
//
// A. Capra     created Sep. 2013
//              used as primary generator 11 March 2014
//
// -------------------------------------------------------------------

#include "SecondaryProducer.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

#include "Randomize.hh"

#include "G4PrimaryParticle.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

bool gpi0only = false;

// LGCP : Below are the number of particles involved when a certain branch/decay mode is taken
G4int NumPar[] = 
{ 
     2,                                 // p0 p0   
     4,                                 // p0 p0 p0 p0 
     3,                                 // p+ p- p0
     4,                                 // p+ p- p0 p0
     5,                                 // p+ p- p0 p0 p0
     6,                                 // p+ p- p0 p0 p0 p0      
     4,                                 // p+ p- p+ p-
     5,                                 // p0 p+ p- p+ p-
     6,                                 // p0 p0 p+ p- p+ p-
     7,                                 // p0 p0 p0 p+ p- p+ p-
     6,                                 // p+ p- p+ p- p+ p-
     7,                                 // p+ p- p+ p- p+ p- p0
     2,                                 // p+ p-
     3                                  // p0 p0 p0      
};


G4int NumParpi0[] = 
{ 
     2,                                 // p0 p0   
     4,                                 // p0 p0 p0 p0 
     3                                  // p0 p0 p0      
};


#define MPC 0.139566 // LGCP : This is the mass of a charged pion in GeV/c^2
#define MP0 0.134976 // LGCP : This is the mass of a neutral pion in GeV/c^2

// LGCP : Below are the masses of the particles involved when a certain branch/annihilation mode is taken
G4double ParMas[][7] = 
{ 
     { MP0, MP0,   0,   0,   0,   0,   0},   // p0 p0
     { MP0, MP0, MP0, MP0,   0,   0,   0},   // p0 p0 p0 p0
     { MP0, MPC, MPC,   0,   0,   0,   0},   // p0 p+ p-
     { MP0, MP0, MPC, MPC,   0,   0,   0},   // p0 p0 p+ p-
     { MP0, MP0, MPC, MPC, MP0,   0,   0},   // p0 p0 p0 p+ p-
     { MP0, MP0, MPC, MPC, MP0, MP0,   0},   // p0 p0 p0 p0  p+ p-
     { MPC, MPC, MPC, MPC,   0,   0,   0},   // p+ p- p+ p-
     { MP0, MPC, MPC, MPC, MPC,   0,   0},   // p0 p+ p- p+ p-
     { MP0, MP0, MPC, MPC, MPC, MPC,   0},   // p0 p0 p+ p- p+ p-
     { MP0, MP0, MPC, MPC, MPC, MPC, MP0},   // p0 p0 p0 p+ p- p+ p-
     { MPC, MPC, MPC, MPC, MPC, MPC,   0},   // p+ p- p+ p- p+ p-
     { MPC, MPC, MPC, MPC, MPC, MPC, MP0},   // p+ p- p+ p- p+ p- p0
     { MPC, MPC,   0,   0,   0,   0,   0},   // p+ p-
     { MP0, MP0, MP0,   0,   0,   0,   0}    // p0 p0 p0    
};


G4double ParMaspi0[][7] = 
{ 
     { MP0, MP0,   0,   0,   0,   0,   0},   // p0 p0
     { MP0, MP0, MP0, MP0,   0,   0,   0},   // p0 p0 p0 p0
     { MP0, MP0, MP0,   0,   0,   0,   0}    // p0 p0 p0    
};

// LGCP : Below are the GEANT3 particle id codes for the particles involved when a certain branch/annihilation mode is taken

#define PI0 111     // PDG particle codes
#define PIP 211
#define PIM -211

G4int ParNum[][7] = 
{    
     {   PI0,   PI0,   0,     0,     0,     0,     0},   // p0 p0        
     {   PI0,   PI0,   PI0,   PI0,   0,     0,     0},   // p0 p0 p0 p0                
     {   PI0,   PIP,   PIM,   0,     0,     0,     0},   // p0 p+ p-
     {   PI0,   PI0,   PIP,   PIM,   0,     0,     0},   // p0 p0 p+ p-
     {   PI0,   PI0,   PIP,   PIM,   PI0,   0,     0},   // p0 p0 p0 p+ p-   
     {   PI0,   PI0,   PIP,   PIM,   PI0,   PI0,   0},   // p0 p0 p0 p0 p+ p-            
     {   PIP,   PIM,   PIP,   PIM,   0,     0,     0},   // p+ p- p+ p-
     {   PI0,   PIP,   PIM,   PIP,   PIM,   0,     0},   // p0 p+ p- p+ p-
     {   PI0,   PI0,   PIP,   PIM,   PIP,   PIM,   0},   // p0 p0 p+ p- p+ p-
     {   PI0,   PI0,   PIP,   PIM,   PIP,   PIM,   PI0}, // p0 p0 p0 p+ p- p+ p-
     {   PIP,   PIM,   PIP,   PIM,   PIP,   PIM,   0},   // p+ p- p+ p- p+ p-
     {   PIP,   PIM,   PIP,   PIM,   PIP,   PIM,   PI0}, // p+ p- p+ p- p+ p- p0
     {   PIP,   PIM,   0,     0,     0,     0,     0},   // p+ p-
     {   PI0,   PI0,   PI0,   0,     0,     0,     0}    // p0 p0 p0 
};


G4int ParNumpi0[][7] = 
{    
     {   PI0,   PI0,   0,     0,     0,     0,     0},   // p0 p0        
     {   PI0,   PI0,   PI0,   PI0,   0,     0,     0},   // p0 p0 p0 p0                
     {   PI0,   PI0,   PI0,   0,     0,     0,     0}    // p0 p0 p0 
};


// LGCP : This MaxWeight function still puzzles me.
// LGCP : This is supposed to be, according to Pablo Genova, a correction to the phase space
G4double MaxWeight[] = 
{
     1.000000, // p0 p0  
     0.118145, // p0 p0 p0 p0  
     0.411809, // p0 p+ p-  
     0.118144, // p0 p0 p+ p-
     0.027187, // p0 p0 p0 p+ p-
     0.005407, // p0 p0 p0 p0 p+ p-
     0.118187, // p+ p- p+ p-  
     0.027182, // p0 p+ p- p+ p-  
     0.005405, // p0 p0 p+ p- p+ p-
     0.000963, // p0 p0 p0 p+ p- p+ p- 
     0.005405, // p+ p- p+ p- p+ p-  
     0.000967, // p+ p- p+ p- p+ p- p0  
     1.000000, // p+ p-  
     0.411814  // p0 p0 p0 
};


G4double MaxWeightpi0[] = 
{
     // correction factor? 3.250615/1.529959*
     1.000000, // p0 p0  
     0.118145, // p0 p0 p0 p0  
     0.411814  // p0 p0 p0 
};


// LGCP : These are the branching ratios for the different p-pbar annihilation modes
G4double BraRat[] = 
{  
     .00028,   // p0 p0  
     .03000,   // p0 p0 p0 p0               
     .06900,   // p0 p+ p-  
     .09300,   // p0 p0 p+ p-
     .23300,   // p0 p0 p0 p+ p-
     .02800,   // p0 p0 p0 p0 p+ p- 
     .06900,   // p+ p- p+ p-  
     .19600,   // p0 p+ p- p+ p-  
     .16600,   // p0 p0 p+ p- p+ p-  
     .04200,   // p0 p0 p0 p+ p- p+ p-  
     .02100,   // p+ p- p+ p- p+ p-  
     .01900,   // p+ p- p+ p- p+ p- p0  
     .00320,   // p+ p-
     .00760    // p0 p0 p0   
};


G4double BraRatpi0[] = 
{  
     // correction factor? *0.97708/0.03788
     .00028*0.97708/0.03788,   // p0 p0  
     .03000*0.97708/0.03788,   // p0 p0 p0 p0               
     .00760*0.97708/0.03788    // p0 p0 p0   
};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

SecondaryProducer::SecondaryProducer() : pspi(), Ppi(0.,0.,0.,2.*938.279e-3),
					 psph(), Pph(0.,0.,0.,2.*511.e-6),
					 fsecondaries(0), fVerbose(0)
{
  

  G4double nor = 0.;
  switch(gpi0only)
    {
      case true:
        {
          NFS = 3;
          BraRatCum = new G4double[NFS];
          for (G4int k=0; k<NFS; k++) nor+=BraRatpi0[k];
          for (G4int k=0; k<NFS; k++) BraRatpi0[k] /= nor;
      
          BraRatCum[0] = BraRatpi0[0];
          for (G4int k=1;k<NFS;k++) BraRatCum[k] = BraRatpi0[k] + BraRatCum[k-1];
          break;
        }
      case false:
        {
          NFS = 14;
          BraRatCum = new G4double[NFS];
          for (G4int k=0; k<NFS; k++) nor+=BraRat[k];
          for (G4int k=0; k<NFS; k++) BraRat[k] /= nor;
        
          BraRatCum[0] = BraRat[0];
          for (G4int k=1;k<NFS;k++) BraRatCum[k] = BraRat[k] + BraRatCum[k-1];
          break;
        }
      default:
        {
          NFS = 14;
          BraRatCum = new G4double[NFS];
          G4cout<<"Error in pi0only assignment"<<G4endl;
          for (G4int k=0; k<NFS; k++) nor+=BraRat[k];
          for (G4int k=0; k<NFS; k++) BraRat[k] /= nor;
        
          BraRatCum[0] = BraRat[0];
          for (G4int k=1;k<NFS;k++) BraRatCum[k] = BraRat[k] + BraRatCum[k-1];
        }
      
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
SecondaryProducer::~SecondaryProducer()
{
  delete[] BraRatCum;
  fsecondaries.clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4int SecondaryProducer::GetChannel()
{
  G4double fs = G4UniformRand();
  G4int fFs = 0;
  while (fs>BraRatCum[fFs]) fFs++;
  return fFs;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double SecondaryProducer::GetWeight(G4int fFs)
{
  G4double wt=0.;
  while (1) // make unweighted events
    {
      wt = pspi.Generate(); // generate a phasespace configuration
      G4double wt_r = MaxWeight[fFs]*G4UniformRand(); // random number between
      switch(gpi0only)
        {
          case true:
            {
              wt_r = MaxWeightpi0[fFs]*G4UniformRand(); // random number between
                                                          // [0,MaxWeightpi0]
              break;
            }
          case false:
            {
              wt_r = MaxWeight[fFs]*G4UniformRand(); // random number between
                                                          // [0,MaxWeight]
              break;
            }
          default:
            {
              G4cout<<"Error in pi0only assignment"<<G4endl;
              wt_r = MaxWeight[fFs]*G4UniformRand(); // random number between
                                                          // [0,MaxWeight]
            }
        }
    
      //printf("wt: %lf wt_r: %lf\n",wt,wt_r);
      if(wt > wt_r) break;
    }
  return wt;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4int SecondaryProducer::Produce()
{  
  G4int ch = GetChannel();
  G4int NofSecondaries;
  switch(gpi0only)
    {
      case true:
        {
          NofSecondaries = NumParpi0[ch];
          pspi.SetDecay(Ppi, NofSecondaries, ParMaspi0[ch]);
          break;
        }
      case false:
        {
          NofSecondaries = NumPar[ch];
          pspi.SetDecay(Ppi, NofSecondaries, ParMas[ch]);
          break;
        }
      default:
        {
          G4cout<<"Error in pi0only assignment"<<G4endl;
          NofSecondaries = NumPar[ch];
          pspi.SetDecay(Ppi, NofSecondaries, ParMas[ch]);
        }
    }
    
  pspi.SetDecay(Ppi, NofSecondaries, ParMas[ch]);

  //  GetWeight(ch);
  G4double wt=0.;
  while (1) // make unweighted events
    {
      wt = pspi.Generate(); // generate a phasespace configuration
      G4double wt_r = MaxWeight[ch]*G4UniformRand(); // random number between
      switch(gpi0only)
        {
          case true:
            {
              wt_r = MaxWeightpi0[ch]*G4UniformRand(); // random number between
                                                          // [0,MaxWeightpi0]
              break;
            }
          case false:
            {
              wt_r = MaxWeight[ch]*G4UniformRand(); // random number between
                                                          // [0,MaxWeight]
              break;
            }
          default:
            {
              G4cout<<"Error in pi0only assignment"<<G4endl;
              wt_r = MaxWeight[ch]*G4UniformRand(); // random number between
                                                          // [0,MaxWeight]
            }
        }
    
      //printf("wt: %lf wt_r: %lf\n",wt,wt_r);
      if(wt > wt_r) break;
    }

  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();

  for (G4int n=0; n<NofSecondaries; ++n) 
    {
      // Lorentz Vector corresponding to the Nth decay
      G4double Epi = (pspi.GetDecay(n)->E())*GeV;       // Energy in GeV
      TVector3 ppi = pspi.GetDecay(n)->Vect();
      G4double pix = ppi.X()*GeV;
      G4double piy = ppi.Y()*GeV;
      G4double piz = ppi.Z()*GeV;

      // Particle type
      G4int pdg  = ParNum[ch][n];
      switch(gpi0only)
        {
          case true:
            {
              pdg  = ParNumpi0[ch][n];
              break;
            }
          case false:
            {
              pdg  = ParNum[ch][n];
              break;
            }
          default:
            {
              G4cout<<"Error in pi0only assignment"<<G4endl;
              pdg  = ParNum[ch][n];
            }
        }
      
      G4ParticleDefinition* pion = particleTable->FindParticle(pdg);

      if(fVerbose>0)
	{
	  G4cout<<n<<"\t"<<pion->GetParticleName()<<"\t"
		<<G4BestUnit(Epi,"Energy")<<G4endl;
	  ppi.Print();
	  G4cout<<"----"<<G4endl;
	}

      fsecondaries.push_back( new G4PrimaryParticle( pion, 
						     pix,piy,piz,
						     Epi ) );
    }

  G4ParticleDefinition* photon = particleTable->FindParticle(22);
  G4double mass[] = {0.,0.};
  psph.SetDecay( Pph, 2, mass );
  psph.Generate();
  for(G4int n=0; n<2; ++n)
    {
      G4double Eph = (psph.GetDecay(n)->E())*GeV;// Energy in GeV      
      TVector3 pph = psph.GetDecay(n)->Vect();
      G4double phx = pph.X()*GeV;
      G4double phy = pph.Y()*GeV;
      G4double phz = pph.Z()*GeV;
      if(fVerbose>0)
	{
	  G4cout<<n<<"\t"<<photon->GetParticleName()<<"\t"
		<<G4BestUnit(Eph,"Energy")<<G4endl;
	  pph.Print();
	  G4cout<<"----"<<G4endl;
	}

      fsecondaries.push_back( new G4PrimaryParticle( photon, 
						     phx,phy,phz, 
						     Eph ) );
    }

  return NofSecondaries+2;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
G4PrimaryParticle* SecondaryProducer::GetSecondary(G4int n)
{
  unsigned int i = (unsigned int) n;
  if( i > (fsecondaries.size()-1) ) return 0;
  return fsecondaries.at(i);
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
void SecondaryProducer::ClearSecondaries()
{
  fsecondaries.clear();
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
