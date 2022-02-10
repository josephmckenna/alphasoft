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

#include "G4ExceptionSeverity.hh"

#include "CADMesh.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4VSolid.hh"
#include "G4Tubs.hh"
#include "G4Trd.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UnitsTable.hh"
// Annihilation position check
#include "G4Orb.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4SDManager.hh"
#include "TPCSD.hh"
#include "ScintBarSD.hh"
#include "G4SDParticleFilter.hh"
#include "G4SDChargedFilter.hh"

#include "G4Region.hh"
#include "G4RegionStore.hh"

#include "FieldSetup.hh"
#include "DetectorMessenger.hh"

#include "TPCBase.hh"

#include <TMath.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <array>

extern double gMagneticField;

extern double gBarRadius; // mm
extern double gBarLength;

extern bool kMat;
extern bool kProto;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
  : G4VUserDetectorConstruction(), fDriftGas(0),// fpFieldSetup(0)
    fQuencherFraction(0.1)
{ 
  fTPCrad=TPCBase::TPCBaseInstance()->GetCathodeRadius()*cm;
  fTPCthi=TPCBase::TPCBaseInstance()->GetROradius()*cm-fTPCrad;
  fTPClen=TPCBase::TPCBaseInstance()->GetFullLengthZ()*cm;

  G4RegionStore::GetInstance()->FindOrCreateRegion("DriftRegion");  

  fpFieldSetup = new FieldSetup();
  fpFieldSetup->SetUniformBField(gMagneticField);
  //  fpFieldSetup->UpdateFieldSetup();
  fpFieldSetup->LocalMagneticFieldSetup();

  fpDetectorMessenger = new DetectorMessenger(this);   
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ 
  if(fpFieldSetup)        delete fpFieldSetup;
  if(fpDetectorMessenger) delete fpDetectorMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{  
  G4double TPCin_thick   = 5.75*mm;
  G4double TPCin_radius  = fTPCrad - TPCin_thick;

  G4double TPCout_radius = TPCBase::TPCBaseInstance()->GetROradius()*cm;
  G4double TPCout_thick  = 4.*mm;

  G4double FieldWiresDiam   = TPCBase::TPCBaseInstance()->GetDiameterFieldWires()*cm;
  G4double NfieldWires      = TPCBase::TPCBaseInstance()->GetNumberOfFieldWires();
  G4double AnodeWiresRadPos = TPCBase::TPCBaseInstance()->GetAnodeWiresRadius()*cm;
  G4double AnodeWiresDiam   = TPCBase::TPCBaseInstance()->GetDiameterAnodeWires()*cm;
  G4double NanodeWires      = TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires();

  G4int    Nbars = 64;
  G4double barLength = gBarLength;
  G4double barThick = 20.0*mm;
  G4double barRadPos1 = 223.0*mm;
  G4double barRadPos2 = barRadPos1 + barThick;
  gBarRadius = 0.5*(barRadPos1+barRadPos2)/mm;

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

  G4double MagnetInOD=109.*cm;
  G4double MagnetInL=290.*cm;

  G4double MagOffset=-12.*cm;

  G4double world_X = 10.1*m;
  G4double world_Y = world_X;
  G4double world_Z = 4.0*m;

  G4RotationMatrix* r = new G4RotationMatrix();
  r->rotateY(90*degree);

  // CAD files path
#ifdef CAD_FILE_PATH
  //CMakeBuild installs the CAD files from a tar.gz, then passes the compiler definition CAD_FILE_PATH
  G4String env_path = "";
  G4String file_path = CAD_FILE_PATH;
  std::string file_ext = "_ASCII.stl";
#else
  G4String env_path = getenv("AGRELEASE");
  G4String file_path = "/simulation/common/CAD_Files/";
  std::string file_ext = ".stl";
#endif


  // Maps
  struct volume {
    std::string name;
    std::string material_name;
    G4LogicalVolume* cad_logical;
    G4Material*  material;
    double R;
    double G;
    double B;
    double alpha;
  };
  
  volume temp_volume;
  std::string file_line;
  std::map<int, volume> volumes;
  
  std::map<std::string, G4Material*> materials;

  // file input stream
  std::ifstream f;

  //--------------------------------------------------------------------
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  G4Material* air      = nist->FindOrBuildMaterial("G4_AIR");
  G4cout<<"air: "<<G4BestUnit(air->GetRadlen(),"Length")<<G4endl;
  G4Material* elec_mat = nist->FindOrBuildMaterial("G4_Al");
  G4cout<<"Aluminum: "<<G4BestUnit(elec_mat->GetRadlen(),"Length")<<G4endl;

  G4Material* shld_mat = nist->FindOrBuildMaterial("G4_Cu");
  G4cout<<"Copper: "<<G4BestUnit(shld_mat->GetRadlen(),"Length")<<G4endl;

  G4Material* fieldw_mat = nist->FindOrBuildMaterial("G4_W");
  G4Material* anodew_mat = nist->FindOrBuildMaterial("G4_Cu");

  G4Element* He = nist->FindOrBuildElement("He");
  G4Material* lHe = new G4Material("lHe",0.125*g/cm3,1);
  lHe->AddElement(He,1.);
  G4cout<<"LHe: "<<G4BestUnit(lHe->GetRadlen(),"Length")<<G4endl;

  materials.insert(std::pair<std::string, G4Material*>("lHe", lHe));

  // define elements for superconducting magnets
  G4Element* Cu = nist->FindOrBuildElement("Cu");
  G4Element* Nb = nist->FindOrBuildElement("Nb");
  G4Element* Ti = nist->FindOrBuildElement("Ti");

  G4Material* mag_mat = new G4Material("CuNbTi",5.6*g/cm3,3); // not used
  mag_mat->AddElement(Cu,0.29);
  mag_mat->AddElement(Nb,0.47);
  mag_mat->AddElement(Ti,0.24);
  G4cout<<"magnets: "<<G4BestUnit(mag_mat->GetRadlen(),"Length")<<G4endl;

  // define gas mixture for TPC
  G4Material* CO2 = nist->FindOrBuildMaterial("G4_CARBON_DIOXIDE");
  //  G4cout<<"CO2: "<<G4BestUnit(CO2->GetRadlen(),"Length")<<G4endl;
  G4Material* Ar = nist->FindOrBuildMaterial("G4_Ar");
  //  G4cout<<"argon: "<<G4BestUnit(Ar->GetRadlen(),"Length")<<G4endl;
  G4Material* drift_gas = new G4Material("ArCO2",0.0035*g/cm3,2,kStateGas,
					 STP_Temperature,STP_Pressure);
  drift_gas->AddMaterial(CO2,fQuencherFraction);
  drift_gas->AddMaterial(Ar,1.-fQuencherFraction);
  fDriftGas=drift_gas;
  G4cout<<"drift gas Ar-CO2 "
	<<(1.-fQuencherFraction)*100.<<":"<<fQuencherFraction*100.
	<<" : "<<G4BestUnit(drift_gas->GetRadlen(),"Length")<<G4endl;

  //  G4Material* TPCmat = nist->FindOrBuildMaterial("G4_POLYCARBONATE");
  G4Element* C = nist->FindOrBuildElement("C");
  G4Element* O = nist->FindOrBuildElement("O");
  G4Element* H = nist->FindOrBuildElement("H");
  G4Material* TPCmat = new G4Material("Garolite",1.850*g/cm3,3);
  TPCmat->AddElement(C,7);
  TPCmat->AddElement(O,2);
  TPCmat->AddElement(H,8);
  G4cout<<"TPC mat (G10): "<<G4BestUnit(TPCmat->GetRadlen(),"Length")<<G4endl;

  G4Material* Scint_mat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");



  // --------------------- Cryostat Materials ----------------------//
  G4Material* Al = nist->FindOrBuildMaterial("G4_Al");
  G4Material* Cu_mat  = nist->FindOrBuildMaterial("G4_Cu");
  G4Material* Stainless_steel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");

  materials.insert(std::pair<std::string, G4Material*>("G4_Al", Al));
  materials.insert(std::pair<std::string, G4Material*>("G4_Cu", Cu_mat));
  materials.insert(std::pair<std::string, G4Material*>("G4_STAINLESS-STEEL", Stainless_steel));

  // Epoxy
  G4Element* Cl =  nist->FindOrBuildElement("Cl");
  G4Material* epoxy = new G4Material("Epoxy", 1.56*g/cm3,4);
  epoxy->AddElement(C,21);
  epoxy->AddElement(H,25);
  epoxy->AddElement(Cl,5);
  epoxy->AddElement(O,5);

  materials.insert(std::pair<std::string, G4Material*>("epoxy", epoxy));

  // Kapton
  G4Element* N = nist->FindOrBuildElement("N");
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

  materials.insert(std::pair<std::string, G4Material*>("Octupole_layer_7_8_coil_mat", Octupole_layer_7_8_coil_mat));

  G4Material* Octupole_layer_3_4_coil_mat = new G4Material("Octupole_layer_3_4_coil_mat", 2.879*g/cm3,4);
  Octupole_layer_3_4_coil_mat->AddMaterial(NbTi, 0.2634);
  Octupole_layer_3_4_coil_mat->AddElement(Cu, 0.3237);
  Octupole_layer_3_4_coil_mat->AddMaterial(kapton, 0.0996);
  Octupole_layer_3_4_coil_mat->AddMaterial(epoxy, 0.3132);

  materials.insert(std::pair<std::string, G4Material*>("Octupole_layer_3_4_coil_mat", Octupole_layer_3_4_coil_mat));

  G4Material* Octupole_layer_1_2_coil_mat = new G4Material("Octupole_layer_1_2_coil_mat", 2.799*g/cm3,4);
  Octupole_layer_1_2_coil_mat->AddMaterial(NbTi, 0.2546);
  Octupole_layer_1_2_coil_mat->AddElement(Cu, 0.3137);
  Octupole_layer_1_2_coil_mat->AddMaterial(kapton, 0.0969);
  Octupole_layer_1_2_coil_mat->AddMaterial(epoxy, 0.3349);

  materials.insert(std::pair<std::string, G4Material*>("Octupole_layer_1_2_coil_mat", Octupole_layer_1_2_coil_mat));

  G4Material* Octupole_layer_5_6_coil_mat = new G4Material("Octupole_layer_5_6_coil_mat", 2.915*g/cm3,4);
  Octupole_layer_5_6_coil_mat->AddMaterial(NbTi, 0.2669);
  Octupole_layer_5_6_coil_mat->AddElement(Cu, 0.3289);
  Octupole_layer_5_6_coil_mat->AddMaterial(kapton, 0.1013);
  Octupole_layer_5_6_coil_mat->AddMaterial(epoxy, 0.3029);

  materials.insert(std::pair<std::string, G4Material*>("Octupole_layer_5_6_coil_mat", Octupole_layer_5_6_coil_mat));

  G4Material* Analysis_coil_mat = new G4Material("Analysis_coil_mat", 3.334*g/cm3,4);
  Analysis_coil_mat->AddMaterial(NbTi, 0.0373);
  Analysis_coil_mat->AddElement(Cu, 0.6235);
  Analysis_coil_mat->AddMaterial(kapton, 0.1146);
  Analysis_coil_mat->AddMaterial(epoxy, 0.2246);

  materials.insert(std::pair<std::string, G4Material*>("Analysis_coil_mat", Analysis_coil_mat));

  G4Material* Mirror_Capture_coil_mat = new G4Material("Mirror_Capture_coil_mat", 3.6571*g/cm3,4);
  Mirror_Capture_coil_mat->AddMaterial(NbTi, 0.3236);
  Mirror_Capture_coil_mat->AddElement(Cu, 0.3994);
  Mirror_Capture_coil_mat->AddMaterial(kapton, 0.0279);
  Mirror_Capture_coil_mat->AddMaterial(epoxy, 0.2491);

  materials.insert(std::pair<std::string, G4Material*>("Mirror_Capture_coil_mat", Mirror_Capture_coil_mat));

  G4Material* Corrector_layer_5_6_coil_mat = new G4Material("Corrector_layer_5_6_coil_mat", 2.0698*g/cm3,4);
  Corrector_layer_5_6_coil_mat->AddMaterial(NbTi, 0.0158);
  Corrector_layer_5_6_coil_mat->AddElement(Cu, 0.2900);
  Corrector_layer_5_6_coil_mat->AddMaterial(kapton, 0.0535);
  Corrector_layer_5_6_coil_mat->AddMaterial(epoxy, 0.6407);

  materials.insert(std::pair<std::string, G4Material*>("Corrector_layer_5_6_coil_mat", Corrector_layer_5_6_coil_mat));

  G4Material* Corrector_layer_7_8_coil_mat = new G4Material("Corrector_layer_7_8_coil_mat", 2.2389*g/cm3,4);
  Corrector_layer_7_8_coil_mat->AddMaterial(NbTi, 0.0204);
  Corrector_layer_7_8_coil_mat->AddElement(Cu, 0.3562);
  Corrector_layer_7_8_coil_mat->AddMaterial(kapton, 0.0659);
  Corrector_layer_7_8_coil_mat->AddMaterial(epoxy, 0.5574);

  materials.insert(std::pair<std::string, G4Material*>("Corrector_layer_7_8_coil_mat", Corrector_layer_7_8_coil_mat));

  // Fill volumes info to map
  int num;
  int k = 0;
  f.open(env_path + file_path + "cad_part_list.txt");
  if(f.is_open()) {
    G4cout << env_path + file_path + "cad_part_list.txt" + " Opened" << G4endl;
    while(!f.eof()) {
      f >> num >> temp_volume.name >> temp_volume.material_name >> temp_volume.R 
	>> temp_volume.G >> temp_volume.B >> temp_volume.alpha;
      temp_volume.material = materials[temp_volume.material_name];
      G4cout << num << " - " << temp_volume.name << " - " << temp_volume.material_name 
	     << " - " << temp_volume.R << " - " << temp_volume.G << " - " 
	     << temp_volume.B << " - " << temp_volume.alpha << G4endl;
      volumes.insert(std::pair<int, volume>(num, temp_volume));
      k++;
      if(k == 41) break;
    }
  } else {
    G4cout << env_path + file_path + "cad_part_list.txt" + " Failed to open" << G4endl;
  }
  f.close();

  //--------------------------------------------------------------------
  // Option to switch on/off checking of volumes overlaps
  G4bool checkOverlaps = true;

  //--------------------------------------------------------------------

  G4Box* solidWorld = new G4Box("World_sol", 0.5*world_X, 
				0.5*world_Y, 0.5*world_Z);

  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, 
						    air,
						    //vacuum,
						    "World_log");
  
  G4VPhysicalVolume* physWorld = new G4PVPlacement(0,
						   G4ThreeVector(),
						   logicWorld,
						   "World",
						   0,
						   false,
						   0,
						   checkOverlaps);
  //------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------

  //------------------------------ 
  // ALPHA-g
  //------------------------------			 
  // G4Tubs* solidAG = new G4Tubs("solidAG",0.,0.5*MagnetOutOD,0.5*SolenoidLength,0.*deg,360.*deg);
  G4Tubs* solidAG = new G4Tubs("solidAG",0.,0.5*MagnetBore,0.5*SolenoidLength,0.*deg,360.*deg);
  G4LogicalVolume* logicAG =  new G4LogicalVolume(solidAG,air,"logicAG");
  //------------------------------ 
  // LOCAL UNIFORM magnetic field
  logicAG->SetFieldManager(fpFieldSetup->GetLocalFieldManager(),true);
  //------------------------------ 
  new G4PVPlacement(0,G4ThreeVector(),
		    logicAG,"ALPHA-g",logicWorld,false,0,checkOverlaps);

  //------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------

  // Liquid Helium
  std::string filename=env_path + file_path + "lHe" + file_ext;
  std::shared_ptr<CADMesh::TessellatedMesh>  meshlHe = CADMesh::TessellatedMesh::FromSTL((char*) filename.c_str());
  G4VSolid* lHe_solid = meshlHe->GetSolid();
  G4LogicalVolume* lHe_log = new G4LogicalVolume(lHe_solid, lHe, "lHe");
  new G4PVPlacement(r, G4ThreeVector(), lHe_log, "lHe", logicWorld, false, 0);
  lHe_log->SetVisAttributes(G4Color(1,0,0,0));

  // CAD Cryostat Volumes
  for(int i = 0; i < 41; i ++) 
    {
      filename=env_path + file_path + std::to_string(i) + file_ext;
      std::shared_ptr<CADMesh::TessellatedMesh>  mesh_vol = CADMesh::TessellatedMesh::FromSTL((char*) filename.c_str());
      G4VSolid* cad_solid = mesh_vol->GetSolid();
      volumes[i].cad_logical = new G4LogicalVolume(cad_solid, volumes[i].material, volumes[i].name);
      if( kMat )
	{
	  new G4PVPlacement(r,G4ThreeVector(), volumes[i].cad_logical, volumes[i].name, logicWorld, false, 0);
	  volumes[i].cad_logical->SetVisAttributes(G4Color(volumes[i].R,volumes[i].G,volumes[i].B,1));
	}

    }


  //------------------------------ 
  // Magnet Bore
  //------------------------------
  G4Tubs* Bore = new G4Tubs("MagnetBore",0.,0.5*MagnetBore,0.5*MagnetOutL,0.*deg,360.*deg);  
  
  //------------------------------ 
  // Magnet Outer
  //------------------------------
  G4Tubs* solidMagnetOut = new G4Tubs("solidMagnetOut",
				      0.5*MagnetOutID,0.5*MagnetOutOD,
				      0.5*MagnetOutL,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetOut = new G4LogicalVolume(solidMagnetOut, 
							elec_mat, "logicMagnetOut");
  if( kMat )  
    new G4PVPlacement(0,G4ThreeVector(0.,-1*MagOffset,0.),
		      logicMagnetOut,"MagnetOut",logicWorld,false,0,checkOverlaps);

  //------------------------------ 
  // Magnet Radiation Shield
  //------------------------------ 
  G4Tubs* solidMagnetShield = new G4Tubs("solidMagShield",
					 0.5*MagnetShieldID,0.5*MagnetShieldOD,
					 0.5*MagnetShieldL,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetShield = new G4LogicalVolume(solidMagnetShield,
							   elec_mat,"logicMagShield");
  if( kMat )
    new G4PVPlacement(0,G4ThreeVector(0.,-1*MagOffset,0.),
		      logicMagnetShield,"MagnetShield",
		      logicWorld,false,0,checkOverlaps);   
  
  //------------------------------ 
  // Magnet Inner
  //------------------------------
  G4Tubs* Inner = new G4Tubs("Inner",0.,0.5*MagnetInOD,0.5*MagnetInL,0.*deg,360.*deg);
  G4SubtractionSolid* solidMagnetIn = new G4SubtractionSolid("solidMagnetIn", 
							     Inner, Bore, 0, 
							     G4ThreeVector(0.,MagOffset,0.));
  G4LogicalVolume* logicMagnetIn = new G4LogicalVolume(solidMagnetIn,lHe,"logicMagnetIn");
  //  logicMagnetIn = new G4LogicalVolume(solidMagnetIn,Vacuum,"logicMagnetIn");
 
  if( kMat )
    new G4PVPlacement(0,G4ThreeVector(0.,-1*MagOffset,0.),
		      logicMagnetIn,"MagnetIn",logicWorld,false,0,checkOverlaps);
  
  //------------------------------ 
  // Magnet Windings
  //------------------------------
  G4Tubs* solidMagnetWinding = new G4Tubs("solidMagnetWinding",
					  0.5*MagnetWindID,0.5*MagnetWindOD,
					  0.5*MagnetInL,0.*deg,360.*deg);
  G4LogicalVolume* logicMagnetWinding = new G4LogicalVolume(solidMagnetWinding,
							    shld_mat,"logicMagnetWinding");
  if( kMat )
    new G4PVPlacement(0,G4ThreeVector(0.,MagOffset,0.),
		      logicMagnetWinding,"MagnetWinding",
		      logicMagnetIn,false,0,checkOverlaps);
  

  //------------------------------ 
  // Magnet Side Covers
  //------------------------------
  G4Tubs* Cover = new G4Tubs("Cover",0.,0.5*MagnetOutOD,0.5*MagnetCover,0.*deg,360.*deg);
  G4SubtractionSolid* solidMagnetCover 
    = new G4SubtractionSolid("solidMagnetCover", 
			     Cover, Bore, 0, 
			     G4ThreeVector(0.,MagOffset,0.));
  G4LogicalVolume* logicMagnetCover = new G4LogicalVolume(solidMagnetCover,
							 elec_mat,"logicMagnetCovers");
  if( kMat )
    {
      new G4PVPlacement(0,G4ThreeVector(0.,-1*MagOffset,-0.5*SolenoidLength-0.5*MagnetCover),
			logicMagnetCover,"MagnetCover01",
			logicWorld,false,0,checkOverlaps);
      new G4PVPlacement(0,G4ThreeVector(0.,-1*MagOffset,0.5*SolenoidLength+0.5*MagnetCover),
			logicMagnetCover,"MagnetCover02",
			logicWorld,false,1,checkOverlaps);
    }

  //------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------

//  Scintillating Bars
  G4Trd* solidScintBar = new G4Trd("bar_sol",	barRadPos2*sin(pi/Nbars), barRadPos1*sin(pi/Nbars),
                                    0.5*barLength,  0.5*barLength, 
                                    0.5*barThick*cos(pi/Nbars));


  // G4Tubs* solidScintBars = new G4Tubs("bars_sol", barRadPos1,barRadPos2,
	// 			      0.5*barLength,0.,360.*deg);


  G4LogicalVolume* logicScintBar = new G4LogicalVolume(solidScintBar, 
							Scint_mat,
							"ScintBar_log");
      
  // G4LogicalVolume* logicScintBars = new G4LogicalVolume(solidScintBars, 
	// 						Scint_mat,
	// 						"ScintBars_log");
               
  if( !kProto )
  {
        // new G4PVPlacement(0, G4ThreeVector(), logicScintBars, "ScintBars", logicAG,
		    //   false, 0, checkOverlaps);
    G4double Radius = barRadPos1+0.5*barThick*cos(pi/Nbars);
    for(int kk=0; kk<Nbars; kk++)
    {
      G4RotationMatrix* rotationMatrix = new G4RotationMatrix();
      rotationMatrix->rotateX(-pi/2*rad); 
      rotationMatrix->rotateY(kk*2*pi/Nbars*rad); 
      new G4PVPlacement(rotationMatrix, G4ThreeVector(Radius*sin(kk*2*pi/Nbars), Radius*cos(kk*2*pi/Nbars) ,0.0), logicScintBar, "ScintBar", logicAG,
		      true, kk, checkOverlaps);
    }
  }
  // outer TPC
  G4Tubs* solidTPCout = new G4Tubs("TPCout_sol", TPCout_radius, 
				   TPCout_radius+TPCout_thick,
				   0.5*fTPClen,
				   0.,360.*deg);
      
  G4LogicalVolume* logicTPCout = new G4LogicalVolume(solidTPCout, 
						     TPCmat,
						     "TPCout_log");
               
  new G4PVPlacement(0, G4ThreeVector(), logicTPCout, "outerTPC", logicAG,
  		    false, 0, checkOverlaps);

  //------------------------------ 
  // drift chamber
  //------------------------------ 
  G4Tubs* soliddrift = new G4Tubs("drift_sol", fTPCrad, 
  				  TPCout_radius,
  				  0.5*fTPClen,
  				  0.,360.*deg);
      
  G4LogicalVolume* logicdrift = new G4LogicalVolume(soliddrift, 
						    fDriftGas,
						    "drift_log");
               
  new G4PVPlacement(0, G4ThreeVector(), logicdrift, "DriftChamber", logicAG,
  		    false, 0, checkOverlaps);

  //------------------------------ 
  // field wires
  //------------------------------ 
  G4Tubs* solidfieldw = new G4Tubs("TPCfw_sol",0.,0.5*FieldWiresDiam,
				   0.5*fTPClen,
				   0.,360.*deg);
  G4LogicalVolume* logicfieldw = new G4LogicalVolume(solidfieldw,fieldw_mat,
						     "fieldw_log");

  G4double xfw,yfw;
  G4int cpy=0;
  for(G4int fw_cpy =0; fw_cpy<NfieldWires; ++fw_cpy)
    {
      TPCBase::TPCBaseInstance()->GetWirePosition( fw_cpy, xfw, yfw );
      new G4PVPlacement(0, G4ThreeVector(xfw/cm,yfw/cm,0.),logicfieldw,"fieldWires",
			logicdrift,false,fw_cpy,false/*checkOverlaps*/);
      ++cpy;
    }
  // G4cout<<"Number of Field Wires: "<<cpy<<"\tSeparated by "
  //  	<<FieldWiresPitch*TMath::RadToDeg()<<" deg or "
  //  	<<FieldWiresPitch*FieldWiresRadPos<<" mm"<<G4endl;
  
  //------------------------------ 
  // anode wires
  //------------------------------ 
  G4Tubs* solidanodew = new G4Tubs("TPCfw_sol",0.,0.5*AnodeWiresDiam,
				   0.5*fTPClen,
				   0.,360.*deg);
  G4LogicalVolume* logicanodew = new G4LogicalVolume(solidanodew,anodew_mat,
						     "anodew_log");

  G4double xaw,yaw;
  cpy=0;
  for(G4int aw_cpy =0; aw_cpy<NanodeWires; ++aw_cpy)
    {
      TPCBase::TPCBaseInstance()->GetAnodePosition( aw_cpy, xaw, yaw );
      new G4PVPlacement(0, G4ThreeVector(xaw/cm,yaw/cm,0.),logicanodew,"anodeWires",
			logicdrift,false,aw_cpy,false/*checkOverlaps*/);
      ++cpy;
    }
  // G4cout<<"Number of Anode Wires: "<<cpy<<"\tSeparated by "
  //  	<<AnodeWiresPitch*TMath::RadToDeg()<<" deg or "
  //  	<<AnodeWiresPitch*AnodeWiresRadPos<<" mm"<<G4endl;
    
  // inner TPC
  G4Tubs* solidTPCin = new G4Tubs("TPCin_sol", TPCin_radius, 
  				  fTPCrad,
  				  0.5*fTPClen,
  				  0.,360.*deg);
      
  G4LogicalVolume* logicTPCin = new G4LogicalVolume(solidTPCin, 
  						    TPCmat,
  						    "TPCin_log");
               
  //  if( kMat )
  new G4PVPlacement(0, G4ThreeVector(), logicTPCin, "innerTPC", logicAG,
		    false, 0, checkOverlaps);


  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // !!! FIELD in the TPC !!!
  //  logicdrift->SetFieldManager(fpFieldSetup->GetLocalFieldManager(),true);
  // !!!!!!!!!!!!!!!!!!!!
  //--------------------------------------------------------------------
  // the DRIFT REGION
  //--------------------------------------------------------------------
  G4Region* driftRegion = G4RegionStore::GetInstance()->GetRegion("DriftRegion",
								  false);
  if (driftRegion) 
    {
      driftRegion->AddRootLogicalVolume( logicdrift );
      G4cout<<"\nTPC has a drift region"<<G4endl;
    } 
  else 
    {
      G4Exception("DetectorConstruction::Construct()", "666",
		  FatalException, "Found no DriftRegion");
    }

  //--------------------------------------------------------------------
  // singleton class which manages the sensitive detectors
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // define TPC as DETECTOR
  //--------------------------------------------------------------------
  TPCSD* theTPCSD = new TPCSD("TPCsense");
  // theTPCSD->SetFiducialRadius( TPCout_radius );
  logicdrift->SetSensitiveDetector(theTPCSD);
  if(SDman)
    {
      SDman->AddNewDetector(theTPCSD);
      G4cout<<"TPC is a detector"<<G4endl;
    }
  else
    G4cout<<"Failed to create TPC as a detector"<<G4endl;
  //--------------------------------------------------------------------
  G4cout<<"\n*******************************************"<<G4endl;
  G4cout<<"*** DetectorConstruction::Construct() ***"<<G4endl;
  G4cout<<"TPC ID = "<<G4BestUnit(TPCin_radius,"Length")<<G4endl;
  G4cout<<"TPC drift region radius  = "<<G4BestUnit(fTPCrad,"Length")<<G4endl;
  G4cout<<"TPC drift region thickness = "<<G4BestUnit(fTPCthi,"Length")<<G4endl;
  G4cout<<"TPC OD = "<<G4BestUnit(TPCout_radius,"Length")<<G4endl;
  G4cout<<"TPC Length = "<<G4BestUnit(fTPClen,"Length")<<G4endl;
  G4cout<<"Anode Wires Position (r) = "<<G4BestUnit(AnodeWiresRadPos,"Length")<<G4endl;
  G4cout<<"Anode Wires Diameter = "<<G4BestUnit(AnodeWiresDiam,"Length")<<G4endl;
  G4cout<<"Magnetic Field = "<<G4BestUnit(fpFieldSetup->GetUniformBField(),"Magnetic flux density")<<G4endl;
  G4cout<<"*******************************************\n"<<G4endl;
  //--------------------------------------------------------------------

  // gammas (from pi0 decay) are making a mess: 0.29 keV energy deposit
  // solution: filter them out :)
  //  G4SDParticleFilter* gammaFilter = new G4SDParticleFilter("gammaFilter","gamma");
  // G4SDParticleFilter* gammaFilter = new G4SDParticleFilter("gammaFilter");
  // gammaFilter->add("gamma");
  //  G4SDChargedFilter* gammaFilter = new G4SDChargedFilter("gammaFilter");
  //--------------------------------------------------------------------
  // define Scintillators as DETECTOR
  //--------------------------------------------------------------------
  ScintBarSD* theScintBarSD = new ScintBarSD("ScintSense");
 // logicScintBars->SetSensitiveDetector(theScintBarSD);
  logicScintBar->SetSensitiveDetector(theScintBarSD);
  //  theScintBarSD->SetFilter(gammaFilter);
  if(SDman)
    {
      SDman->AddNewDetector(theScintBarSD);
      G4cout<<"Scintillator Bars is a detector\n\n"<<G4endl;
    }
  else
    G4cout<<"Failed to create Scintillator Bars as a detector"<<G4endl;
  //--------------------------------------------------------------------


  // Visualization attributes
  logicWorld->SetVisAttributes(G4VisAttributes::Invisible);
  logicAG->SetVisAttributes(G4VisAttributes::Invisible);

  // G4VisAttributes* ElecVisAtt= new G4VisAttributes(G4Colour(1.0,1.0,0.0,0.8));
  // ElecVisAtt->SetVisibility(false);//true
  // logicElec->SetVisAttributes(ElecVisAtt);

  // G4VisAttributes* UHVVisAtt= new G4VisAttributes(G4Colour(0.46,0.53,0.6,0.4));
  // UHVVisAtt->SetVisibility(true);
  // logicUHV->SetVisAttributes(UHVVisAtt);

  // G4VisAttributes* HeVisAtt= new G4VisAttributes(G4Colour::Blue());
  // //  HeVisAtt->SetVisibility(true);
  // HeVisAtt->SetVisibility(false);
  // logicHe->SetVisAttributes(HeVisAtt);

  // G4VisAttributes* OctVisAtt= new G4VisAttributes(G4Colour::Red());
  // OctVisAtt->SetVisibility(true);
  // logicOct->SetVisAttributes(OctVisAtt);  
  // G4VisAttributes* MirVisAtt= new G4VisAttributes(G4Colour::Green());
  // MirVisAtt->SetVisibility(true);
  // logicMirr->SetVisAttributes(MirVisAtt);

  // G4VisAttributes* OVCVisAtt= new G4VisAttributes(G4Colour(0.46,0.53,0.6,0.8));
  // OVCVisAtt->SetVisibility(true);
  // logicOVCin->SetVisAttributes(OVCVisAtt);
  // logicOVCout->SetVisAttributes(OVCVisAtt);

  // G4VisAttributes* ShldVisAtt= new G4VisAttributes(G4Colour(1.,0.5,0.));
  // ShldVisAtt->SetVisibility(true);
  // logicShield->SetVisAttributes(ShldVisAtt);
 
  G4VisAttributes* DriftVisAtt= new G4VisAttributes(G4Colour::Cyan());
  DriftVisAtt->SetVisibility(true);
  logicdrift->SetVisAttributes(DriftVisAtt);
  
  //  logicTPCout->SetVisAttributes(G4VisAttributes::Invisible);

  G4VisAttributes* FwVisAtt= new G4VisAttributes(G4Colour(1.,0.5,0.));
  //  FwVisAtt->SetVisibility(true);
  FwVisAtt->SetVisibility(false);
  logicfieldw->SetVisAttributes(FwVisAtt);
  G4VisAttributes* AwVisAtt= new G4VisAttributes(G4Colour::Black());
  //  AwVisAtt->SetVisibility(true);
  AwVisAtt->SetVisibility(false);
  logicanodew->SetVisAttributes(AwVisAtt);

//  G4VisAttributes* ScintVisAtt= new G4VisAttributes(G4Colour::Green());
  G4VisAttributes* ScintVisAtt= new G4VisAttributes(G4Colour::Red()); //Blue());
  ScintVisAtt->SetVisibility(true);
  //  ScintVisAtt->SetVisibility(false);
    //logicScintBars->SetVisAttributes(ScintVisAtt);
  logicScintBar->SetVisAttributes(ScintVisAtt);

  // logicChamber->SetVisAttributes(G4VisAttributes::Invisible);
  // logicTrap->SetVisAttributes(G4VisAttributes::Invisible);

  G4VisAttributes* MagOutVisAtt=new G4VisAttributes(G4Colour::Green());
  logicMagnetOut->SetVisAttributes(MagOutVisAtt);
  
  G4VisAttributes* MagShlVisAtt= new G4VisAttributes(G4Colour(0.7, 0.4, 0.1)); // brown
  logicMagnetShield->SetVisAttributes(MagShlVisAtt);
  
  G4VisAttributes* MagInVisAtt= new G4VisAttributes(G4Colour::Cyan()); 
  logicMagnetIn ->SetVisAttributes(MagInVisAtt);
  
  G4VisAttributes* MagWindVisAtt= new G4VisAttributes(G4Colour::Yellow()); 
  logicMagnetWinding->SetVisAttributes(MagWindVisAtt);
  
  // G4VisAttributes* MagCovVisAtt= new G4VisAttributes(false); 
  G4VisAttributes* MagCovVisAtt= new G4VisAttributes(G4Colour::Grey());
  logicMagnetCover->SetVisAttributes(MagCovVisAtt);

  //always return the physical World
  return physWorld;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4RunManager.hh"

void DetectorConstruction::UpdateGeometry()
{  
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();
  //  G4RegionStore::GetInstance()->Clean();
  //  G4SDManager::GetSDMpointer()->Clean();
  G4RunManager::GetRunManager()->DefineWorldVolume(Construct());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
