///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcStack.h"
#include "a2mcRootManager.h"

ClassImp(a2mcStack)
a2mcStack* a2mcStack::fgInstance = 0;

//_____________________________________________________________________________
a2mcStack::a2mcStack(Int_t size)
  : fParticles(0),
    fTracks(0),
    fCurrentTrack(-1),
    fNPrimary(0)
{

    // Instance is then used in a2mcXXSD.cxx to access a2mcStack functions
    if (fgInstance) {
        Fatal("a2mcStack", "Singleton instance already exists.");
        return;
    }
  
    fParticles = new TClonesArray("TParticle", size);
    fTracks    = new TClonesArray("a2mcMCTrack", size); 
    // fTracks -> Same as fParticles [+ hit points along the track ... not implemente here for the moment]
    fTracksLim = size;
    fgInstance = this;
}

//_____________________________________________________________________________
a2mcStack::a2mcStack()
  : fParticles(0),
    fTracks(0),
    fCurrentTrack(-1),
    fNPrimary(0)
{
/// Default constructor
    if (fgInstance) {
        Fatal("a2mcStack", "Singleton instance already exists.");
        return;
    }
    fgInstance = this;  
    fTracksLim = 1000; ///< Setting a limit to 1000 ... just to be sure
}

//_____________________________________________________________________________
a2mcStack::~a2mcStack() 
{
/// Destructor

  if (fParticles) fParticles->Delete();
  delete fParticles;

  if (fTracks) fTracks->Delete();
  delete fTracks;
  fgInstance = 0;
}
//_____________________________________________________________________________
void  a2mcStack::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
  	                 Double_t px, Double_t py, Double_t pz, Double_t e,
  		         Double_t vx, Double_t vy, Double_t vz, Double_t tof,
		         Double_t polx, Double_t poly, Double_t polz,
		         TMCProcess mech, Int_t& ntr, Double_t weight,
		         Int_t is) 
{
/// Create a new particle with specified properties,
/// adds it to the particles array (fParticles) and if not done to the 
/// stack (fStack).
/// Use TParticle::fMother[1] to store Track ID. 
/// toBeDone  1 if particles should go to tracking, 0 otherwise
/// parent    number of the parent track, -1 if track is primary
/// pdg       PDG encoding
/// px        particle momentum - x component [GeV/c]
/// py        particle momentum - y component [GeV/c]
/// pz        particle momentum - z component [GeV/c]
/// e         total energy [GeV]
/// vx        position - x component [cm]
/// vy        position - y component  [cm]
/// vz        position - z component  [cm]
/// tof       time of flight [s]
/// polx      polarization - x component
/// poly      polarization - y component
/// polz      polarization - z component
/// mech      creator process VMC code
/// ntr       track number (is filled by the stack
/// weight    particle weight
/// is        generation status code

    const Int_t kFirstDaughter=-1;
    const Int_t kLastDaughter=-1;

    // Adding new particle
    TClonesArray& particlesRef = *fParticles;
    Int_t trackId = GetNtrack();
    TParticle* particle = new(particlesRef[trackId]) TParticle(pdg, is, parent, trackId, kFirstDaughter, kLastDaughter, px, py, pz, e, vx, vy, vz, tof);
   
    particle->SetPolarisation(polx, poly, polz);
    particle->SetWeight(weight);
    particle->SetUniqueID(mech);

    // Updating daughters info for mother particle
    if(parent>=0) { // For all particles but for the primary
        TParticle *partMother = (TParticle*) (fParticles->UncheckedAt(parent));	   
        partMother->SetLastDaughter(trackId); // Updating last daugther track index
        if (partMother->GetFirstDaughter()==-1) partMother->SetFirstDaughter(trackId);
    }
    if (parent<0) fNPrimary++;
    if (toBeDone) fStack.push(particle);
    ntr = GetNtrack() - 1;  
    if(ntr>=fTracksLim) { 
        if(ntr==fTracksLim) std::cout << "a2mcStack::PushTrack -> Maximum number of tracks to be stored (" << fTracksLim << ") reached" << std::endl;
        return; 
    }

    // Adding new track, starting from "particle"
    TClonesArray& ref = *fTracks;
    Int_t size = ref.GetEntriesFast();
    new(ref[size]) a2mcMCTrack(particle);

    // Updating daughters info for mother track
    if(parent>=0) { // For all particles but for the primary
        a2mcMCTrack *trkMother = (a2mcMCTrack*) (fTracks->UncheckedAt(parent));
        trkMother->SetLastDaughter(trackId); // Updating last daugther track index
        if (trkMother->GetFirstDaughter()==-1) trkMother->SetFirstDaughter(trackId);
    }
}

//_____________________________________________________________________________
TParticle* a2mcStack::PopNextTrack(Int_t& itrack)
{
/// Get next particle for tracking from the stack.
///     The popped particle object
/// track  The index of the popped track

  itrack = -1;
  if  (fStack.empty()) return 0;
		      
  TParticle* particle = fStack.top();
  fStack.pop();

  if (!particle) return 0;  
  
  fCurrentTrack = particle->GetSecondMother();
  itrack = fCurrentTrack;
  
  return particle;
}    

//_____________________________________________________________________________
TParticle* a2mcStack::PopPrimaryForTracking(Int_t i)
{
/// Return \em i -th particle in fParticles.
/// The popped primary particle object
/// i  The index of primary particle to be popped

  if (i < 0 || i >= fNPrimary)
    Fatal("GetPrimaryForTracking", "Index out of range"); 
  
  return (TParticle*)fParticles->At(i);
}     

// -----   Public method Register   ----------------------------------------
void a2mcStack::RegisterMCTrack() {
  a2mcRootManager::Instance()->Register("MCTracks", "TClonesArray", &fTracks);
}

//_____________________________________________________________________________
void a2mcStack::Print(Option_t* /*option*/) const 
{
/// Print info for all particles.

  std::cout << "a2mcStack Info  " << std::endl;
  std::cout << "Total number of particles:   " <<  GetNtrack() << std::endl;
  std::cout << "Number of primary particles: " <<  GetNprimary() << std::endl;

  for (Int_t i=0; i<GetNtrack(); i++)
    GetParticle(i)->Print();
}

//_____________________________________________________________________________
void a2mcStack::Reset()
{
/// Delete contained particles, reset particles array and stack
  fCurrentTrack = -1;
  fNPrimary = 0;
  fParticles->Clear();
  fTracks->Clear();
}       

//_____________________________________________________________________________
void  a2mcStack::SetCurrentTrack(Int_t track) 
{
/// Set the current track number to a given value
  fCurrentTrack = track;
}     

//_____________________________________________________________________________
Int_t  a2mcStack::GetNtrack() const 
{
/// The total number of all tracks

  return fParticles->GetEntriesFast();
}  

//_____________________________________________________________________________
Int_t  a2mcStack::GetNprimary() const 
{
/// The total number of primary tracks.

  return fNPrimary;
}  

//_____________________________________________________________________________
TParticle*  a2mcStack::GetCurrentTrack() const 
{
/// The current track particle

  TParticle* current = GetParticle(fCurrentTrack);

  if (!current)    
    Warning("GetCurrentTrack", "Current track not found in the stack");

  return current;
}  

//_____________________________________________________________________________
Int_t  a2mcStack::GetCurrentTrackNumber() const 
{
/// The current track number
  return fCurrentTrack;
}  

//_____________________________________________________________________________
Int_t  a2mcStack::GetCurrentParentTrackNumber() const 
{
/// The current track parent ID.

  TParticle* current = GetCurrentTrack();

  if (current) 
    return current->GetFirstMother();
  else 
    return -1;
}  

//_____________________________________________________________________________
TParticle*  a2mcStack::GetParticle(Int_t id) const
{
/// The \em id -th particle in fParticles
/// id The index of the particle to be returned

  if (id < 0 || id >= fParticles->GetEntriesFast())
    Fatal("GetParticle", "Index out of range"); 
   
  return (TParticle*)fParticles->At(id);
}

//__Get the a2mcMCTrack for the corresponding array ___________________________
a2mcMCTrack*  a2mcStack::GetTrack(Int_t id) const
{
// Returning the pointer to the a2mcMCTrack with index id
  if (id < 0 || id >= fTracks->GetEntriesFast())
    Fatal("GetParticle", "Index out of range"); 
   
  return (a2mcMCTrack*)fTracks->At(id);
}
//_____________________________________________________________________________
a2mcStack* a2mcStack::Instance()
{
  // Instance is then used in a2mcXXSD.cxx to access a2mcStack functions
  return fgInstance;
} 

