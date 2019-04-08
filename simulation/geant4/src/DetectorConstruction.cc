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
// $Id$
//

#include "DetectorConstruction.hh"

#include "CADMesh.hh"
#include "G4VSolid.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UnitsTable.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4SDManager.hh"
#include "TPCSD.hh"
#include "FieldSetup.hh"

#include "G4Region.hh"
#include "G4RegionStore.hh"

#include "MediumMagboltz.hh"
#include "HeedInterfaceModel.hh"
#include "HeedOnlyModel.hh"

#include <fstream>
#include <map>

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(GasModelParameters* gmp): G4VUserDetectorConstruction(), 
  fMagneticField(1.0*tesla),fQuencherFraction(0.3),
  kMat(true), kProto(false), 
  logicAG(0),logicdrift(0),
  fGasPressure(0.96658731*bar), fGasTemperature(293.15*kelvin), drift_gas(0),
  fDriftCell(-4000.,3100.,-99.), fGasModelParameters(gmp),
  fVerboseCAD(false)
{ 
  fDetectorMessenger = new DetectorMessenger(this);
  fpFieldSetup = new FieldSetup();
  G4cout<<"DetectorConstruction::DetectorConstruction()"<<G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ 
  if(fDetectorMessenger) delete fDetectorMessenger;
  if(fpFieldSetup)  delete fpFieldSetup;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{  
  G4cout<<"DetectorConstruction::Construct()"<<G4endl;

  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // GARFIELD++
  //--------------------------------------------------------------------
  ConstructGarfieldGeometry();
  //--------------------------------------------------------------------
 
  G4cout << "DetectorConstruction::Construct() Geant4 geometry" << G4endl;
  G4double TPCrad=fDriftCell.GetCathodeRadius()*cm;
  G4double TPClen=fDriftCell.GetFullLengthZ()*cm;

  G4double TPCin_thick   = 5.75*mm;
  G4double TPCin_radius  = TPCrad - TPCin_thick;

  G4double TPCout_radius = fDriftCell.GetROradius()*cm;
  G4double TPCout_thick  = 4.*mm;
 
  G4double MagnetOutID=124.*cm; 
  G4double MagnetOutOD=125.*cm;

  G4double SolenoidLength = 3.*m;
  G4double MagnetBore=50.*cm;
  G4double MagnetCover=1.*cm;

  G4double MagnetOutL=SolenoidLength-MagnetCover;
  
  G4double MagnetShieldID=109.*cm;
  G4double MagnetShieldOD=110.4*cm;

  G4double MagnetShieldL=280.*cm;
  
  G4double MagnetWindID=50.5*cm;
  G4double MagnetWindOD=51.*cm;

  //  G4double MagnetInOD=109.*cm;
  G4double MagnetInOD=MagnetWindOD+5.*cm;
  G4double MagnetInL=290.*cm;

  G4double world_X = MagnetOutOD + 1.*cm;
  G4double world_Y = world_X;
  G4double world_Z = 4.0*m;
  
  //--------------------------------------------------------------------
  G4Material* air     = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
  G4cout<<"Air: "<<G4BestUnit(air->GetRadlen(),"Length")<<G4endl;

  G4Material* vacuum = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
  G4cout<<"Vacuum: "<<G4BestUnit(vacuum->GetRadlen(),"Length")<<G4endl;

  G4Material* alu_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");
  G4cout<<"Aluminum: "<<G4BestUnit(alu_mat->GetRadlen(),"Length")<<G4endl;
 
  G4Material* cop_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Cu");
  G4cout<<"Copper: "<<G4BestUnit(cop_mat->GetRadlen(),"Length")<<G4endl;
  //--------------------------------------------------------------------

  // ------------------ define gas mixture for TPC ------------------//
  //  BuildDriftMaterial(725.0*Torr+3.*mbar);
  //  BuildDriftMaterial(0.97*bar);
  BuildDriftMaterial();
  // ----------------------------------------------------------------//

  G4Element* C = G4NistManager::Instance()->FindOrBuildElement("C");
  G4Element* O = G4NistManager::Instance()->FindOrBuildElement("O");
  G4Element* H = G4NistManager::Instance()->FindOrBuildElement("H");
  G4Material* TPCmat = new G4Material("Garolite",1.850*g/cm3,3);
  TPCmat->AddElement(C,7);
  TPCmat->AddElement(O,2);
  TPCmat->AddElement(H,8);
  G4cout<<"TPC mat (G10): "<<G4BestUnit(TPCmat->GetRadlen(),"Length")<<G4endl;


  // --------------------- Cryostat Materials ----------------------//
  BuildCryostatMaterials();

  // ------------------ Fill volume from CAD list -------------------//
  G4int number_of_CAD_parts = ReadCADparts();
  G4cout << "DetectorConstruction::Construct() " << number_of_CAD_parts 
	 << " CAD parts found " << G4endl;

  //--------------------------------------------------------------------
  // Option to switch on/off checking of volumes overlaps
  G4bool checkOverlaps = true;

  //--------------------------------------------------------------------

  G4Box* solidWorld = new G4Box("World_sol", 0.5*world_X, 
				0.5*world_Y, 0.5*world_Z);

  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, 
						    air,
						    "World_log");
  
  G4VPhysicalVolume* physWorld = new G4PVPlacement(0,
						   G4ThreeVector(),
						   logicWorld,
						   "World",
						   0,
						   false,
						   0,
						   checkOverlaps);

  //------------------------------ 
  // ALPHA-g
  //------------------------------			 
  G4Tubs* solidAG = new G4Tubs("solidAG",0.,0.5*MagnetBore,0.5*(SolenoidLength+51.*cm),0.*deg,360.*deg);
  //logicAG =  new G4LogicalVolume(solidAG,air,"logicAG");
  logicAG =  new G4LogicalVolume(solidAG,vacuum,"logicAG");
  //------------------------------ 
  new G4PVPlacement(0,G4ThreeVector(),
		    logicAG,"ALPHA-g",logicWorld,false,0,checkOverlaps);
  //------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------

  //------------------------------ 
  // Use CADMesh to build the cryostat from step files
  //------------------------------
  //BuildCryostat( checkOverlaps );
  BuildCryostat( false );
  //------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------

  // ------------------------------ 
  // Backcock Magnet
  //------------------------------
  G4Tubs* solidBabcock = new G4Tubs("solidBabcock",
				    0.5*MagnetBore,0.5*MagnetOutOD,
				    0.5*SolenoidLength+MagnetCover,0.*deg,360.*deg);
  G4LogicalVolume* logicBabcock = new G4LogicalVolume(solidBabcock, 
						      vacuum, "logicBabcock");
  if( kMat )  
    new G4PVPlacement(0,G4ThreeVector(),
		      logicBabcock,"Babcock",logicWorld,false,0,checkOverlaps);

  //------------------------------ 
  // Magnet Outer
  //------------------------------
  G4Tubs* solidMagnetOut = new G4Tubs("solidMagnetOut",
				      0.5*MagnetOutID,0.5*MagnetOutOD,
				      0.5*MagnetOutL,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetOut = new G4LogicalVolume(solidMagnetOut, 
							alu_mat, "logicMagnetOut");
  if( kMat )  
    new G4PVPlacement(0,G4ThreeVector(),
		      logicMagnetOut,"MagnetOut",logicBabcock,false,0,checkOverlaps);

  //------------------------------ 
  // Magnet Radiation Shield
  //------------------------------ 
  G4Tubs* solidMagnetShield = new G4Tubs("solidMagShield",
					 0.5*MagnetShieldID,0.5*MagnetShieldOD,
					 0.5*MagnetShieldL,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetShield = new G4LogicalVolume(solidMagnetShield,
							   cop_mat,"logicMagShield");
  if( kMat )
    new G4PVPlacement(0,G4ThreeVector(),
		      logicMagnetShield,"MagnetShield",
		      logicBabcock,false,0,checkOverlaps);   
  
  //------------------------------ 
  // Magnet Inner
  //------------------------------
  G4Tubs* solidMagnetIn = new G4Tubs("solidMagnetIn",0.5*MagnetBore,0.5*MagnetInOD,0.5*MagnetInL,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetIn = new G4LogicalVolume(solidMagnetIn, materials["lHe"],"logicMagnetIn");
 
  if( kMat )
    new G4PVPlacement(0,G4ThreeVector(),
		      logicMagnetIn,"MagnetIn",logicBabcock,false,0,checkOverlaps);
  
  //------------------------------ 
  // Magnet Windings
  //------------------------------
  G4Tubs* solidMagnetWinding = new G4Tubs("solidMagnetWinding",
					  0.5*MagnetWindID,0.5*MagnetWindOD,
					  0.5*MagnetInL,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetWinding = new G4LogicalVolume(solidMagnetWinding,
							    cop_mat,"logicMagnetWinding");
  if( kMat )
    new G4PVPlacement(0,G4ThreeVector(),
		      logicMagnetWinding,"MagnetWinding",
		      logicMagnetIn,false,0,checkOverlaps);
  

  //------------------------------ 
  // Magnet Side Covers
  //------------------------------
  G4Tubs* solidMagnetCover = new G4Tubs("solidMagnetCover",0.5*MagnetBore,0.5*MagnetOutOD,0.5*MagnetCover,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetCover = new G4LogicalVolume(solidMagnetCover,
							  alu_mat,"logicMagnetCovers");
  if( kMat )
    {
      new G4PVPlacement(0,G4ThreeVector(0.,0.,-0.5*SolenoidLength-0.5*MagnetCover),
			logicMagnetCover,"MagnetCover01",
			logicBabcock,false,0,checkOverlaps);
      new G4PVPlacement(0,G4ThreeVector(0.,0.,0.5*SolenoidLength+0.5*MagnetCover),
			logicMagnetCover,"MagnetCover02",
			logicBabcock,false,1,checkOverlaps);
    }

  //------------------------------------------------------------------------------------------
  
  // outer TPC
  G4Tubs* solidTPCout = new G4Tubs("TPCout_sol", TPCout_radius, 
				   TPCout_radius+TPCout_thick,
				   0.5*TPClen,
				   0.,360.*deg);
      
  G4LogicalVolume* logicTPCout = new G4LogicalVolume(solidTPCout, 
						     TPCmat,
						     "TPCout_log");
               
  new G4PVPlacement(0, G4ThreeVector(), logicTPCout, "outerTPC", logicAG,
  		    false, 0, checkOverlaps);

  //------------------------------ 
  // drift chamber
  //------------------------------ 
  G4Tubs* soliddrift = new G4Tubs("drift_sol", TPCrad, 
  				  TPCout_radius,
  				  0.5*TPClen,
  				  0.,360.*deg);
      
  logicdrift = new G4LogicalVolume(soliddrift, drift_gas, "drift_log");
               
  new G4PVPlacement(0, G4ThreeVector(), logicdrift, "DriftChamber", logicAG,
  		    false, 0, checkOverlaps);

 
  //------------------------------ 
  ConstructAllWires(false); // without checking overlaps
  //ConstructAllWires(true); // with overlaps check
  //------------------------------ 
    
  // inner TPC
  G4Tubs* solidTPCin = new G4Tubs("TPCin_sol", TPCin_radius, 
  				  TPCrad,
  				  0.5*TPClen,
  				  0.,360.*deg);
      
  G4LogicalVolume* logicTPCin = new G4LogicalVolume(solidTPCin, 
  						    TPCmat,
  						    "TPCin_log");
               
  new G4PVPlacement(0, G4ThreeVector(), logicTPCin, "innerTPC", logicAG,
		    false, 0, checkOverlaps);


  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // the DRIFT REGION
  //--------------------------------------------------------------------
  G4Region* driftRegion = new G4Region("DriftRegion");
  driftRegion->AddRootLogicalVolume( logicdrift );
  //--------------------------------------------------------------------

  // Visualization attribute
  logicWorld->SetVisAttributes(G4VisAttributes::Invisible);
  logicBabcock->SetVisAttributes(G4VisAttributes::Invisible);
  logicAG->SetVisAttributes(G4VisAttributes::Invisible);

  G4VisAttributes* DriftVisAtt= new G4VisAttributes(G4Colour::Cyan());
  DriftVisAtt->SetVisibility(true);
  logicdrift->SetVisAttributes(DriftVisAtt);

  G4VisAttributes* MagOutVisAtt=new G4VisAttributes(G4Colour::Green());
  logicMagnetOut->SetVisAttributes(MagOutVisAtt);
  
  G4VisAttributes* MagShlVisAtt= new G4VisAttributes(G4Colour(0.7, 0.4, 0.1)); // brown
  logicMagnetShield->SetVisAttributes(MagShlVisAtt);
  
  G4VisAttributes* MagInVisAtt= new G4VisAttributes(G4Colour::Cyan()); 
  logicMagnetIn ->SetVisAttributes(MagInVisAtt);
  
  G4VisAttributes* MagWindVisAtt= new G4VisAttributes(G4Colour::Yellow()); 
  logicMagnetWinding->SetVisAttributes(MagWindVisAtt);
  
  G4VisAttributes* MagCovVisAtt= new G4VisAttributes(G4Colour::Grey());
  logicMagnetCover->SetVisAttributes(MagCovVisAtt);

  //always return the physical World
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::ConstructSDandField()
{
  //------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------
  // FIELD SETUP
  fpFieldSetup->SetUniformBField(fMagneticField);
  fpFieldSetup->LocalMagneticFieldSetup();
  //------------------------------ 
  // LOCAL UNIFORM magnetic field
  logicAG->SetFieldManager(fpFieldSetup->GetLocalFieldManager(),true);
  G4cout<<"DetectorConstruction::ConstructSDandField() Local Magnetic Field Setup"<<G4endl;


  //--------- Sensitive detector -------------------------------------
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // define TPC as DETECTOR
  //--------------------------------------------------------------------
  TPCSD* theTPCSD = new TPCSD("TPCsense");
  logicdrift->SetSensitiveDetector(theTPCSD);
  if(SDman)
    {
      SDman->AddNewDetector(theTPCSD);
      G4cout<<"DetectorConstruction::ConstructSDandField() TPC is a detector"<<G4endl;
    }
  else
    G4cout<<"Failed to create TPC as a detector"<<G4endl;


  G4Region* driftRegion = G4RegionStore::GetInstance()->GetRegion("DriftRegion");
  if( driftRegion )
    G4cout<<"DetectorConstruction::ConstructSDandField() TPC has a drift region"<<G4endl;

  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // GARFIELD++ G4 interface
  //--------------------------------------------------------------------

  if( fGasModelParameters->GetParticleNamesHeedOnly().size() )
    {
      HeedOnlyModel* HOM = new HeedOnlyModel(fGasModelParameters,
					     "HeedOnlyModel",
					     driftRegion, this, theTPCSD);
      G4cout << "DetectorConstruction::ConstructSDandField()  initializes: " 
		<< HOM->GetName() << G4endl;
    }
  
  if( fGasModelParameters->GetParticleNamesHeedInterface().size() )
    {
      HeedInterfaceModel* HIM = new HeedInterfaceModel(fGasModelParameters,
						       "HeedInterfaceModel",
						       driftRegion, this, theTPCSD);   
      G4cout << "DetectorConstruction::ConstructSDandField() initializes: " 
		<< HIM->GetName() << G4endl; 
    }
  //--------------------------------------------------------------------
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::BuildCryostatMaterials()
{
  // define elements for superconducting magnets
  G4Element* Cu = G4NistManager::Instance()->FindOrBuildElement("Cu");
  G4Element* Nb = G4NistManager::Instance()->FindOrBuildElement("Nb");
  G4Element* Ti = G4NistManager::Instance()->FindOrBuildElement("Ti");

  G4Element* He = G4NistManager::Instance()->FindOrBuildElement("He");
  G4Material* lHe = new G4Material("lHe",0.125*g/cm3,1);
  lHe->AddElement(He,1.);
  G4cout<<"LHe: "<<G4BestUnit(lHe->GetRadlen(),"Length")<<G4endl;

  materials.insert(std::pair<G4String, G4Material*>("lHe", lHe));

  G4Material* Al = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");
  G4Material* Cu_mat  = G4NistManager::Instance()->FindOrBuildMaterial("G4_Cu");
  G4Material* Stainless_steel = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL");

  materials.insert(std::pair<G4String, G4Material*>("G4_Al", Al));
  materials.insert(std::pair<G4String, G4Material*>("G4_Cu", Cu_mat));
  materials.insert(std::pair<G4String, G4Material*>("G4_STAINLESS-STEEL", Stainless_steel));

  // Epoxy
  G4Element* Cl = G4NistManager::Instance()->FindOrBuildElement("Cl");
  G4Element* C  = G4NistManager::Instance()->FindOrBuildElement("C");
  G4Element* H  = G4NistManager::Instance()->FindOrBuildElement("H");
  G4Element* O  = G4NistManager::Instance()->FindOrBuildElement("O");
  G4Material* epoxy = new G4Material("Epoxy", 1.56*g/cm3,4);
  epoxy->AddElement(C,21);
  epoxy->AddElement(H,25);
  epoxy->AddElement(Cl,5);
  epoxy->AddElement(O,5);

  materials.insert(std::pair<G4String, G4Material*>("epoxy", epoxy));

  // Kapton
  G4Element* N = G4NistManager::Instance()->FindOrBuildElement("N");
  G4Material* kapton =  new G4Material("Kapton", 1.42*g/cm3, 4);
  kapton->AddElement(H,0.026362);
  kapton->AddElement(C,0.691133);
  kapton->AddElement(N,0.073270);
  kapton->AddElement(O,0.209235);

  // Coil Materials
  G4Material* NbTi = new G4Material("NbTi", 6.538*g/cm3, 2);
  NbTi->AddElement(Nb,1);
  NbTi->AddElement(Ti,1);

  G4Material* Octupole_layer_7_8_coil_mat = new G4Material("Octupole_layer_7_8_coil_mat", 2.786*g/cm3, 4);
  Octupole_layer_7_8_coil_mat->AddMaterial(NbTi,0.2511);
  Octupole_layer_7_8_coil_mat->AddElement(Cu,0.312);
  Octupole_layer_7_8_coil_mat->AddMaterial(kapton,0.0958);
  Octupole_layer_7_8_coil_mat->AddMaterial(epoxy,0.341);

  materials.insert(std::pair<G4String, G4Material*>("Octupole_layer_7_8_coil_mat", Octupole_layer_7_8_coil_mat));

  G4Material* Octupole_layer_3_4_coil_mat = new G4Material("Octupole_layer_3_4_coil_mat", 2.879*g/cm3,4);
  Octupole_layer_3_4_coil_mat->AddMaterial(NbTi, 0.2634);
  Octupole_layer_3_4_coil_mat->AddElement(Cu, 0.3237);
  Octupole_layer_3_4_coil_mat->AddMaterial(kapton, 0.0996);
  Octupole_layer_3_4_coil_mat->AddMaterial(epoxy, 0.3132);

  materials.insert(std::pair<G4String, G4Material*>("Octupole_layer_3_4_coil_mat", Octupole_layer_3_4_coil_mat));

  G4Material* Octupole_layer_1_2_coil_mat = new G4Material("Octupole_layer_1_2_coil_mat", 2.799*g/cm3,4);
  Octupole_layer_1_2_coil_mat->AddMaterial(NbTi, 0.2546);
  Octupole_layer_1_2_coil_mat->AddElement(Cu, 0.3137);
  Octupole_layer_1_2_coil_mat->AddMaterial(kapton, 0.0969);
  Octupole_layer_1_2_coil_mat->AddMaterial(epoxy, 0.3349);

  materials.insert(std::pair<G4String, G4Material*>("Octupole_layer_1_2_coil_mat", Octupole_layer_1_2_coil_mat));

  G4Material* Octupole_layer_5_6_coil_mat = new G4Material("Octupole_layer_5_6_coil_mat", 2.915*g/cm3,4);
  Octupole_layer_5_6_coil_mat->AddMaterial(NbTi, 0.2669);
  Octupole_layer_5_6_coil_mat->AddElement(Cu, 0.3289);
  Octupole_layer_5_6_coil_mat->AddMaterial(kapton, 0.1013);
  Octupole_layer_5_6_coil_mat->AddMaterial(epoxy, 0.3029);

  materials.insert(std::pair<G4String, G4Material*>("Octupole_layer_5_6_coil_mat", Octupole_layer_5_6_coil_mat));

  G4Material* Analysis_coil_mat = new G4Material("Analysis_coil_mat", 3.334*g/cm3,4);
  Analysis_coil_mat->AddMaterial(NbTi, 0.0373);
  Analysis_coil_mat->AddElement(Cu, 0.6235);
  Analysis_coil_mat->AddMaterial(kapton, 0.1146);
  Analysis_coil_mat->AddMaterial(epoxy, 0.2246);

  materials.insert(std::pair<G4String, G4Material*>("Analysis_coil_mat", Analysis_coil_mat));

  G4Material* Mirror_Capture_coil_mat = new G4Material("Mirror_Capture_coil_mat", 3.6571*g/cm3,4);
  Mirror_Capture_coil_mat->AddMaterial(NbTi, 0.3236);
  Mirror_Capture_coil_mat->AddElement(Cu, 0.3994);
  Mirror_Capture_coil_mat->AddMaterial(kapton, 0.0279);
  Mirror_Capture_coil_mat->AddMaterial(epoxy, 0.2491);

  materials.insert(std::pair<G4String, G4Material*>("Mirror_Capture_coil_mat", Mirror_Capture_coil_mat));

  G4Material* Corrector_layer_5_6_coil_mat = new G4Material("Corrector_layer_5_6_coil_mat", 2.0698*g/cm3,4);
  Corrector_layer_5_6_coil_mat->AddMaterial(NbTi, 0.0158);
  Corrector_layer_5_6_coil_mat->AddElement(Cu, 0.2900);
  Corrector_layer_5_6_coil_mat->AddMaterial(kapton, 0.0535);
  Corrector_layer_5_6_coil_mat->AddMaterial(epoxy, 0.6407);

  materials.insert(std::pair<G4String, G4Material*>("Corrector_layer_5_6_coil_mat", Corrector_layer_5_6_coil_mat));

  G4Material* Corrector_layer_7_8_coil_mat = new G4Material("Corrector_layer_7_8_coil_mat", 2.2389*g/cm3,4);
  Corrector_layer_7_8_coil_mat->AddMaterial(NbTi, 0.0204);
  Corrector_layer_7_8_coil_mat->AddElement(Cu, 0.3562);
  Corrector_layer_7_8_coil_mat->AddMaterial(kapton, 0.0659);
  Corrector_layer_7_8_coil_mat->AddMaterial(epoxy, 0.5574);

  materials.insert(std::pair<G4String, G4Material*>("Corrector_layer_7_8_coil_mat", Corrector_layer_7_8_coil_mat));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4int DetectorConstruction::ReadCADparts()
{
  volume temp_volume;
  int num;
  // file input stream
  G4String fname = G4String(getenv("AGRELEASE"))+"/simulation/common/CAD_Files/cad_part_list.txt";
  std::ifstream f(fname.c_str());
  if( f.is_open() ) 
    {
      G4cout << "DetectorConstruction::ReadCADparts() cad_part_list.txt is open" << G4endl;
      while(1) 
	{
	  f >> num >> temp_volume.name >> temp_volume.material_name >> temp_volume.R 
	    >> temp_volume.G >> temp_volume.B >> temp_volume.alpha;
	  if( !f.good() ) break;
	  temp_volume.material = materials[temp_volume.material_name];
	  if( fVerboseCAD )
	    G4cout << num << " - " << temp_volume.name << " - " << temp_volume.material_name 
		   << " - " << temp_volume.R << " - " << temp_volume.G << " - " 
		   << temp_volume.B << " - " << temp_volume.alpha << G4endl;
	  volumes.insert(std::pair<int, volume>(num, temp_volume));
	}
      f.close();
    } 
  else 
    {
      G4cout << "DetectorConstruction::ReadCADparts() FAIL: cad_part_list.txt not found" << G4endl;
    }
  return G4int(volumes.size());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::BuildCryostat(bool checkOverlaps)
{
  if( !logicAG ) return;

  // CAD files path
  G4String env_path = getenv("AGRELEASE");
  G4String file_path = "/simulation/common/CAD_Files/";
  G4String file_ext = ".stl";

  // Liquid Helium
  G4String filename = env_path + file_path + "lHe" + file_ext;
  CADMesh * lHe_mesh = new CADMesh((char*) filename.c_str());
  G4VSolid* lHe_solid = lHe_mesh->TessellatedMesh();
  G4LogicalVolume* lHe_log = new G4LogicalVolume(lHe_solid, materials["lHe"], "lHe");
  G4RotationMatrix* r = new G4RotationMatrix();
  r->rotateY(90*degree);
  new G4PVPlacement(r, G4ThreeVector(), lHe_log, "lHe", logicAG, false, 0, checkOverlaps);
  lHe_log->SetVisAttributes(G4Colour(0.,0.3,0.7));
  delete lHe_mesh;

  // CAD Cryostat Volumes
  for(uint i = 0; i < volumes.size(); ++i) 
    {
      filename=env_path + file_path + std::to_string(i) + file_ext;
      CADMesh * mesh = new CADMesh((char*) filename.c_str());
      G4VSolid* cad_solid = mesh->TessellatedMesh();
      volumes[i].cad_logical = new G4LogicalVolume(cad_solid, volumes[i].material, volumes[i].name);
      if( kMat )
	{
	  new G4PVPlacement(r,G4ThreeVector(), volumes[i].cad_logical, volumes[i].name, logicAG, false, 0, checkOverlaps);
	  volumes[i].cad_logical->SetVisAttributes(G4Colour(volumes[i].R,volumes[i].G,volumes[i].B,1));
	}
      delete mesh;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::ConstructGarfieldGeometry()
{
  G4cout << "DetectorConstruction::ConstructGarfieldGeometry()" << G4endl;
  fDriftCell.SetPrototype(kProto);
  fDriftCell.SetMagneticField(0.,0.,fMagneticField/tesla); // T

  double vAW = fGasModelParameters->GetVoltageAnode(),
    vCathode = fGasModelParameters->GetVoltageCathode(),
    vFW = fGasModelParameters->GetVoltageField();
  fDriftCell.SetVoltage( vCathode, vAW, vFW );

  G4cout << "DetectorConstruction::ConstructGarfieldGeometry() set the gas file" << G4endl;
  G4String gasFile = fGasModelParameters->GetGasFile();
  G4String ionMobFile = fGasModelParameters->GetIonMobilityFile();

  Garfield::MediumMagboltz* MediumMagboltz = new Garfield::MediumMagboltz;
  MediumMagboltz->SetPressure(fGasPressure/bar*750.06158);
  MediumMagboltz->SetTemperature(fGasTemperature);
  G4cout << gasFile << G4endl;
  const G4String path = getenv("GARFIELD_HOME");
  G4AutoLock lock(&aMutex);
  bool stat;
  if( gasFile != "" )
    {
      stat = MediumMagboltz->LoadGasFile(gasFile.c_str());
      G4cout << "DetectorConstruction::ConstructGarfieldGeometry() GasFile is "
	     << (stat?"ok":"bad") << G4endl;
    }
  if( ionMobFile != "" )
    {
      stat = MediumMagboltz->LoadIonMobility(path + "/Data/" + ionMobFile);
      G4cout << "DetectorConstruction::ConstructGarfieldGeometry() IonMobility is "
	     << (stat?"ok":"bad") << G4endl;
    }
  fDriftCell.SetGas( MediumMagboltz );

  fDriftCell.init();
  G4cout << "DetectorConstruction::ConstructGarfieldGeometry() DONE" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//void DetectorConstruction::BuildDriftMaterial(G4double pressure)
void DetectorConstruction::BuildDriftMaterial()
{
  G4Material* CO2 = G4NistManager::Instance()->FindOrBuildMaterial("G4_CARBON_DIOXIDE");
  G4Material* Ar = G4NistManager::Instance()->FindOrBuildMaterial("G4_Ar");
  drift_gas = new G4Material("ArCO2",0.0035*g/cm3,2,kStateGas,
			     fGasTemperature, 
			     fGasPressure);
  drift_gas->AddMaterial(CO2,fQuencherFraction);
  drift_gas->AddMaterial(Ar,1.-fQuencherFraction);
  G4cout<<"drift gas: "<<G4BestUnit(drift_gas->GetRadlen(),"Length")<<G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::ConstructAllWires(bool checkOverlaps)
{
  if( !logicdrift ) return;
  
  G4double FieldWiresDiam   = fDriftCell.GetDiameterFieldWires()*cm;
  G4int NfieldWires         = fDriftCell.GetNumberOfFieldWires();
  //  G4double AnodeWiresRadPos = fDriftCell.GetAnodeWiresRadius()*cm;
  G4double AnodeWiresDiam   = fDriftCell.GetDiameterAnodeWires()*cm;
  G4int NanodeWires         = fDriftCell.GetNumberOfAnodeWires();

  G4Material* fieldw_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_W");
  G4Material* anodew_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Cu");

  //------------------------------ 
  // field wires
  //------------------------------ 
  G4Tubs* solidfieldw = new G4Tubs("TPCfw_sol",0.,0.5*FieldWiresDiam,
				   fDriftCell.GetHalfLengthZ()*cm,
				   0.,360.*deg);
  G4LogicalVolume* logicfieldw = new G4LogicalVolume(solidfieldw,fieldw_mat,
						     "fieldw_log");

  G4double xfw,yfw;
  G4int cpy=0;
  for(G4int fw_cpy =0; fw_cpy<NfieldWires; ++fw_cpy)
    {
      fDriftCell.GetWirePosition(fw_cpy, xfw, yfw, true);
      new G4PVPlacement(0, G4ThreeVector(xfw,yfw,0.),logicfieldw,"fieldWires",
			logicdrift,false,fw_cpy,checkOverlaps);
      ++cpy;
    }
   
  //------------------------------ 
  // anode wires
  //------------------------------ 
  G4Tubs* solidanodew = new G4Tubs("TPCfw_sol",0.,0.5*AnodeWiresDiam,
				   fDriftCell.GetHalfLengthZ()*cm,
				   0.,360.*deg);
  G4LogicalVolume* logicanodew = new G4LogicalVolume(solidanodew,anodew_mat,
						     "anodew_log");

  G4double xaw,yaw;
  cpy=0;
  for(G4int aw_cpy =0; aw_cpy<NanodeWires; ++aw_cpy)
    {
      fDriftCell.GetAnodePosition(aw_cpy, xaw, yaw, true);
      new G4PVPlacement(0, G4ThreeVector(xaw,yaw,0.),logicanodew,"anodeWires",
			logicdrift,false,aw_cpy,checkOverlaps);
      ++cpy;
    }

  // ------------------------------ 
  // vis attributes for wires
  //------------------------------ 
  G4VisAttributes* FwVisAtt= new G4VisAttributes(G4Colour(1.,0.5,0.));
  //  FwVisAtt->SetVisibility(true);
  FwVisAtt->SetVisibility(false);
  logicfieldw->SetVisAttributes(FwVisAtt);
  G4VisAttributes* AwVisAtt= new G4VisAttributes(G4Colour::Black());
  //  AwVisAtt->SetVisibility(true);
  AwVisAtt->SetVisibility(false);
  logicanodew->SetVisAttributes(AwVisAtt);
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
