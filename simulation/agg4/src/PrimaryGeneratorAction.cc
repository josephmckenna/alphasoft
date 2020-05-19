//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id$
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "PrimaryGeneratorAction.hh"
#include "PrimaryGeneratorMessenger.hh"

#include "SecondaryProducer.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"

#include "RunAction.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4RandomDirection.hh"

#include "CRYSetup.h"
#include "CRYGenerator.h"
#include "CRYParticle.h"
#include "CRYUtils.h"
#include "RNGWrapper.hh"

#include "ScintPadProto.hh"

#include "TClonesArray.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TRandom3.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <string>
#include <time.h>

extern double gMagneticField;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction():fType(0),fGravDir(-1),
						 fZcenter(50.*cm),fZlength(2.*cm),
						 fRunAction(0)
{
  // create annihilation products
  fHbarAnnihilation = new SecondaryProducer();

  // get annihilation position
  // char fname[80];
  // sprintf(fname,"%s/annihilation.dat",getenv("RUN_TPC"));
  // fin.open(fname,std::ios::in);
  fin.open("./annihilation.dat",std::ios::in);
  //G4cout<<"Annihilation position loaded from "<<fname<<G4endl;

  // define a particle gun
  fParticleGun = new G4ParticleGun();

  // Initilize random generator
  rand_gen = new TRandom3();
  rand_gen->SetSeed(time(NULL));

  // Read annihilation positions from file
  for(int i = 0;i < 10000;i ++) {
    anni_pos[i] = std::vector<double>(4);
    temp_pos[i] = std::vector<double>(4);
  }
  
  // Read annihilation positions
  char s[80];
  if(gMagneticField == 0.65) {
    strcpy(s,getenv("AGRELEASE"));
    strcat(s,"/simulation/common/Annihilation_Files/Down/annipos_0.65T_down.csv");
    down_anni_file = s;
    strcpy(s,getenv("AGRELEASE"));
    strcat(s,"/simulation/common/Annihilation_Files/Up/annipos_0.65T_up.csv");
    up_anni_file = s;
  } else if(gMagneticField == 1.0) {
    strcpy(s,getenv("AGRELEASE"));
    strcat(s,"/simulation/common/Annihilation_Files/Down/annipos_1T_down.csv");
    down_anni_file = s;
    strcpy(s,getenv("AGRELEASE"));
    strcat(s,"/simulation/common/Annihilation_Files/Up/annipos_1T_up.csv");
    up_anni_file = s;
  }
  
  std::string opened_file;

  if(fGravDir == -1) {
    anni_file.open(down_anni_file, std::ios::in);
    opened_file = down_anni_file;
    loaded_anni_file = -1;
  } else if(fGravDir == 1) {
    anni_file.open(up_anni_file, std::ios::in);
    opened_file = up_anni_file;
    loaded_anni_file = 1;
  }

  int k = 0;
  if(anni_file.is_open()) {
    G4cout << opened_file + " Opened" << G4endl;
    while(!anni_file.eof()) {
      anni_file >> temp_pos[k][0] >> temp_pos[k][1] >> temp_pos[k][2] >> temp_pos[k][3];
      k++;
    }

    // Filter annihilation positions
    double least = temp_pos[0][3];
    for(int x = 0;x < k;x ++) {
      if(temp_pos[x][3] < least) {
	least = temp_pos[x][3];
      }
    }
    
    num_anni_pos = 0;
    num_elim = 0;
    for(int i = 0;i < k;i ++) {
      if((temp_pos[i][3] - least) < 0.5 && (temp_pos[i][3] - least) > -0.5) {
	num_elim++;
	continue;
      }    
      
      if(temp_pos[i][3] < -680) {
	anni_pos[i-num_elim][0] = temp_pos[i][0];
	anni_pos[i-num_elim][1] = temp_pos[i][1];
	anni_pos[i-num_elim][2] = temp_pos[i][2];
	anni_pos[i-num_elim][3] = (temp_pos[i][3] - 58.55);
	num_anni_pos ++;
      } else {
	anni_pos[i-num_elim][0] = temp_pos[i][0];
	anni_pos[i-num_elim][1] = temp_pos[i][1];
	anni_pos[i-num_elim][2] = temp_pos[i][2];
	anni_pos[i-num_elim][3] = temp_pos[i][3];
	num_anni_pos ++;
      }
    }
    
  } else {
    G4cout << opened_file + " Failed to open" << G4endl;
  }
  anni_file.close();
  
  G4cout << "# Possible Annihilation Positions: " << num_anni_pos << G4endl;
  G4cout << "# Removed Positons: " << num_elim << G4endl;

  // clear temp array
  for(int i = 0;i < 10000;i++) {
    temp_pos[i] = std::vector<double>(4);
  }

  // Read the cry input file
  std::ifstream CRYFile;
  // char CRYname[80];
  // sprintf(CRYname,"%s/cry.file",getenv("RUN_TPC"));
  // CRYFile.open(CRYname,std::ios::in);
  CRYFile.open("./cry.file",std::ios::in);
  char buffer[1000];
  std::string setupString("");
  while ( !CRYFile.getline(buffer,1000).eof())
    {
      setupString.append(buffer);
      setupString.append(" ");
    }
  CRYFile.close();
  char datadir[80];
  sprintf(datadir,"%s",getenv("CRYDATAPATH"));
  // setup CRY generator
  CRYSetup *setup=new CRYSetup(setupString,datadir);
  fCosmicGen = new CRYGenerator(setup);

  // set random number generator
  RNGWrapper<CLHEP::HepRandomEngine>::set(CLHEP::HepRandom::getTheEngine(),&CLHEP::HepRandomEngine::flat);
  setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);
  // create a vector to store the CRY particle properties
  fvect=new std::vector<CRYParticle*>;

  // define Gaussian Random Generator controlled by
  // fZcenter = mean and fZlength = st. dev.
  fRandGaus = new G4RandGauss(CLHEP::HepRandom::getTheEngine(),fZcenter,fZlength);

  //create a messenger for this class
  fMessenger = new PrimaryGeneratorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fHbarAnnihilation;
  delete fParticleGun;
  delete fCosmicGen;

  if (fin.is_open()) fin.close();

  delete fRandGaus;

  delete fMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void PrimaryGeneratorAction::SetRunAction(RunAction* run_action)
{
  fRunAction=run_action;
  fRunAction->GetMCvertexArray()->Clear();
  fRunAction->GetMCpionsArray()->Clear();
  fRunAction->GetMCpi0Array()->Clear();
  fRunAction->GetMCpositronsArray()->Clear();
  fRunAction->GetMCpositronVtxArray()->Clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  //this function is called at the begining of event
  if(!fRunAction)
    {
      G4cerr << "Unable to save MC vertex... quit..." << G4endl;
      return;
    }

  G4double vx,vy,vz,tt; // position of the MC vertex (Hbar annhilation)

  //4-momentum of generated charged pions in MeV
  TClonesArray& mcpicarray = *(fRunAction->GetMCpionsArray());
  mcpicarray.Clear();
  int Npic=0;

  //4-momentum of generated neutral pions in MeV
  TClonesArray& mcpi0array = *(fRunAction->GetMCpi0Array());
  mcpi0array.Clear();
  int Npi0=0;

  //4-momentum of generated positrons in MeV
  TClonesArray& mcpositronsarray = *(fRunAction->GetMCpositronsArray());
  mcpositronsarray.Clear();
  int Npos=0;

  //vtx position of generated positrons
  TClonesArray& mcpositronvtxarray = *(fRunAction->GetMCpositronVtxArray());
  mcpositronvtxarray.Clear();

  G4double E,px,py,pz;
  
  //  G4double Xcentre=1.*m;
  G4double TrapRadius = TPCBase::TPCBaseInstance()->GetTrapRadius()*cm;

  switch(fType)
    {
    case 1: // Berkeley data
      {
	if( fin.is_open() && fin.good() ) fin>>tt>>vx>>vy>>vz;
	else 
	  {
	    G4cout<<"Check annihilation data input"<<G4endl;
	    assert(0);
	  }

	G4PrimaryVertex *vt = new G4PrimaryVertex(vx, vy, vz, tt);
	// produce secondaries
	G4int nSec = fHbarAnnihilation->Produce();
	// loop over the produced secondaries
	for( G4int i=0; i<nSec; ++i )
	  {
	    G4PrimaryParticle *pp = fHbarAnnihilation->GetSecondary(i);
	    if(abs(pp->GetPDGcode())==211)  // it's a charged pion
	      {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpicarray[Npic] ) TLorentzVector(px,py,pz,E);
		++Npic;
	      }
	    vt->SetPrimary(pp);
	  }    
	anEvent->AddPrimaryVertex(vt);
	
	fHbarAnnihilation->ClearSecondaries();
	break;
      }
    case 10:
      {
	// MC vertex
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();
	
	if((fGravDir == 1) && (loaded_anni_file == -1)) {
	  
	  anni_file.open(up_anni_file,std::ios::in);
	  if(anni_file.is_open()) {
	    loaded_anni_file = fGravDir;
	    G4cout << "Loading " + up_anni_file + " Positions" << G4endl;
	    int k = 0;
	    while(!anni_file.eof()) {
	      anni_file >> temp_pos[k][0] >> temp_pos[k][1] >> temp_pos[k][2] >> temp_pos[k][3];
	      k++;
	    }
	    // Filter annihilation positions
	    double least = temp_pos[0][3];
	    for(int x = 0;x < k;x ++) {
	      if(temp_pos[x][3] < least) {
		least = temp_pos[x][3];
	      }
	    }
	    num_anni_pos = 0;
	    num_elim = 0;
	    for(int i = 0;i < k;i ++) {
	      if((temp_pos[i][3] - least) < 0.5 && (temp_pos[i][3] - least) > -0.5) {
		num_elim++;
		continue;
	      }    
	      if(temp_pos[i][3] < -680) {
		anni_pos[i-num_elim][0] = temp_pos[i][0];
		anni_pos[i-num_elim][1] = temp_pos[i][1];
		anni_pos[i-num_elim][2] = temp_pos[i][2];
		anni_pos[i-num_elim][3] = (temp_pos[i][3] - 58.55);
		num_anni_pos ++;
	      } else {
		anni_pos[i-num_elim][0] = temp_pos[i][0];
		anni_pos[i-num_elim][1] = temp_pos[i][1];
		anni_pos[i-num_elim][2] = temp_pos[i][2];
		anni_pos[i-num_elim][3] = temp_pos[i][3];
		num_anni_pos ++;
	      }
	    }
	  } else {
	    G4cout << up_anni_file + " Failed to open" << G4endl;
	  }
	  anni_file.close();
	  G4cout << "# Possible Annihilation Positions: " << num_anni_pos << G4endl;
	  G4cout << "# Removed Positons: " << num_elim << G4endl;
	  for(int i = 0;i < 10000;i++) {
	    temp_pos[i] = std::vector<double>(4);
	  }
	  
	} else if((fGravDir == -1) && (loaded_anni_file == 1)) {
	  
	  anni_file.open(down_anni_file,std::ios::in);
	  if(anni_file.is_open()) {
	    loaded_anni_file = fGravDir;
	    G4cout << "Loading " + down_anni_file + " Positions" << G4endl;
	    int k = 0;
	    while(!anni_file.eof()) {
	      anni_file >> temp_pos[k][0] >> temp_pos[k][1] >> temp_pos[k][2] >> temp_pos[k][3];
	      k++;
	    }
	    // Filter annihilation positions
	    double least = temp_pos[0][3];
	    for(int x = 0;x < k;x ++) {
	      if(temp_pos[x][3] < least) {
		least = temp_pos[x][3];
	      }
	    }
	    num_anni_pos = 0;
	    num_elim = 0;
	    for(int i = 0;i < k;i ++) {
	      if((temp_pos[i][3] - least) < 0.5 && (temp_pos[i][3] - least) > -0.5) {
		num_elim++;
		continue;
	      }    
	      if(temp_pos[i][3] < -680) {
		anni_pos[i-num_elim][0] = temp_pos[i][0];
		anni_pos[i-num_elim][1] = temp_pos[i][1];
		anni_pos[i-num_elim][2] = temp_pos[i][2];
		anni_pos[i-num_elim][3] = (temp_pos[i][3] - 58.55);
		num_anni_pos ++;
	      } else {
		anni_pos[i-num_elim][0] = temp_pos[i][0];
		anni_pos[i-num_elim][1] = temp_pos[i][1];
		anni_pos[i-num_elim][2] = temp_pos[i][2];
		anni_pos[i-num_elim][3] = temp_pos[i][3];
		num_anni_pos ++;
	      }
	    }
	  } else {
	    G4cout << up_anni_file + " Failed to open" << G4endl;
	  }
	  anni_file.close();
	  G4cout << "# Possible Annihilation Positions: " << num_anni_pos << G4endl;
	  G4cout << "# Removed Positons: " << num_elim << G4endl;
	  for(int i = 0;i < 10000;i++) {
	    temp_pos[i] = std::vector<double>(4);
	  }
	}
	
	int rand_num = (int)rand_gen->TRandom::Uniform((num_anni_pos-1));
	tt = anni_pos[rand_num][0];
	vx = anni_pos[rand_num][1];
	vy = anni_pos[rand_num][2];
	vz = anni_pos[rand_num][3];

	G4PrimaryVertex *vt = new G4PrimaryVertex(vx, vy, vz, tt);

	// MC vertex
	new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);

	// produce secondaries
	G4int nSec = fHbarAnnihilation->Produce();
	// loop over the produced secondaries
	for( G4int i=0; i<nSec; ++i )
	  {
	    G4PrimaryParticle *pp = fHbarAnnihilation->GetSecondary(i);
	    if(abs(pp->GetPDGcode())==211)  // it's a charged pion
	      {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpicarray[Npic] ) TLorentzVector(px,py,pz,E);
		++Npic;
	      }
	    vt->SetPrimary(pp);
	  }    
	anEvent->AddPrimaryVertex(vt);

	fRunAction->GetMCinfoTree()->Fill();
	fHbarAnnihilation->ClearSecondaries();
	break;
      }
    case 2: // cosmic ray generator
      {
	fvect->clear();
	fCosmicGen->genEvent(fvect);
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	for ( unsigned j=0; j<fvect->size(); j++)
	  {
	    fParticleGun->SetParticleDefinition(particleTable->FindParticle((*fvect)[j]->PDGid()));
	    fParticleGun->SetParticleEnergy((*fvect)[j]->ke()*MeV);
	    fParticleGun->SetParticlePosition(G4ThreeVector( (*fvect)[j]->x()*m, 
							     (*fvect)[j]->y()*m, 
							     (*fvect)[j]->z()*m + fZcenter ));
	    fParticleGun->SetParticleMomentumDirection(G4ThreeVector( (*fvect)[j]->u(), 
								      (*fvect)[j]->v(), 
								      (*fvect)[j]->w() ));
	    fParticleGun->SetParticleTime((*fvect)[j]->t());
	    
	    E=fParticleGun->GetParticleEnergy();
	    G4double particle_mass = fParticleGun->GetParticleDefinition()->GetPDGMass();
	    G4double p = TMath::Sqrt(E*E-particle_mass*particle_mass);
	    px=p*(fParticleGun->GetParticleMomentumDirection().x());
	    py=p*(fParticleGun->GetParticleMomentumDirection().y());
	    pz=p*(fParticleGun->GetParticleMomentumDirection().z());
	    new( mcpicarray[j] ) TLorentzVector(px/MeV,py/MeV,pz/MeV,E/MeV);
	    fParticleGun->GeneratePrimaryVertex(anEvent);
	    delete (*fvect)[j];
	  }
	fRunAction->GetMCinfoTree()->Fill();
	break;
      }
      // case 20: // cosmic ray generator -- horizontal? NO
      //   {
      // 	fvect->clear();
      // 	fCosmicGen->genEvent(fvect);
      // 	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
      // 	mcvtxarray.Clear();
      // 	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
      // 	for ( unsigned j=0; j<fvect->size(); j++)
      // 	  {
      // 	    fParticleGun->SetParticleDefinition(particleTable->FindParticle((*fvect)[j]->PDGid()));
      // 	    fParticleGun->SetParticleEnergy((*fvect)[j]->ke()*MeV);
      // 	    vx = (*fvect)[j]->x()*m + Xcentre; 
      // 	    vy = (*fvect)[j]->y()*m;
      // 	    vz = (*fvect)[j]->z()*m + fZcenter;
      // 	    fParticleGun->SetParticlePosition(G4ThreeVector( vx, vy, vz ));
      // 	    // MC vertex
      // 	    //	    new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);
      // 	    new(mcvtxarray[j]) TVector3(vx/mm,vy/mm,vz/mm);
      // 	    fParticleGun->SetParticleMomentumDirection(G4ThreeVector( (*fvect)[j]->u(), 
      // 								      (*fvect)[j]->v(), 
      // 								      (*fvect)[j]->w() ));
      // 	    fParticleGun->SetParticleTime((*fvect)[j]->t());
	    
      // 	    E=fParticleGun->GetParticleEnergy();
      // 	    G4double particle_mass = fParticleGun->GetParticleDefinition()->GetPDGMass();
      // 	    G4double p = TMath::Sqrt(E*E-particle_mass*particle_mass);
      // 	    px=p*(fParticleGun->GetParticleMomentumDirection().x());
      // 	    py=p*(fParticleGun->GetParticleMomentumDirection().y());
      // 	    pz=p*(fParticleGun->GetParticleMomentumDirection().z());
      // 	    new( mcpicarray[j] ) TLorentzVector(px/MeV,py/MeV,pz/MeV,E/MeV);
      // 	    fParticleGun->GeneratePrimaryVertex(anEvent);
      // 	    delete (*fvect)[j];
      // 	  }
      // 	fRunAction->GetMCinfoTree()->Fill();
      // 	break;
      //   }
    case 21: // cosmic ray generator -- horizontal
      {
	fvect->clear();
	fCosmicGen->genEvent(fvect);
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	for ( unsigned j=0; j<fvect->size(); j++)
	  {
	    fParticleGun->SetParticleDefinition(particleTable->FindParticle((*fvect)[j]->PDGid()));
	    fParticleGun->SetParticleEnergy((*fvect)[j]->ke()*MeV);
	    vx = (*fvect)[j]->x()*m; 
	    vz = (*fvect)[j]->y()*m;
	    vy = (*fvect)[j]->z()*m + fZcenter;
	    fParticleGun->SetParticlePosition(G4ThreeVector( vx, vy, vz ));
	    // MC vertex
	    //	    new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);
	    new(mcvtxarray[j]) TVector3(vx/mm,vy/mm,vz/mm);
	    fParticleGun->SetParticleMomentumDirection(G4ThreeVector( (*fvect)[j]->u(), 
								      (*fvect)[j]->w(),
								      (*fvect)[j]->v() ));
	    fParticleGun->SetParticleTime((*fvect)[j]->t());
	    
	    E=fParticleGun->GetParticleEnergy();
	    G4double particle_mass = fParticleGun->GetParticleDefinition()->GetPDGMass();
	    G4double p = TMath::Sqrt(E*E-particle_mass*particle_mass);
	    px=p*(fParticleGun->GetParticleMomentumDirection().x());
	    py=p*(fParticleGun->GetParticleMomentumDirection().y());
	    pz=p*(fParticleGun->GetParticleMomentumDirection().z());
	    new( mcpicarray[j] ) TLorentzVector(px/MeV,py/MeV,pz/MeV,E/MeV);
	    fParticleGun->GeneratePrimaryVertex(anEvent);
	    delete (*fvect)[j];
	  }
	fRunAction->GetMCinfoTree()->Fill();
	break;
      }
    case 22: // cosmic ray generator -- horizontal with Scott's scintillators
      {
	fvect->clear();
	fCosmicGen->genEvent(fvect);
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	double dirx, diry, dirz;
	bool inter = false;
	for ( unsigned j=0; j<fvect->size(); j++)
	  {
	    vx = (*fvect)[j]->x()*m; 
	    vz = (*fvect)[j]->y()*m;
	    vy = (*fvect)[j]->z()*m + fZcenter;
	    tt = (*fvect)[j]->t();
	    TVector3 r0(vx/cm,vy/cm,vz/cm);
	    dirx = (*fvect)[j]->u();
	    diry = (*fvect)[j]->w();
	    dirz = (*fvect)[j]->v();
	    TVector3 slope(dirx, diry, dirz);
	    //TVector3 dir = u.Unit();
	    // ScintPadProto pads( r0, slope );
	    // if( pads.Intersection() )
	    //   {
		inter = true;
		fParticleGun->SetParticleDefinition( particleTable->FindParticle((*fvect)[j]->PDGid()) );
		fParticleGun->SetParticleEnergy( (*fvect)[j]->ke()*MeV );
		fParticleGun->SetParticlePosition(G4ThreeVector( vx, vy, vz ));
		// MC vertex
		//	    new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);
		new(mcvtxarray[j]) TVector3(vx/mm,vy/mm,vz/mm);
		// fParticleGun->SetParticleMomentumDirection(G4ThreeVector( (*fvect)[j]->u(), 
		// 							      (*fvect)[j]->w(),
		// 							      (*fvect)[j]->v() ));
		fParticleGun->SetParticleMomentumDirection( G4ThreeVector( dirx, diry, dirz ) );
		fParticleGun->SetParticleTime( tt );
	    
		E=fParticleGun->GetParticleEnergy();
		G4double particle_mass = fParticleGun->GetParticleDefinition()->GetPDGMass();
		G4double p = TMath::Sqrt(E*E-particle_mass*particle_mass);
		px=p*(fParticleGun->GetParticleMomentumDirection().x());
		py=p*(fParticleGun->GetParticleMomentumDirection().y());
		pz=p*(fParticleGun->GetParticleMomentumDirection().z());
		new( mcpicarray[j] ) TLorentzVector(px/MeV,py/MeV,pz/MeV,E/MeV);
		fParticleGun->GeneratePrimaryVertex(anEvent);
		//	      }
	    delete (*fvect)[j];
	  }
	if( inter )
	  fRunAction->GetMCinfoTree()->Fill();
	break;
      }
    case 3: // "leak" from the end of the octupole (no mirror coils)
      {
	// MC vertex
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();

	G4double theta  = G4UniformRand()*twopi;
	vx     = TrapRadius*cos(theta);
	vy     = TrapRadius*sin(theta);
	G4double ztemp = fRandGaus->fire(fZcenter,fZlength), zz;
	if( ztemp>0. && ztemp<fZcenter )
	  {
	    G4double dd = fZcenter - ztemp;
	    zz = fZcenter + dd;
	  }
	else if( ztemp<0. && ztemp>fZcenter )
	  {
	    G4double dd = ztemp - fZcenter;
	    zz = fZcenter - dd;
	  }
	else
	  zz = ztemp;
	vz     = zz;
	tt     = 0.;

	// MC vertex
	new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);
	
	G4PrimaryVertex *vt = new G4PrimaryVertex(vx, vy, vz, tt);
	// produce secondaries
	G4int nSec = fHbarAnnihilation->Produce();
	// loop over the produced secondaries
	for( G4int i=0; i<nSec; ++i )
	  {
	    G4PrimaryParticle *pp = fHbarAnnihilation->GetSecondary(i);
	    if(abs(pp->GetPDGcode())==211)  // it's a charged pion
	      {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpicarray[Npic] ) TLorentzVector(px,py,pz,E);
		++Npic;
	      }
	    vt->SetPrimary(pp);
	  }

	anEvent->AddPrimaryVertex(vt);

	fRunAction->GetMCinfoTree()->Fill();

	fHbarAnnihilation->ClearSecondaries();
	break;
      }
    case 4: //annihilation on axis
      {
	// MC vertex
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();

	vx = vy = 0.0;
	vz     = (G4UniformRand()-0.5)*fZlength + fZcenter;
	tt     = 0.;

	// MC vertex
	new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);
	
	G4PrimaryVertex *vt = new G4PrimaryVertex(vx, vy, vz, tt);
	// produce secondaries
	G4int nSec = fHbarAnnihilation->Produce();
	// loop over the produced secondaries
	for( G4int i=0; i<nSec; ++i )
	  {
	    G4PrimaryParticle *pp = fHbarAnnihilation->GetSecondary(i);
	    if(abs(pp->GetPDGcode())==211)  // it's a charged pion
	      {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpicarray[Npic] ) TLorentzVector(px,py,pz,E);
		++Npic;
	      }
	    else if(pp->GetPDGcode()==111)  // it's a neutral pion
              {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpi0array[Npi0] ) TLorentzVector(px,py,pz,E);
		++Npi0;
              }
            else if(pp->GetPDGcode()==-11)  // positron
              {
                E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
                new( mcpositronsarray[Npos] )  TLorentzVector(px,py,pz,E);
              }
	    vt->SetPrimary(pp);
	  }

	anEvent->AddPrimaryVertex(vt);

	fRunAction->GetMCinfoTree()->Fill();

	fHbarAnnihilation->ClearSecondaries();
	break;
      }
    case 5: // annihilation on residual gas
      {
	// MC vertex
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();

	G4double theta  = G4UniformRand()*twopi,
	  radius = G4UniformRand()*TrapRadius*TrapRadius;
	vx     = sqrt(radius)*cos(theta);
	vy     = sqrt(radius)*sin(theta); 
	vz     = (G4UniformRand()-0.5)*fZlength + fZcenter;
	tt     = 0.;

	// MC vertex
	new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);
	
	G4PrimaryVertex *vt = new G4PrimaryVertex(vx, vy, vz, tt);
	// produce secondaries
	G4int nSec = fHbarAnnihilation->Produce();
	// loop over the produced secondaries
	for( G4int i=0; i<nSec; ++i )
	  {
	    G4PrimaryParticle *pp = fHbarAnnihilation->GetSecondary(i);
	    if(abs(pp->GetPDGcode())==211)  // it's a charged pion
	      {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpicarray[Npic] ) TLorentzVector(px,py,pz,E);
		++Npic;
	      }
	    vt->SetPrimary(pp);
	  }

	anEvent->AddPrimaryVertex(vt);

	fRunAction->GetMCinfoTree()->Fill();
	
	fHbarAnnihilation->ClearSecondaries();
	break;
      }
    case 6: // test: single track at fixed location
      {
	// MC vertex
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();
	vx = TrapRadius; vy = 0.;
	vz = fZcenter;
	tt = 0.;
	pz = 300.*MeV/sqrt(2.0);
	//	pz = 150.*MeV/sqrt(2.0);
	px = pz; py = 0.;
	if( anEvent->GetEventID() > 15 && G4UniformRand() > 0.5 )
	  pz*=-1.;
	switch( anEvent->GetEventID()%4 )
	  {
	  case 0:
	    vx = TrapRadius; vy = 0.;
	    px = abs(pz); py = 0.;
	    break;
	  case 1:
	    vx = 0.; vy = TrapRadius;
	    px = 0.; py = abs(pz);
	    break;
	  case 2:
	    vx = -TrapRadius; vy = 0.;
	    px = -abs(pz); py = 0.;
	    break;
	  case 3:
	    vx = 0.; vy = -TrapRadius;
	    px = 0.; py = -abs(pz);
	    break;
	  }

	new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);

	G4PrimaryVertex *vt = new G4PrimaryVertex(vx, vy, vz, tt);

	G4int pdgc=22;
	if( (anEvent->GetEventID()%2) == 0 )
	  pdgc = 211;
	else
	  pdgc = -211;
	
	// a 300 MeV pion
	G4PrimaryParticle *pp = new G4PrimaryParticle(pdgc,px,py,pz);
	vt->SetPrimary(pp);
	anEvent->AddPrimaryVertex(vt);

	E = pp->GetTotalEnergy();
	new( mcpicarray[0] ) TLorentzVector(px/MeV,py/MeV,pz/MeV,E/MeV);
	
	fRunAction->GetMCinfoTree()->Fill();
	break;
      }
    default:
      {
	// MC vertex
	TClonesArray& mcvtxarray = *(fRunAction->GetMCvertexArray());
	mcvtxarray.Clear();

	G4double theta  = G4UniformRand()*twopi;
	vx     = TrapRadius*cos(theta);
	vy     = TrapRadius*sin(theta); 
	vz     = (G4UniformRand()-0.5)*fZlength + fZcenter;
	tt     = 0.;

	// MC vertex
	new(mcvtxarray[anEvent->GetEventID()]) TVector3(vx/mm,vy/mm,vz/mm);
	
	G4PrimaryVertex *vt = new G4PrimaryVertex(vx, vy, vz, tt);
	// produce secondaries
	G4int nSec = fHbarAnnihilation->Produce();
	// loop over the produced secondaries
	for( G4int i=0; i<nSec; ++i )
	  {
	    G4PrimaryParticle *pp = fHbarAnnihilation->GetSecondary(i);
	    if(abs(pp->GetPDGcode())==211)  // it's a charged pion
	      {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpicarray[Npic] ) TLorentzVector(px,py,pz,E);
		++Npic;
	      }
	    else if(pp->GetPDGcode()==111)  // it's a neutral pion
              {
		E = pp->GetTotalEnergy()/MeV;
		px = pp->GetPx()/MeV;
		py = pp->GetPy()/MeV;
		pz = pp->GetPz()/MeV;
		new( mcpi0array[Npi0] ) TLorentzVector(px,py,pz,E);
		++Npi0;
              }
	    vt->SetPrimary(pp);
	  }

	anEvent->AddPrimaryVertex(vt);

	fRunAction->GetMCinfoTree()->Fill();

	fHbarAnnihilation->ClearSecondaries();
	break;
      }
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

