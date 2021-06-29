///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

// Geometrical constant values are in cm
// Y coordinate is the vertical one
//  (y)| 
//     |  / (x)
//     | /
//     |/_________ (z)
// 
// --- >>> VALUES ARE IN cm <<<---

#include "a2mcApparatus.h"

ClassImp(a2mcApparatus)
a2mcApparatus* a2mcApparatus::fgInstance = 0;

using namespace std;

//_____________________________________________________________________________
a2mcApparatus::a2mcApparatus(Int_t run_number, a2mcSettings a2mc_conf)
: TObject(),
    fWorld_Dx(0.),fWorld_Dy(0.),fWorld_Dz(0.)
{
    // Default constuctor
    if (fgInstance) {
      Fatal("a2mcApparatus", "Singleton instance already exists.");
      return;
    }

    runNumber = run_number;
    a2mcConf = a2mc_conf;
    if (!a2mcConf.isValid()) {
        cout << "Error reading configuration " << endl;
        return;
    }
    fgInstance = this;
}

//_____________________________________________________________________________
a2mcApparatus::~a2mcApparatus()
{
    /// Destructor
    fgInstance = 0;
}

//_____________________________________________________________________________
a2mcApparatus* a2mcApparatus::Instance()
{
    /// \return The singleton instance.
    return fgInstance;
}

// _________________________________________________________________________
// | ConstructGeometry [MAIN] -------------------------------------------- |
void a2mcApparatus::ConstructGeometry()
{
    ///< --------- Setting principal geometrical components (solids)  --------->
    Init();

    ///< --------- Inserting the volumes/nodes -------------------------------->
    InsertWorld();
    if(a2mcConf.GetInnEnviro())  InsertInnEnviro();    
    if(a2mcConf.GetOutEnviro())  InsertOutEnviro();    
    if(a2mcConf.GetSilDet())     InsertSilDet();

    gGeoManager->SetVisLevel(3);
    gGeoManager->CloseGeometry();

    //-------- Saving the geometry in a root file
    ostringstream sgeo;
    sgeo << "output/a2mcApparatus_" << runNumber << ".root";
    gGeoManager->Export(sgeo.str().c_str());

    // notify VMC about Root geometry
    gMC->SetRootGeometry();
}

// #########################################################################
// ###########################    SET CONSTANTS     ######################## 
// #########################################################################

// _________________________________________________________________________
// | WORLD --------------------------------------------------------------- |
void a2mcApparatus::Init()
{
    ///< Color code [Cu -> red, Steel -> gray, Al -> dark gray, etc.]
    kcolAl = kGray+3; kcolCu = kRed; kcolFe = kGray; kcolSi = kGreen; kcolNb = kGray; 
    kcolCsI = kYellow; kcolLiqHe = kBlue; kcolLiqN = kBlue; 
    kcolSteel316LN = kGray; kcolCuNbTi = kRed; kcolAlT6082 = kGray+3; kcolEpoxy = kOrange;

    // Geometrical constants (values are in cm)
    // Y coordinate is the vertical one
    //  (y)| 
    //     |  / (x)
    //     | /
    //     |/_________ (z)
    // 
    ///< =========== < VALUES ARE IN cm > =============

    // Defining rotation matrices
    nullRot = new TGeoRotation("nullRot", 0., 0., 0.);

    //------------------------------ 
    // World (x and z are bigger for muon generation over a flat sky)
    //------------------------------ 
    fWorld_Dx = 200.;
    fWorld_Dy = 100.;
    fWorld_Dz = 200.;
    //------------------------------ 
    // Enviro
    //------------------------------ 
    oxfordMag_rMin  = 17.6;
    oxfordMag_rMax  = 19.0; 
    oxfordMag_halfZ = 32.;

    vacuumChamber_rMin  = 7.5975;
    vacuumChamber_rMax  = 7.9375;
    vacuumChamber_halfZ = 31.100;

    rInnMax = vacuumChamber_rMax;   ///< The "inner enviro" limit (max radius)
    rOutMin = oxfordMag_rMin;       ///< The "outer enviro" limit (min radius)

    //------------------------------ 
    // Silicon detector
    //------------------------------ 
    silBox_posZ  = 0.; ///< The detector is centered to the Oxford Magnet (also the center of the alpha reference system)
    silBox_rMin  = rInnMax + 0.01;
    silBox_rMax  = 13.70; ///< This is just enough to contain all the silicon modules
    silBox_halfZ = vacuumChamber_halfZ;

    silMod_halfX = 3.;
    silMod_halfY = 0.0150; ///< the silicon wafer was 300 um thick
    silMod_halfZ = 11.5;
    ///< Making the PCB thicker (same amount of silicon wafer), so to put the silicon module inside the PCB volume (extruding it)
    silPCB_halfX = silMod_halfX;
    silPCB_halfY = 0.0800+silMod_halfY; ///< the PCB support/mount was 1.6 mm
    silPCB_halfZ = silMod_halfZ;
}


// #########################################################################
// ###########################    INSERT VOLUMES    ######################## 
// #########################################################################

// _________________________________________________________________________
// | WORLD --------------------------------------------------------------- |
void a2mcApparatus::InsertWorld()
{
    Double_t world[3];
    world[0] = fWorld_Dx/2.;
    world[1] = fWorld_Dy/2.;
    world[2] = fWorld_Dz/2.;

    top = gGeoManager->Volume("World","BOX", fmedVacuum, world, 3);
    gGeoManager->SetTopVolume(top);
    top->SetVisibility(kFALSE);
}

// _________________________________________________________________________
// | SILICON DETS -------------------------------------------------------- |
void a2mcApparatus::InsertSilDet()
{
    std::cout << "##################### -> Inserting SilDet " << endl;
    ///< ---------------------------------------------------- >
    ///< ---------------------- DetBox ---------------------- >
    ///< ---------------------------------------------------- >
    ///< This "virtual" box is simply a container for all the modules
    double detBox_size[3];
    detBox_size[0] = silBox_rMin;
    detBox_size[1] = silBox_rMax;
    detBox_size[2] = silBox_halfZ;
    ///< Geometrical volume
    TGeoVolume *detBox = gGeoManager->Volume("detBox","TUBE",fmedVacuum, detBox_size,3);
    detBox->SetLineColor(kBlack); detBox->SetTransparency(90);
    double detBox_pos[3];
    ///< Physical volume (positioning the geometrical volume)
    detBox_pos[0] = 0.;
    detBox_pos[1] = 0.;
    detBox_pos[2] = silBox_posZ;
    TGeoCombiTrans *combiDetBox = new TGeoCombiTrans("combiDetBox", detBox_pos[0], detBox_pos[1], detBox_pos[2], nullRot);
    top->AddNode(detBox, 0, combiDetBox);
    ///< ---------------------------------------------------- >
    ///< ------- PCB mount/support & silicon module --------- >
    ///< ---------------------------------------------------- >
    ///< Geometrical volume of the PCB mount
    Double_t silPCB_size[3];
    silPCB_size[0] = silPCB_halfX;
    silPCB_size[1] = silPCB_halfY;
    silPCB_size[2] = silPCB_halfZ;
    TGeoVolume *silPCB = gGeoManager->Volume("silPCB","BOX",fmedFR4,silPCB_size,3);
    silPCB->SetLineColor(kGray); silPCB->SetTransparency(50);
    ///< Geometrical volume of the silicon module (wager)
    Double_t silMod_size[3];
    silMod_size[0] = silMod_halfX;
    silMod_size[1] = silMod_halfY;
    silMod_size[2] = silMod_halfZ;
    TGeoVolume *silMod = gGeoManager->Volume("silMod","BOX",fmedSi,silMod_size,3);
    silMod->SetLineColor(kYellow+2); silMod->SetTransparency(50);
    ///< Physical volume (positioning the silicon module/wafer geometrical volume inside the PCB)
    Double_t silMod_pos[3]={0.,0.,0.};
    silMod_pos[1] = silPCB_halfY-silMod_halfY;
    TGeoRotation* silRot = new TGeoRotation("silRot", 0., 0., 0.);
    TGeoCombiTrans *combiMod = new TGeoCombiTrans("combiMod", silMod_pos[0], silMod_pos[1], silMod_pos[2], silRot);
    silPCB->AddNode(silMod, 1, combiMod);
    ///< Physical volume (positioning the PCB volumes inside the detector box)
    Int_t id = -1;
    Double_t silPCB_pos[3]={0.,0.,0.};
    Double_t phi1 = 0., phi2 = 0.;
    for(UInt_t lay=0; lay<nLayers; lay++) {
        for(UInt_t mod=0; mod<nModules[lay]; mod++) {
            id = SilModPos(lay, mod, phi1, phi2, silPCB_pos[0], silPCB_pos[1], silPCB_pos[2]);
            if(id<0) continue; ///< The correlation layer, module didn't work out
            TGeoRotation* silPCBRot = new TGeoRotation("silPCBRot", phi2, 0., 0.);
            TGeoCombiTrans *combiPCB = new TGeoCombiTrans("combiPCB", silPCB_pos[0], silPCB_pos[1], silPCB_pos[2], silPCBRot);
            detBox->AddNode(silPCB, id, combiPCB);
            ///< Updating the map ["name" -> ID] of the silicon modules
            ostringstream s; ///< Writing the name of the module (see a2mcAppartus.h for a legend)
            s << lay << "si" << std::uppercase << std::hex << mod;
//            cout << id << ": lay " << lay << " mod " << mod << " -> " << s.str() << endl;
            silNameIDMap.insert(std::pair<int, std::string>(id,s.str()));
        }
    }
//    ///< ---------------------------------------------------- >
//    ///< ------------------ silDet modules ------------------ >
//    ///< ---------------------------------------------------- >
//    Double_t silMod_size[3];
//    silMod_size[0] = silMod_halfX;
//    silMod_size[1] = silMod_halfY;
//    silMod_size[2] = silMod_halfZ;
//    ///< Geometrical volume
//    ///< Physical volume (positioning the geometrical volume)
//    Int_t id = -1;
//    Double_t silMod_pos[3]={0.,0.,0.};
//    Double_t phi1 = 0., phi2 = 0.;
//    for(UInt_t lay=0; lay<nLayers; lay++) {
//        for(UInt_t mod=0; mod<nModules[lay]; mod++) {
//            ostringstream s; ///< Writing the name of the module (see a2mcAppartus.h for a legend)
//            s << lay << "si" << std::uppercase << std::hex << mod;
//            TGeoVolume *silMod = gGeoManager->Volume(s.str().c_str(),"BOX",fmedSi,silMod_size,3);
//            silMod->SetLineColor(kGreen); silMod->SetTransparency(50);
//            id = SilModPos(lay, mod, phi1, phi2, silMod_pos[0], silMod_pos[1], silMod_pos[2]);
//            if(id<0) continue; ///< The correlation layer, module didn't work out
//            TGeoRotation* silRot = new TGeoRotation("silRot", phi2, 0., 0.);
//            TGeoCombiTrans *combiSilMod = new TGeoCombiTrans("combiSilMod", silMod_pos[0], silMod_pos[1], silMod_pos[2], silRot);
//            detBox->AddNode(silMod, id, combiSilMod);
//            ///< Updating the map ["name" -> ID] of the silicon modules
//            silNameIDMap.insert(std::pair<int, std::string>(id,s.str()));
//        }
//    }
}

// _________________________________________________________________________
// | ENVIRONMENT --------------------------------------------------------- |
void a2mcApparatus::InsertOutEnviro()
{
    double cyl_size[3];     ///< geometrical size parameters for cylinders
    double sect_size[5];    ///< geometrical size parameters for cylindrical sectors
    double volume_pos[3];   ///< geometrical position parameters
    volume_pos[0] = 0.;     ///< X -> always centered in X
    volume_pos[1] = 0.;     ///< Y -> always centered in Y

    std::cout << "##################### -> Inserting Outer Enviro " << endl;
    ///< ---------------------------------------------------- >
    ///< ----------------- OUTER ENVIRONMENT ---------------- >
    ///< ---------------------------------------------------- >
    ///< The Oxford Magnet is "outside" of the Silicon Detector 
    ///< All the rest is "inside" of the Silicon Detector
    ///< Creating a "fake container" (made of Vaccum) to host them all
    ///< ---------------------------------------------------- >
    ///< ------------------ Oxford Magnet ------------------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = oxfordMag_rMin;      ///< min radius 
    cyl_size[1] = oxfordMag_rMax;      ///< max radius
    cyl_size[2] = oxfordMag_halfZ;    ///< dz (half size)
    volume_pos[2] = 0.;               ///< Z position
    ///< Geometrical volume
    TGeoVolume *oxfordMag = gGeoManager->Volume("oxfordMag", "TUBE", fmedCu , cyl_size, 3);
    oxfordMag->SetLineColor(kcolCu); oxfordMag->SetTransparency(60);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiOxfordMag = new TGeoCombiTrans("combiOxfordMag", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    top->AddNode(oxfordMag, 0, combiOxfordMag);
}

void a2mcApparatus::InsertInnEnviro()
{
    double cyl_size[3];     ///< geometrical size parameters for cylinders
    double sect_size[5];    ///< geometrical size parameters for cylindrical sectors
    double volume_pos[3];   ///< geometrical position parameters
    volume_pos[0] = 0.;     ///< X -> always centered in X
    volume_pos[1] = 0.;     ///< Y -> always centered in Y

    std::cout << "##################### -> Inserting Inner Enviro " << endl;
    ///< ---------------------------------------------------- >
    ///< ----------------- INNER ENVIRONMENT ---------------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = 0.;                       ///< min radius
    cyl_size[1] = vacuumChamber_rMax +0.01; ///< max radius (100 um bigger than the Vaccum Chamber outer wall)
    cyl_size[2] = vacuumChamber_halfZ+0.01; ///< dz (half size) 
    volume_pos[2]  = 0.;        ///< Z position
    ///< Geometrical volume
    TGeoVolume *innEnvironBox = gGeoManager->Volume("innEnvironBox", "TUBE", fmedVacuum , cyl_size, 3);
    innEnvironBox->SetLineColor(kBlack); innEnvironBox->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiInnEnvironBox = new TGeoCombiTrans("combiInnEnvironBox", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    ///< The physical volume is placed once all the other volumes have been placed inside it [SEE THE END OF THIS METHOD]
    
    ///< ---------------------------------------------------- >
    ///< ------------ Vacuum Chamber outer wall ------------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = vacuumChamber_rMin ;       ///< min radius
    cyl_size[1] = vacuumChamber_rMax ;       ///< max radius
    cyl_size[2] = vacuumChamber_halfZ;       ///< dz (half size)
    volume_pos[2]  = 0.;        ///< Z position
    ///< Geometrical volume
    TGeoVolume *vacChambOutWall = gGeoManager->Volume("vacChambOutWall", "TUBE", fmedSteel316LN , cyl_size, 3);
    vacChambOutWall->SetLineColor(kcolSteel316LN); vacChambOutWall->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiVacChambOutWall = new TGeoCombiTrans("combiVacChambOutWall", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(vacChambOutWall, 0, combiVacChambOutWall);

    ///< ---------------------------------------------------- >
    ///< ------------------- Heat shield -------------------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = 6.1905;       ///< min radius
    cyl_size[1] = 6.5085;       ///< max radius
    cyl_size[2] = 31.100;       ///< dz (half size)
    volume_pos[2]  = 0.;        ///< Z position
    ///< Geometrical volume
    TGeoVolume *headShield = gGeoManager->Volume("headShield", "TUBE", fmedCu , cyl_size, 3); ///< Copper -> TO CHECK
    headShield->SetLineColor(kcolCu); headShield->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiHeadShield = new TGeoCombiTrans("combiHeadShield", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(headShield, 0, combiHeadShield);

    ///< ---------------------------------------------------- >
    ///< ------------ Vacuum Chamber inner wall ------------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = 5.504;        ///< min radius
    cyl_size[1] = 5.715;        ///< max radius
    cyl_size[2] = 31.100;       ///< dz (half size)
    volume_pos[2]  = 0.;        ///< Z position
    ///< Geometrical volume
    TGeoVolume *vacChambInnWall = gGeoManager->Volume("vacChambInnWall", "TUBE", fmedSteel316LN , cyl_size, 3);
    vacChambInnWall->SetLineColor(kcolSteel316LN); vacChambInnWall->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiVacChambInnWall = new TGeoCombiTrans("combiVacChambInnWall", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(vacChambInnWall, 0, combiVacChambInnWall);

    ///< ---------------------------------------------------- >
    ///< ----- Ultra High Vacuum Chamber outer wall --------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = 2.40000;      ///< min radius
    cyl_size[1] = 2.52500;      ///< max radius
    cyl_size[2] = 15.5945;      ///< dz (half size)
    volume_pos[2]  = 0.;        ///< Z position
    ///< Geometrical volume
    TGeoVolume *UHVChambOutWall = gGeoManager->Volume("UHVChambOutWall", "TUBE", fmedSteel316LN , cyl_size, 3);
    UHVChambOutWall->SetLineColor(kcolSteel316LN); UHVChambOutWall->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiUHVChambOutWall = new TGeoCombiTrans("combiUHVChambOutWall", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(UHVChambOutWall, 0, combiUHVChambOutWall);

    ///< ---------------------------------------------------- >
    ///< ------- Ultra High Vacuum Chamber outer A ---------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = 2.40000;      ///< min radius
    cyl_size[1] = 2.88000;      ///< max radius
    cyl_size[2] = 7.72900;      ///< dz (half size)
    volume_pos[2]  = -23.324;   ///< Z position
    ///< Geometrical volume
    TGeoVolume *UHVChambOutA = gGeoManager->Volume("UHVChambOutA", "TUBE", fmedSteel316LN , cyl_size, 3);
    UHVChambOutA->SetLineColor(kcolSteel316LN); UHVChambOutA->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiUHVChambOutA = new TGeoCombiTrans("combiUHVChambOutA", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(UHVChambOutA, 0, combiUHVChambOutA);

    ///< ---------------------------------------------------- >
    ///< ------- Ultra High Vacuum Chamber outer B ---------- >
    ///< ---------------------------------------------------- >
    cyl_size[0] = 2.40000;      ///< min radius
    cyl_size[1] = 2.88000;      ///< max radius
    cyl_size[2] = 7.72900;      ///< dz (half size)
    volume_pos[2]  = +23.324;   ///< Z position
    ///< Geometrical volume
    TGeoVolume *UHVChambOutB = gGeoManager->Volume("UHVChambOutB", "TUBE", fmedSteel316LN , cyl_size, 3);
    UHVChambOutB->SetLineColor(kcolSteel316LN); UHVChambOutB->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiUHVChambOutB = new TGeoCombiTrans("combiUHVChambOutB", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(UHVChambOutB, 0, combiUHVChambOutB);

    ///< ---------------------------------------------------- >
    ///< ---------------- ELECTRODES STACK ------------------ >
    ///< ---------------------------------------------------- >
    cyl_size[0] = 2.2275;       ///< min radius
    cyl_size[1] = 2.3780;       ///< max radius
    cyl_size[2] = 31.100;       ///< dz (half size)
    volume_pos[2]  = 0.;        ///< Z position
    ///< Geometrical volume
    TGeoVolume *electrodesStack = gGeoManager->Volume("electrodesStack", "TUBE", fmedAlT6082 , cyl_size, 3);
    electrodesStack->SetLineColor(kcolAlT6082); electrodesStack->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiElectrodesStack = new TGeoCombiTrans("combiElectrodesStack", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(electrodesStack, 0, combiElectrodesStack);

    ///< ---------------------------------------------------- >
    ///< -------------------- BNL MAGNETS ------------------- >
    ///< ---------------------------------------------------- >
    //<!-- Mirror Coil 1 (CuNiTi) -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.8925;       ///< dz (half size)
    volume_pos[2]  = -13.702;   ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLmirrorC1 = gGeoManager->Volume("BNLmirrorC1", "TUBE", fmedCuNbTi , cyl_size, 3);
    BNLmirrorC1->SetLineColor(kcolCuNbTi); BNLmirrorC1->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLmirrorC1 = new TGeoCombiTrans("combiBNLmirrorC1", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLmirrorC1, 0, combiBNLmirrorC1);
    //<!-- Mirror Coil 2 (CuNiTi) -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.8925;       ///< dz (half size)
    volume_pos[2]  = -6.851;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLmirrorC2 = gGeoManager->Volume("BNLmirrorC2", "TUBE", fmedCuNbTi , cyl_size, 3);
    BNLmirrorC2->SetLineColor(kcolCuNbTi); BNLmirrorC2->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLmirrorC2 = new TGeoCombiTrans("combiBNLmirrorC2", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLmirrorC2, 0, combiBNLmirrorC2);
    //<!-- Mirror Coil 3 (CuNiTi) -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.8925;       ///< dz (half size)
    volume_pos[2]  = 0.;        ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLmirrorC3 = gGeoManager->Volume("BNLmirrorC3", "TUBE", fmedCuNbTi , cyl_size, 3);
    BNLmirrorC3->SetLineColor(kcolCuNbTi); BNLmirrorC3->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLmirrorC3 = new TGeoCombiTrans("combiBNLmirrorC3", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLmirrorC3, 0, combiBNLmirrorC3);
    //<!-- Mirror Coil 4 (CuNiTi) -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.8925;       ///< dz (half size)
    volume_pos[2]  = 6.8510;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLmirrorC4 = gGeoManager->Volume("BNLmirrorC4", "TUBE", fmedCuNbTi , cyl_size, 3);
    BNLmirrorC4->SetLineColor(kcolCuNbTi); BNLmirrorC4->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLmirrorC4 = new TGeoCombiTrans("combiBNLmirrorC4", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLmirrorC4, 0, combiBNLmirrorC4);
    //<!-- Mirror Coil 5 (CuNiTi) -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.8925;       ///< dz (half size)
    volume_pos[2]  = 13.702;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLmirrorC5 = gGeoManager->Volume("BNLmirrorC5", "TUBE", fmedCuNbTi , cyl_size, 3);
    BNLmirrorC5->SetLineColor(kcolCuNbTi); BNLmirrorC5->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLmirrorC5 = new TGeoCombiTrans("combiBNLmirrorC5", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLmirrorC5, 0, combiBNLmirrorC5);
    //<! ###################################  Octupole winding (CuNiTi) ###################################### -->
    ///< ----------------------------------------------- 1 & 12
    //<!-- BNLOCT1 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 0.;          ///< phi start
    sect_size[4] = 30.;         ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT1 = gGeoManager->Volume("BNLOCT1", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT1->SetLineColor(kcolCuNbTi); BNLOCT1->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT1 = new TGeoCombiTrans("combiBNLOCT1", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT1, 0, combiBNLOCT1);
    //<!-- BNLOCT12 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 0.;          ///< phi start
    sect_size[4] = 75.;         ///< phi stop
    volume_pos[2] = -13.101;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT12 = gGeoManager->Volume("BNLOCT12", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT12->SetLineColor(kcolCuNbTi); BNLOCT12->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT12 = new TGeoCombiTrans("combiBNLOCT12", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT12, 0, combiBNLOCT12);
    ///< ----------------------------------------------- 2 & 23
    //<!-- BNLOCT2 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 45.;         ///< phi start
    sect_size[4] = 75.;         ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT2 = gGeoManager->Volume("BNLOCT2", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT2->SetLineColor(kcolCuNbTi); BNLOCT2->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT2 = new TGeoCombiTrans("combiBNLOCT2", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT2, 0, combiBNLOCT2);
    //<!-- BNLOCT23 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 45.;         ///< phi start
    sect_size[4] = 120.;        ///< phi stop
    volume_pos[2] = 13.101;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT23 = gGeoManager->Volume("BNLOCT23", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT23->SetLineColor(kcolCuNbTi); BNLOCT23->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT23 = new TGeoCombiTrans("combiBNLOCT23", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT23, 0, combiBNLOCT23);
    ///< ----------------------------------------------- 3 & 34
    //<!-- BNLOCT3 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 90.;         ///< phi start
    sect_size[4] = 120.;        ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT3 = gGeoManager->Volume("BNLOCT3", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT3->SetLineColor(kcolCuNbTi); BNLOCT3->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT3 = new TGeoCombiTrans("combiBNLOCT3", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT3, 0, combiBNLOCT3);
    //<!-- BNLOCT34 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 90.;         ///< phi start
    sect_size[4] = 165.;        ///< phi stop
    volume_pos[2] = -13.101;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT34 = gGeoManager->Volume("BNLOCT34", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT34->SetLineColor(kcolCuNbTi); BNLOCT34->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT34 = new TGeoCombiTrans("combiBNLOCT34", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT34, 0, combiBNLOCT34);
    ///< ----------------------------------------------- 4 & 45
    //<!-- BNLOCT4 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 135.;        ///< phi start
    sect_size[4] = 165.;        ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT4 = gGeoManager->Volume("BNLOCT4", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT4->SetLineColor(kcolCuNbTi); BNLOCT4->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT4 = new TGeoCombiTrans("combiBNLOCT4", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT4, 0, combiBNLOCT4);
    //<!-- BNLOCT45 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 135.;        ///< phi start
    sect_size[4] = 210.;        ///< phi stop
    volume_pos[2] = 13.101;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT45 = gGeoManager->Volume("BNLOCT45", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT45->SetLineColor(kcolCuNbTi); BNLOCT45->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT45 = new TGeoCombiTrans("combiBNLOCT45", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT45, 0, combiBNLOCT45);
    ///< ----------------------------------------------- 5 & 56
    //<!-- BNLOCT5 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 180.;        ///< phi start
    sect_size[4] = 210.;        ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT5 = gGeoManager->Volume("BNLOCT5", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT5->SetLineColor(kcolCuNbTi); BNLOCT5->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT5 = new TGeoCombiTrans("combiBNLOCT5", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT5, 0, combiBNLOCT5);
    //<!-- BNLOCT56 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 180.;        ///< phi start
    sect_size[4] = 255.;        ///< phi stop
    volume_pos[2] = -13.101;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT56 = gGeoManager->Volume("BNLOCT56", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT56->SetLineColor(kcolCuNbTi); BNLOCT56->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT56 = new TGeoCombiTrans("combiBNLOCT56", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT56, 0, combiBNLOCT56);
    ///< ----------------------------------------------- 6 & 67
    //<!-- BNLOCT6 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 225.;        ///< phi start
    sect_size[4] = 255.;        ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT6 = gGeoManager->Volume("BNLOCT6", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT6->SetLineColor(kcolCuNbTi); BNLOCT6->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT6 = new TGeoCombiTrans("combiBNLOCT6", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT6, 0, combiBNLOCT6);
    //<!-- BNLOCT67 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 225.;        ///< phi start
    sect_size[4] = 300.;        ///< phi stop
    volume_pos[2] = 13.101;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT67 = gGeoManager->Volume("BNLOCT67", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT67->SetLineColor(kcolCuNbTi); BNLOCT67->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT67 = new TGeoCombiTrans("combiBNLOCT67", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT67, 0, combiBNLOCT67);
    ///< ----------------------------------------------- 7 & 78
    //<!-- BNLOCT7 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 270.;        ///< phi start
    sect_size[4] = 300.;        ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT7 = gGeoManager->Volume("BNLOCT7", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT7->SetLineColor(kcolCuNbTi); BNLOCT7->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT7 = new TGeoCombiTrans("combiBNLOCT7", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT7, 0, combiBNLOCT7);
    //<!-- BNLOCT78 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 270.;        ///< phi start
    sect_size[4] = 345.;        ///< phi stop
    volume_pos[2] = -13.101;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT78 = gGeoManager->Volume("BNLOCT78", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT78->SetLineColor(kcolCuNbTi); BNLOCT78->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT78 = new TGeoCombiTrans("combiBNLOCT78", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT78, 0, combiBNLOCT78);    
    ///< ----------------------------------------------- 8 & 81
    //<!-- BNLOCT8 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 12.500;      ///< dz (half size)
    sect_size[3] = 315.;        ///< phi start
    sect_size[4] = 345.;        ///< phi stop
    volume_pos[2]  = 0;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT8 = gGeoManager->Volume("BNLOCT8", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT8->SetLineColor(kcolCuNbTi); BNLOCT8->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT8 = new TGeoCombiTrans("combiBNLOCT8", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT8, 0, combiBNLOCT8);
    //<!-- BNLOCT81 (CuNiTi) -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 0.601;       ///< dz (half size)
    sect_size[3] = 315.;        ///< phi start
    sect_size[4] = 30.;         ///< phi stop
    volume_pos[2] = 13.101;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLOCT81 = gGeoManager->Volume("BNLOCT81", "TUBE", fmedCuNbTi , sect_size, 5);
    BNLOCT81->SetLineColor(kcolCuNbTi); BNLOCT81->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLOCT81 = new TGeoCombiTrans("combiBNLOCT81", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLOCT81, 0, combiBNLOCT81);    
    //<! ###################################  Solenoids ###################################### -->
    //<!-- SolUp -->
    cyl_size[0] = 2.8800;       ///< min radius
    cyl_size[1] = 3.8560;       ///< max radius
    cyl_size[2] = 7.0520;       ///< dz (half size)
    volume_pos[2] = -24.001;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLSolUp = gGeoManager->Volume("BNLSolUp", "TUBE", fmedCuNbTi , cyl_size, 3);
    BNLSolUp->SetLineColor(kcolCuNbTi); BNLSolUp->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSolUp = new TGeoCombiTrans("combiBNLSolUp", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSolUp, 0, combiBNLSolUp);
    //<!-- SolDw -->
    cyl_size[0] = 2.8800;       ///< min radius
    cyl_size[1] = 3.8560;       ///< max radius
    cyl_size[2] = 7.0520;       ///< dz (half size)
    volume_pos[2] = 24.001;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLSolDw = gGeoManager->Volume("BNLSolDw", "TUBE", fmedCuNbTi , cyl_size, 3);
    BNLSolDw->SetLineColor(kcolCuNbTi); BNLSolDw->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSolDw = new TGeoCombiTrans("combiBNLSolDw", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSolDw, 0, combiBNLSolDw);
    //<! ###################################  Supports ###################################### -->
    //<!-- SuppMirrUp -->
    cyl_size[0] = 2.8800;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 0.6770;       ///< dz (half size)
    volume_pos[2] = -16.2715;   ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLSuppMirrUp = gGeoManager->Volume("BNLSuppMirrUp", "TUBE", fmedSteel316LN , cyl_size, 3);
    BNLSuppMirrUp->SetLineColor(kcolSteel316LN); BNLSuppMirrUp->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSuppMirrUp = new TGeoCombiTrans("combiBNLSuppMirrUp", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSuppMirrUp, 0, combiBNLSuppMirrUp);
    //<!-- SuppMirrDw -->
    cyl_size[0] = 2.8800;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 0.6770;       ///< dz (half size)
    volume_pos[2] = 16.2715;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLSuppMirrDw = gGeoManager->Volume("BNLSuppMirrDw", "TUBE", fmedSteel316LN , cyl_size, 3);
    BNLSuppMirrDw->SetLineColor(kcolSteel316LN); BNLSuppMirrDw->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSuppMirrDw = new TGeoCombiTrans("combiBNLSuppMirrDw", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSuppMirrDw, 0, combiBNLSuppMirrDw);
    //<!-- SuppSolUpA -->
    cyl_size[0] = 3.8560;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 0.9255;       ///< dz (half size)
    volume_pos[2] = -17.874;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLSuppSolUpA = gGeoManager->Volume("BNLSuppSolUpA", "TUBE", fmedSteel316LN , cyl_size, 3);
    BNLSuppSolUpA->SetLineColor(kcolSteel316LN); BNLSuppSolUpA->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSuppSolUpA = new TGeoCombiTrans("combiBNLSuppSolUpA", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSuppSolUpA, 0, combiBNLSuppSolUpA);
    //<!-- SuppSolUpB -->
    cyl_size[0] = 3.8560;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 0.9255;       ///< dz (half size)
    volume_pos[2] = -30.127;    ///< Z position
    ///< Geometrical volume
     TGeoVolume *BNLSuppSolUpB = gGeoManager->Volume("BNLSuppSolUpB", "TUBE", fmedSteel316LN , cyl_size, 3);
    BNLSuppSolUpB->SetLineColor(kcolSteel316LN); BNLSuppSolUpB->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSuppSolUpB = new TGeoCombiTrans("combiBNLSuppSolUpB", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSuppSolUpB, 0, combiBNLSuppSolUpB);
    //<!-- SuppSolDwA -->
    cyl_size[0] = 3.8560;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 0.9255;       ///< dz (half size)
    volume_pos[2] = 17.874;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLSuppSolDwA = gGeoManager->Volume("BNLSuppSolDwA", "TUBE", fmedSteel316LN , cyl_size, 3);
    BNLSuppSolDwA->SetLineColor(kcolSteel316LN); BNLSuppSolDwA->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSuppSolDwA = new TGeoCombiTrans("combiBNLSuppSolDwA", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSuppSolDwA, 0, combiBNLSuppSolDwA);
    //<!-- SuppSolDwB -->
    cyl_size[0] = 3.8560;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 0.9255;       ///< dz (half size)
    volume_pos[2] = 30.127;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *BNLSuppSolDwB = gGeoManager->Volume("BNLSuppSolDwB", "TUBE", fmedSteel316LN , cyl_size, 3);
    BNLSuppSolDwB->SetLineColor(kcolSteel316LN); BNLSuppSolDwB->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiBNLSuppSolDwB = new TGeoCombiTrans("combiBNLSuppSolDwB", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(BNLSuppSolDwB, 0, combiBNLSuppSolDwB);

    ///< ---------------------------------------------------- >
    ///< ------------------- LIQUID HELIUM ------------------ >
    ///< ---------------------------------------------------- >
    //<!-- Liquid Helium 0-->
    cyl_size[0] = 4.4160;       ///< min radius
    cyl_size[1] = 5.5040;       ///< max radius
    cyl_size[2] = 31.100;       ///< dz (half size)
    volume_pos[2] = 0.;         ///< Z position
    ///< Geometrical volume
    TGeoVolume *LHeTube0 = gGeoManager->Volume("LHeTube0", "TUBE", fmedLiqHe , cyl_size, 3);
    LHeTube0->SetLineColor(kcolLiqHe); LHeTube0->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiLHeTube0 = new TGeoCombiTrans("combiLHeTube0", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(LHeTube0, 0, combiLHeTube0);
    //<! ########################## Liquid Helium between mirror coils ############################ -->
    //<!-- LHeC12 -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.5330;       ///< dz (half size)
    volume_pos[2] = -10.277;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *LHeC12 = gGeoManager->Volume("LHeC12", "TUBE", fmedLiqHe , cyl_size, 3);
    LHeC12->SetLineColor(kcolLiqHe); LHeC12->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiLHeC12 = new TGeoCombiTrans("combiLHeC12", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(LHeC12, 0, combiLHeC12);
    //<!-- LHeC23 -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.5330;       ///< dz (half size)
    volume_pos[2] = -3.4260;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *LHeC23 = gGeoManager->Volume("LHeC23", "TUBE", fmedLiqHe , cyl_size, 3);
    LHeC23->SetLineColor(kcolLiqHe); LHeC23->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiLHeC23 = new TGeoCombiTrans("combiLHeC23", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(LHeC23, 0, combiLHeC23);
    //<!-- LHeC34 -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.5330;       ///< dz (half size)
    volume_pos[2] = 3.426;      ///< Z position
    ///< Geometrical volume
    TGeoVolume *LHeC34 = gGeoManager->Volume("LHeC34", "TUBE", fmedLiqHe , cyl_size, 3);
    LHeC34->SetLineColor(kcolLiqHe); LHeC34->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiLHeC34 = new TGeoCombiTrans("combiLHeC34", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(LHeC34, 0, combiLHeC34);
    //<!-- LHeC45 -->
    cyl_size[0] = 3.9280;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 1.5330;       ///< dz (half size)
    volume_pos[2] = 10.277;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *LHeC45 = gGeoManager->Volume("LHeC45", "TUBE", fmedLiqHe , cyl_size, 3);
    LHeC45->SetLineColor(kcolLiqHe); LHeC45->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiLHeC45 = new TGeoCombiTrans("combiLHeC45", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(LHeC45, 0, combiLHeC45);
    //<! ##########################  Liquid Helium around solenoids ############################ -->
    //<!-- LHeA -->
    cyl_size[0] = 3.8560;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 5.2010;       ///< dz (half size)
    volume_pos[2] = -24.001;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *LHeA = gGeoManager->Volume("LHeA", "TUBE", fmedLiqHe , cyl_size, 3);
    LHeA->SetLineColor(kcolLiqHe); LHeA->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiLHeA = new TGeoCombiTrans("combiLHeA", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(LHeA, 0, combiLHeA);
    //<!-- LHeB -->
    cyl_size[0] = 3.8560;       ///< min radius
    cyl_size[1] = 4.4160;       ///< max radius
    cyl_size[2] = 5.2010;       ///< dz (half size)
    volume_pos[2] = 24.001;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *LHeB = gGeoManager->Volume("LHeB", "TUBE", fmedLiqHe , cyl_size, 3);
    LHeB->SetLineColor(kcolLiqHe); LHeB->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiLHeB = new TGeoCombiTrans("combiLHeB", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(LHeB, 0, combiLHeB);

    ///< ---------------------------------------------------- >
    ///< ----------------------- EPOXY ---------------------- >
    ///< ---------------------------------------------------- >
    //<! ########################## EPOXY around octupole ############################ -->
    //<!-- EPO1 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 30.;         ///< phi start
    sect_size[4] = 45.;         ///< phi stop
    volume_pos[2]  = 0.601;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO1 = gGeoManager->Volume("EPO1", "TUBE", fmedEpoxy , sect_size, 5);
    EPO1->SetLineColor(kcolEpoxy); EPO1->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO1 = new TGeoCombiTrans("combiEPO1", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO1, 0, combiEPO1);
    //<!-- EPO2 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 75.;         ///< phi start
    sect_size[4] = 90.;         ///< phi stop
    volume_pos[2]  = -0.601;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO2 = gGeoManager->Volume("EPO2", "TUBE", fmedEpoxy , sect_size, 5);
    EPO2->SetLineColor(kcolEpoxy); EPO2->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO2 = new TGeoCombiTrans("combiEPO2", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO2, 0, combiEPO2);
    //<!-- EPO3 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 120.;        ///< phi start
    sect_size[4] = 135.;        ///< phi stop
    volume_pos[2]  = 0.601;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO3 = gGeoManager->Volume("EPO3", "TUBE", fmedEpoxy , sect_size, 5);
    EPO3->SetLineColor(kcolEpoxy); EPO3->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO3 = new TGeoCombiTrans("combiEPO3", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO3, 0, combiEPO3);
    //<!-- EPO4 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 165.;        ///< phi start
    sect_size[4] = 180.;        ///< phi stop
    volume_pos[2]  = -0.601;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO4 = gGeoManager->Volume("EPO4", "TUBE", fmedEpoxy , sect_size, 5);
    EPO4->SetLineColor(kcolEpoxy); EPO4->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO4 = new TGeoCombiTrans("combiEPO4", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO4, 0, combiEPO4);
    //<!-- EPO5 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 210.;        ///< phi start
    sect_size[4] = 225.;        ///< phi stop
    volume_pos[2]  = 0.601;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO5 = gGeoManager->Volume("EPO5", "TUBE", fmedEpoxy , sect_size, 5);
    EPO5->SetLineColor(kcolEpoxy); EPO5->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO5 = new TGeoCombiTrans("combiEPO5", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO5, 0, combiEPO5);
    //<!-- EPO6 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 255.;        ///< phi start
    sect_size[4] = 270.;        ///< phi stop
    volume_pos[2]  = -0.601;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO6 = gGeoManager->Volume("EPO6", "TUBE", fmedEpoxy , sect_size, 5);
    EPO6->SetLineColor(kcolEpoxy); EPO6->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO6 = new TGeoCombiTrans("combiEPO6", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO6, 0, combiEPO6);
    //<!-- EPO7 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 300.;        ///< phi start
    sect_size[4] = 315.;        ///< phi stop
    volume_pos[2]  = 0.601;     ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO7 = gGeoManager->Volume("EPO7", "TUBE", fmedEpoxy , sect_size, 5);
    EPO7->SetLineColor(kcolEpoxy); EPO7->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO7 = new TGeoCombiTrans("combiEPO7", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO7, 0, combiEPO7);
    //<!-- EPO8 -->
    sect_size[0] = 2.5250;      ///< min radius
    sect_size[1] = 3.9280;      ///< max radius
    sect_size[2] = 13.101;      ///< dz (half size)
    sect_size[3] = 345.;        ///< phi start
    sect_size[4] = 360.;        ///< phi stop
    volume_pos[2]  = -0.601;    ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPO8 = gGeoManager->Volume("EPO8", "TUBE", fmedEpoxy , sect_size, 5);
    EPO8->SetLineColor(kcolEpoxy); EPO8->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPO8 = new TGeoCombiTrans("combiEPO8", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPO8, 0, combiEPO8);
    //<!-- EPOUp -->
    cyl_size[0] = 2.5250;      ///< min radius
    cyl_size[1] = 3.9280;      ///< max radius
    cyl_size[2] = 0.94625;     ///< dz (half size)
    volume_pos[2]  = -14.64825; ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPOUp = gGeoManager->Volume("EPOUp", "TUBE", fmedEpoxy , cyl_size, 3);
    EPOUp->SetLineColor(kcolEpoxy); EPOUp->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPOUp = new TGeoCombiTrans("combiEPOUp", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPOUp, 0, combiEPOUp);
    //<!-- EPODw -->
    cyl_size[0] = 2.5250;      ///< min radius
    cyl_size[1] = 3.9280;      ///< max radius
    cyl_size[2] = 0.94625;     ///< dz (half size)
    volume_pos[2]  = 14.64825; ///< Z position
    ///< Geometrical volume
    TGeoVolume *EPODw = gGeoManager->Volume("EPODw", "TUBE", fmedEpoxy , cyl_size, 3);
    EPODw->SetLineColor(kcolEpoxy); EPODw->SetTransparency(90);
    ///< Physical volume (positioning the geometrical volume)
    TGeoCombiTrans *combiEPODw = new TGeoCombiTrans("combiEPODw", volume_pos[0], volume_pos[1], volume_pos[2], nullRot);
    innEnvironBox->AddNode(EPODw, 0, combiEPODw);

    ///< The physical volume innEnvironBox is placed here once all the other volumes have been placed inside it
    top->AddNode(innEnvironBox, 0, combiInnEnvironBox);

}

//_____________________________________________________________________________
void a2mcApparatus::ConstructMaterials()
{

//--------- Material definition ---------

    // Create Root geometry manager 
    new TGeoManager("a2MCGeo", "Alpha2 Apparatus Geometry");

    Double_t a;         ///< Mass of a mole in g/mole   
    Double_t z;         ///< Atomic number
    Double_t density;   ///< Material density in g/cm3

    ///< ---------------------------------------------------- >
    ///< --------- Elements (with their properties) --------- >
    ///< ---------------------------------------------------- >
    TGeoElementTable *table = gGeoManager->GetElementTable();
    TGeoElement *elH   = table->GetElement(1);
    TGeoElement *elHe  = table->GetElement(2);
    TGeoElement *elC   = table->GetElement(6);
    TGeoElement *elN   = table->GetElement(7);
    TGeoElement *elO   = table->GetElement(8);
    TGeoElement *elF   = table->GetElement(9);
    TGeoElement *elNa  = table->GetElement(11);
    TGeoElement *elMg  = table->GetElement(12);
    TGeoElement *elAl  = table->GetElement(13);
    TGeoElement *elSi  = table->GetElement(14);
    TGeoElement *elP   = table->GetElement(15);
    TGeoElement *elS   = table->GetElement(16);
    TGeoElement *elCl  = table->GetElement(17);
    TGeoElement *elK   = table->GetElement(19);
    TGeoElement *elCa  = table->GetElement(20);
    TGeoElement *elTi  = table->GetElement(22);
    TGeoElement *elCr  = table->GetElement(24);
    TGeoElement *elMn  = table->GetElement(25);
    TGeoElement *elFe  = table->GetElement(26);
    TGeoElement *elNi  = table->GetElement(28);
    TGeoElement *elCu  = table->GetElement(29);
    TGeoElement *elZn  = table->GetElement(30);
    TGeoElement *elBr  = table->GetElement(35);
    TGeoElement *elSr  = table->GetElement(38);
    TGeoElement *elNb  = table->GetElement(41);
    TGeoElement *elMo  = table->GetElement(42);
    TGeoElement *elAg  = table->GetElement(47);
    TGeoElement *elI   = table->GetElement(53);
    TGeoElement *elCs  = table->GetElement(55);
    TGeoElement *elBa  = table->GetElement(56);
    TGeoElement *elAu  = table->GetElement(79);
    TGeoElement *elPb  = table->GetElement(82);

    ///< ---------------------------------------------------- >
    ///< ------------- Materials (and mixtures) ------------- >
    ///< ---------------------------------------------------- >
    // ---------------------> materials <-------------------------- //
    ///< density is given in g/cm^3
    ///< Vacuum
    TGeoMaterial *matVacuum = new TGeoMaterial("Vacuum", a=1.e-16, z=1.e-16, density=1.e-16); 
    ///< Air
    TGeoMixture *matAir = new TGeoMixture("Air", 2, 1.29e-03);
    matAir->AddElement(elN, 0.7); 
    matAir->AddElement(elO, 0.3);
    ///< Aluminum
    TGeoMaterial *matAl = new TGeoMaterial("Aluminium", elAl, 2.70);
    ///< Copper
    TGeoMaterial *matCu = new TGeoMaterial("Copper",    elCu, 8.94);
    ///< Iron
    TGeoMaterial *matFe = new TGeoMaterial("Iron",      elFe, 7.874);
    ///< Silicon
    TGeoMaterial *matSi = new TGeoMaterial("Silicon",   elSi, 2.329);
    ///< Niobium
    TGeoMaterial *matNb = new TGeoMaterial("Niobium",   elNb, 8.570);
    ///< Cesium-iodine
    TGeoMixture *matCsI = new TGeoMixture("CsI", 2, 4.53);
    matCsI->AddElement(elCs, 0.51); 
    matCsI->AddElement(elI,  0.49);
    ///< Liquid Helium
    TGeoMaterial *matLiqHe = new TGeoMaterial("LiqHe",  elHe, 0.125);
    ///< Liquid Nitrogen
    TGeoMaterial *matLiqN = new TGeoMaterial("LiqN",    elN, 0.807);
    ///< Stainless steel (316LN)
    TGeoMixture *matSteel316LN = new TGeoMixture("Steel316LN", 10, 8.00);
    matSteel316LN->AddElement(elC,  0.00006); // [01]
    matSteel316LN->AddElement(elN,  0.00040); // [02]
    matSteel316LN->AddElement(elSi, 0.00500); // [03]
    matSteel316LN->AddElement(elP,  0.00020); // [04]
    matSteel316LN->AddElement(elS,  0.00009); // [05]
    matSteel316LN->AddElement(elCr, 0.16000); // [06]
    matSteel316LN->AddElement(elMn, 0.01900); // [07]
    matSteel316LN->AddElement(elFe, 0.77065); ///< It was 0.76950 in material2.xml -> changed to make sum = 100%
    matSteel316LN->AddElement(elNi, 0.00160); // [09]
    matSteel316LN->AddElement(elMo, 0.04300); // [10]
///< -------------------------------1.00000-------
    ///< Copper Niobium Titanium 
    TGeoMixture *matCuNbTi = new TGeoMixture("CuNbTi", 3, 5.60);
    matCuNbTi->AddElement(elCu, 0.29); // [02]
    matCuNbTi->AddElement(elNb, 0.47); // [03]
    matCuNbTi->AddElement(elTi, 0.24); // [01]
    ///< Aluminum T6082
    TGeoMixture *matAlT6082 = new TGeoMixture("AlT6082", 9, 2.70);
    matAlT6082->AddElement(elMg, 0.0071); // [01]
    matAlT6082->AddElement(elAl, 0.9470); // [02]
    matAlT6082->AddElement(elSi, 0.0102); // [03]
    matAlT6082->AddElement(elTi, 0.0017); // [04]
    matAlT6082->AddElement(elCr, 0.0047); // [05]
    matAlT6082->AddElement(elMn, 0.0120); // [06]
    matAlT6082->AddElement(elFe, 0.0102); // [07]
    matAlT6082->AddElement(elCu, 0.0023); // [08]
    matAlT6082->AddElement(elZn, 0.0048); // [09]
    ///< Epoxy
    TGeoMixture *matEpoxy = new TGeoMixture("Epoxy", 4, 1.25);
    matEpoxy->AddElement(elH,   0.07); // [01]
    matEpoxy->AddElement(elC,   0.64); // [02]
    matEpoxy->AddElement(elO,   0.20); // [03]
    matEpoxy->AddElement(elCl,  0.09); // [04]
    
    TGeoMixture *matFR4 = new TGeoMixture("FR4", 4, 1.85);
    ///< The following composition is indeed the one used for G10 ... could not find a more precise one
    matFR4->AddElement(elC,  0.259);  // [01]
    matFR4->AddElement(elH,  0.288);  // [02]
    matFR4->AddElement(elO,  0.248);  // [03]
    matFR4->AddElement(elSi, 0.205);  // [04]

    ///< ---------------------------------------------------- >
    ///< ------------------------ Media --------------------- >
    ///< ---------------------------------------------------- >
//    // Setting maximum field
//    Double_t fieldm=0.;             // Maximum value of the B field along z (in kG)
//    Int_t    ifield = 0;
//    if(mag_field==0) { 
//        ifield = 0;
//        fieldm = 0.;
//    }
//    if(mag_field==1) {
//        ifield = 3;
//        fieldm = 50.;
//    }
//    if(mag_field==2) { 
//        ifield = 2;
//        fieldm = 50.;
//    }
    ///< Setting field parameters for media
    ///< [0] isvol : Not used
    ///< [1] ifield: User defined magnetic field type (see details here below)
    //      = 0 no magnetic field;
    //      = 1 strongly inhomogeneous magnetic field (returned by the user function GUFLD): tracking performed with the Runge-Kutta method;
    //      = 2 inhomogeneous magnetic field (returned by the user function GUFLD), tracking along a helix;
    //      = 3 uniform magnetic field along the z axis of strength FIELDM, tracking performed along a helix;
    ///< [2] fieldm: Maximum solenoidal field value (in kiloGauss)
    ///< [3] tmaxfd: Maximum angle due to field deflection 
    ///< [4] stemax: Maximum displacement for multiple scattering in one step (in cm) 
    ///< [5] deemax: Maximum fractional energy loss (DLS) in one step
    ///< [6] epsil : Tracking precision (in cm)
    ///< [7] stmin : Minimum step (in cm)
    Int_t    isvol  = 0;
    Int_t    ifield = 0;
    Double_t fieldm = 40.;
    Double_t tmaxfd = 5.;
    Double_t stemax = 0.1;
    Double_t deemax = 0.05;
    Double_t epsil  = 0.0005;
    Double_t stmin  = 0.0010;
    // ALTERNATIVE SET [Really?]
    //    stemax = 0.0010;
    //    deemax = 0.050; 
    //    epsil  = 0.0001;
    //    stmin  = 0.0001;

    // ---> Vacuum [DO THESE PARAMETERS MAKE SENSE? PLEASE CHECK!]
    Double_t parAir[20];
    parAir[0] = -1;     
    parAir[1] = ifield;
    parAir[2] = fieldm;
    parAir[3] = tmaxfd;
    parAir[4] = -0.01;
    parAir[5] = -.3;
    parAir[6] = .001;
    parAir[7] = -.8;
    for ( Int_t i=8; i<20; ++i) parAir[i] = 0.;

    Int_t mediumId  = 1;
    ///< Vacuum
    fmedVacuum = mediumId;
    new TGeoMedium(matVacuum->GetName(), mediumId++, matVacuum, parAir);
    ///< Air
    fmedAir = mediumId;
    new TGeoMedium(matAir->GetName(), mediumId++, matAir, parAir);
    
    // ---> other media [DO THESE PARAMETERS MAKE SENSE? ARE THEY AFFECTING THE SIMULATION? PLEASE CHECK!]
    Double_t parMedium[20];
    parMedium[0] = isvol;
    parMedium[1] = ifield; 
    parMedium[2] = fieldm; 
    parMedium[3] = tmaxfd;
    parMedium[4] = stemax;
    parMedium[5] = deemax;
    parMedium[6] = epsil;
    parMedium[7] = stmin;
    for ( Int_t i=8; i<20; ++i) parMedium[i] = 0.;
    ///< Aluminum
    fmedAl = mediumId;
    new TGeoMedium(matAl->GetName(), mediumId++, matAl, parMedium);
    ///< Copper
    fmedCu = mediumId;
    new TGeoMedium(matCu->GetName(), mediumId++, matCu, parMedium);
    ///< Iron
    fmedFe = mediumId;
    new TGeoMedium(matFe->GetName(), mediumId++, matFe, parMedium);
    ///< Silicon
    fmedSi = mediumId;
    new TGeoMedium(matSi->GetName(), mediumId++, matSi, parMedium);
    ///< Niobium
    fmedNb = mediumId;
    new TGeoMedium(matNb->GetName(), mediumId++, matNb, parMedium);
    ///< Cesium-iodine
    fmedCsI = mediumId;
    new TGeoMedium(matCsI->GetName(), mediumId++, matCsI, parMedium);
    ///< Liquid Helium
    fmedLiqHe = mediumId;
    new TGeoMedium(matLiqHe->GetName(), mediumId++, matLiqHe, parMedium);
    ///< Liquid Nitrogen
    fmedLiqN = mediumId;
    new TGeoMedium(matLiqN->GetName(), mediumId++, matLiqN, parMedium);
    ///< Stainless steel (316LN)
    fmedSteel316LN = mediumId;
    new TGeoMedium(matSteel316LN->GetName(), mediumId++, matSteel316LN, parMedium);
    ///< Copper Niobium Titanium 
    fmedCuNbTi = mediumId;
    new TGeoMedium(matCuNbTi->GetName(), mediumId++, matCuNbTi, parMedium);
    ///< Aluminum T6082
    fmedAlT6082 = mediumId;
    new TGeoMedium(matAlT6082->GetName(), mediumId++, matAlT6082, parMedium);
    ///< Epoxy
    fmedEpoxy = mediumId;
    new TGeoMedium(matEpoxy->GetName(), mediumId++, matEpoxy, parMedium);
    ///< FR4
    fmedFR4 = mediumId;
    new TGeoMedium(matFR4->GetName(), mediumId++, matFR4, parMedium);    
}
//_____________________________________________________________________________
void a2mcApparatus::SetCuts()
{
    /// Set cuts for e-, gamma equivalent to 1mm cut in G4.

    Int_t mediumId = gMC->MediumId("Silicon");
    if ( mediumId ) {
        gMC->Gstpar(mediumId, "CUTGAM", 10.e-06);
        gMC->Gstpar(mediumId, "BCUTE",  10.e-06);
        gMC->Gstpar(mediumId, "CUTELE", 597.e-06);
        gMC->Gstpar(mediumId, "DCUTE",  597.e-06);
    }

    mediumId = gMC->MediumId("Air");
    if ( mediumId ) {
        gMC->Gstpar(mediumId, "CUTGAM", 990.e-09);
        gMC->Gstpar(mediumId, "BCUTE",  990.e-09);
        gMC->Gstpar(mediumId, "CUTELE", 990.e-09);
        gMC->Gstpar(mediumId, "DCUTE",  990.e-09);
    }
}

//_____________________________________________________________________________
Int_t a2mcApparatus::SilModPos(UInt_t lay, UInt_t mod, Double_t& phi1, Double_t& phi2, Double_t& x, Double_t& y, Double_t& z)
{
///< It calculate the position variables (phi1, phi2, x, y, and z) for each silicon module
    ///< These formulas were calculated using the values found in detector2_geo.xml (AlphaSoftware2020/aux/geo)
    if(lay>=nLayers||mod>=nModules[lay]) {
        return -1;
        cout << "a2mcApparatus::SilModPos --> Please check combination of (layer, module) [" << lay << "," << mod << "]" << endl;
    }
    ///< #####################################
    ///< Setting angular and radial parameters
    Double_t phi_delta = 360./nModules[lay];
    Double_t phi1_start =0., phi2_start =0.;
    Double_t abs_z = 11.5;
    Double_t r_even = 0., r_odd = 0., r = 0.;
    if(lay==0||lay==3) {
        phi1_start = 90.; phi2_start =  180.;
        r_even = 8.9; r_odd = 9.45;
    } else if(lay==1||lay==4) {
        phi1_start = 99.; phi2_start = -171.;
        r_even = 10.8; r_odd = 11.35;
    } else if(lay==2||lay==5) {
        phi1_start = 95.; phi2_start = -175.;
        r_even = 12.7; r_odd = 13.25;
    } else {
        return -1;
        cout << "a2mcApparatus::SilModPos --> Please check combination of (layer, module) [" << lay << "," << mod << "]" << endl;
    }
    ///< ####################################
    ///< Calculating the angles phi1 and phi2
    ///< Calculating the x, y and z coordinates
    ///< phi1 and phi2 are between -180 and + 180
    phi1 = phi1_start + phi_delta*mod;         if(phi1>180) phi1 -= 360.;
    phi2 = phi2_start + phi_delta*mod;         if(phi2>180) phi2 -= 360.;
    ///< x, y, z
    if(mod%2==0) {r = r_even;} else {r = r_odd;};
    x = r*TMath::Cos(phi1*TMath::DegToRad());
    y = r*TMath::Sin(phi1*TMath::DegToRad());
    z = lay<=2? -abs_z : +abs_z; ////< First 3 layers have negative z, the other positive z

    Double_t lower_limit = 1.e-12;
    if(fabs(x)<lower_limit) x = 0.;
    if(fabs(y)<lower_limit) y = 0.;
    if(fabs(z)<lower_limit) z = 0.;
    ///< id --> see legend/map at the end of a2mcApparatus.h
    UInt_t start = 0;
    for(UInt_t i=0; i<lay; i++) {
        start += nModules[i];
    }
    return start + mod;
}
