///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################
///< This macro is a modification of the code contained in .../root/tutorials/eve/alice_vsd.C
///< properly modified for a2mc. It is based on ROOT VSD:
//
//   TEveVSD Class Reference
//   Graphics » 3D Graphics » Event Display
//   Visualization Summary Data - a collection of trees holding standard 
//                                event data in experiment independent format
///< ------------------ comment from alice_vsd.C ----------------------------
/// \author Matevz Tadel
///  Only standard ROOT is used to process the VSD files.
///
///  No ALICE code is needed -- the VSD file is exported from AliRoot into
///  VSD format -- see TEveVSDStructs.h and TEveVSD.h.
///< ------------------------------------------------------------------------

#include <TEveManager.h>
#include <TEveEventManager.h>
#include <TEveVSD.h>
#include <TEveVSDStructs.h>

#include <TEveTrack.h>
#include <TEveTrackPropagator.h>
#include <TEveGeoShape.h>

#include <TGTab.h>
#include <TGButton.h>

#include <TFile.h>
#include <TKey.h>
#include <TSystem.h>
#include <TPRegexp.h>


// Include components -- compile time link
#include "a2mcMultiView.C"
a2mcMultiView* gMultiView = 0;
#include "a2mcVSDReader.C"
a2mcVSDReader* gVSDReader = 0;
#include "a2mcToVSD.C"
a2mcToVSD* gToVSD = 0;

Int_t nMaxEvents = 0;
ULong_t blue,green,yellow,black,violet,red;     // Define some colors
// Forward declaration.
void    init();
void    make_gui();
Bool_t  make_geo(Int_t);
//______________________________________________________________________________
void runViewer(Int_t runNumber=0) {
    
///< ------------------ comment from alice_vsd.C --------------------------
   // Main function, initializes the application.
   //
   // 1. Load the auto-generated library holding ESD classes and
   //    ESD dictionaries.
   // 2. Open ESD data-files.
   // 3. Load cartoon geometry.
   // 4. Spawn simple GUI.
   // 5. Load first event.
///< ----------------------------------------------------------------------
    init();
///< --------------------------------------------
///< 0 - INITIALIZATION
///< --------------------------------------------
    ///< Initial settings
    //=====================
    TFile::SetCacheFileDir(".");
    TEveVSD::DisableTObjectStreamersForVSDStruct();

    ///< Creating the EVE manager 
   //=====================
    TEveManager::Create();

///< --------------------------------------------
///< 1 - READING THE GEOMETRY
///< --------------------------------------------
    TEveGeoShape *gentle_geom = 0;
    ///< Writing "gentle" geometry file with make_geo
    //=====================
	Bool_t geo = make_geo(runNumber);
    if(!geo) {
        cout << "checkGeo --> could not make geometry - please check file name/run number" << endl;
        return;
    }
    ///< Reading "gentle" geometry file generated with make_geo
    //=====================
    ostringstream gEveGeoInput;
    gEveGeoInput << "./root/a2mcEveApparatus-" << runNumber << ".root";
    auto geom = TFile::Open(gEveGeoInput.str().c_str());
    if (!geom) return;
    auto gse = (TEveGeoShapeExtract*) geom->Get("Gentle");
    gentle_geom = TEveGeoShape::ImportShapeExtract(gse, 0);
    geom->Close();
    delete geom;
    gEve->AddGlobalElement(gentle_geom);

///< --------------------------------------------
///< 2 - READING THE VSD DATA
///< --------------------------------------------
    ///< Creating the VSD data file (from the MC output data file to the VSD data format)
    //=====================
    gToVSD = new a2mcToVSD(runNumber);
    gToVSD->WriteVSD();
    nMaxEvents = gToVSD->fTotEvents;
    delete gToVSD;
    
    ///< Reading the VSD data file 
    //=====================
    ostringstream vsd_file_name;
    vsd_file_name << "root/a2mcVSD_" << runNumber << ".root";
    gVSDReader = new a2mcVSDReader(vsd_file_name.str().c_str());

///< --------------------------------------------
///< 3 - SETTING THE VISUALIZATION
///< --------------------------------------------
    // Standard multi-view
    //=====================
    gMultiView = new a2mcMultiView;
    gMultiView->f3DView->GetGLViewer()->SetStyle(TGLRnrCtx::kOutline);
    
    gMultiView->SetDepth(-10);
    gMultiView->ImportGeomRPhi(gentle_geom);
    gMultiView->ImportGeomRhoZ(gentle_geom);
    gMultiView->SetDepth(0);

    // Multi-view settings
    //=============
    gEve->GetViewers()->SwitchColorSet();
    gEve->GetDefaultGLViewer()->SetStyle(TGLRnrCtx::kOutline);
    gEve->GetBrowser()->GetTabRight()->SetTab(1);
//    auto camBase  = gEve->GetDefaultGLViewer()->CurrentCamera().GetCamBase();
//    auto camTrans = gEve->GetDefaultGLViewer()->CurrentCamera().GetCamTrans();

    // Creating and setting the GUI
    //=============
    make_gui();
    gEve->AddEvent(new TEveEventManager("Event", "A2MC VSD Event"));
    gVSDReader->GotoEvent(0);
    gEve->GetDefaultGLViewer()->CurrentCamera().IncTimeStamp();
    gEve->Redraw3D(kTRUE); // Reset camera after the first event has been shown.
}

//______________________________________________________________________________
void make_gui() {
   // Create minimal GUI for event navigation.
    auto browser = gEve->GetBrowser();
    browser->StartEmbedding(TRootBrowser::kLeft);

    auto frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
    frmMain->SetWindowName("XX GUI");
    frmMain->SetCleanup(kDeepCleanup);

    ///< #######################################
    ///< Top Frame with the number of events and the "Next" and "Previous" event buttons
    ///< #######################################
    auto topFrame = new TGHorizontalFrame(frmMain);

    TString icon_dir(TString::Format("%s/icons/", gSystem->Getenv("ROOTSYS")));

    //=============
    ///< Number of events in the run/sample
    TGTextEntry        *nEvents;
    nEvents = new TGTextEntry(frmMain);
    nEvents->SetEnabled(kTRUE);
    topFrame->AddFrame(nEvents, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 10,10,5,5));
    nEvents->SetTextColor(0x0033FF);
    nEvents->SetText(Form("Tot. Events: %d",nMaxEvents));

    //=============
    // Previous and Next event buttons
    TGPictureButton* b = 0;
    b = new TGPictureButton(topFrame, gClient->GetPicture(icon_dir+"GoBack.gif"));
    topFrame->AddFrame(b);
    b->Connect("Clicked()", "a2mcVSDReader", gVSDReader, "PrevEvent()");

    b = new TGPictureButton(topFrame, gClient->GetPicture(icon_dir+"GoForward.gif"));
    topFrame->AddFrame(b);
    b->Connect("Clicked()", "a2mcVSDReader", gVSDReader, "NextEvent()");

    frmMain->AddFrame(topFrame);

	// adding a 3D line to separate frames each other
	TGHorizontal3DLine *separator0 = new TGHorizontal3DLine(frmMain);

	frmMain->AddFrame(separator0, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

///< GOTO FRAME - INIT
//    TGVerticalFrame    *gotoframe;
//    TGNumberEntry *numev;
//    gotoframe = new TGVerticalFrame(frmMain,200,200);
//	{
//        numev = new TGNumberEntry(gotoframe, 0, 9,999, TGNumberFormat::kNESInteger,
//                         TGNumberFormat::kNEANonNegative,
//                         TGNumberFormat::kNELLimitMinMax,
//                         0, 99999);
//		// Number of events view on go to frame
//		TGButton  *gotoEv = new TGTextButton(gotoframe, "Go To Event");
//		gotoframe->AddFrame(gotoEv, new TGLayoutHints(kLHintsTop | kLHintsExpandX,2, 0, 2, 2));
//		gotoEv->ChangeBackground(green);
//        ostringstream s; s << "GotoEvent(Int_t=" << numev->GetNumberEntry()->GetIntNumber() << ")";
//        gotoEv->Connect("Clicked()", "a2mcVSDReader", gVSDReader, s.str().c_str());
//        numev->Connect("ValueSet(int)", "a2mcVSDReader", gVSDReader, s.str().c_str());
//        numev->GetNumberEntry()->Connect("ReturnPressed()", "a2mcVSDReader", gVSDReader, s.str().c_str());
//        gotoframe->AddFrame(numev, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
//        TGGroupFrame *fGframe = new TGGroupFrame(gotoframe, "Going to event .. ");
//        TGLabel *fgoto = new TGLabel(fGframe, "No input.");
//        fGframe->AddFrame(fgoto, new TGLayoutHints(kLHintsTop | kLHintsLeft,5, 5, 5, 5));
//        gotoframe->AddFrame(fGframe, new TGLayoutHints(kLHintsExpandX, 2, 2, 1, 1));
//	}
//
//	frmMain->AddFrame(gotoframe);
//
///< GOTO FRAME - END
    
    frmMain->MapSubwindows();
    frmMain->Resize();
    frmMain->MapWindow();

    browser->StopEmbedding();
    browser->SetTabTitle("Event Control", 0);
}

Bool_t make_geo(Int_t runNumber=0) {

/*****************************************************************************************/
//			       MAKING  SIMPLE  GEOMETRY
/*****************************************************************************************/

//-----------------------------------------------------------------------------------------
// Getting the geometry from output file and making the gentle geometry (symple geometry)
//-----------------------------------------------------------------------------------------

    gSystem->Load("libGeom");

    // Reading the MC geometry output file
    //=============
    std::ostringstream sgeo;
    sgeo << "ls ../output/a2mcApparatus-" << runNumber << ".root";
    TString file_name(gSystem->GetFromPipe(sgeo.str().c_str()));
    string sfile = file_name.Data();
    if(strcmp(sfile.c_str(),"")==0) {
        cout << "Please check presence of file " << runNumber << endl;
        return false;
    }           
    gGeoManager = TGeoManager::Import(file_name);

    // Writing the MC EVE geometry (gentle) file
    //=============
    ostringstream gEveGeoOutput;
    gEveGeoOutput << "./root/a2mcEveApparatus-" << runNumber << ".root";

    TGeoNode* topNode = gGeoManager->GetTopNode();
    TEveGeoTopNode* eveTopNode = new TEveGeoTopNode(gGeoManager, topNode);
    eveTopNode->SetVisOption(0);
    eveTopNode->SetVisLevel(6);
    eveTopNode->GetNode()->GetVolume()->SetVisibility(kFALSE);
    eveTopNode->GetNode()->GetVolume()->VisibleDaughters(kTRUE);
    gEve->AddElement(eveTopNode);  

    eveTopNode->ExpandIntoListTreesRecursively();
    ///< Actual command to write the geometry out
    eveTopNode->SaveExtract(gEveGeoOutput.str().c_str(),"Gentle",kFALSE); 

    gEve->GetCurrentEvent()->DestroyElements();
    return true;
}

void init() {
///< Setting some colors
	gClient->GetColorByName("blue", blue);
	gClient->GetColorByName("yellow", yellow);
	gClient->GetColorByName("green", green);
	gClient->GetColorByName("black", black);
	gClient->GetColorByName("violet", violet);
	gClient->GetColorByName("red", red);    
}
