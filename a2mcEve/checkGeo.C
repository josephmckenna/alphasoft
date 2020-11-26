///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include <Riostream.h>
#include <iostream>

#include "TEveManager.h"
#include "TEveGeoNode.h"

#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TGeoMedium.h"

#include <TFile.h>
#include <vector>

Bool_t makeGeo(Int_t);

void checkGeo(Int_t runNumber=0)
{
	/******************************************************************************/
	//             CHECKING THE GEOMETRY
	/******************************************************************************/
	//         Getting the geometry from output file
	//-----------------------------------------------------------------------------

	TEveManager::Create();

	///< Building the simple "gentle" geometry (and saving into a "new" file)
	Bool_t geo = makeGeo(runNumber);
    if(!geo) {
        cout << "checkGeo --> could not make geometry - please check file name/run number" << endl;
        return;
    }
    ///< Reading "gentle" geometry file generated with makeGeo
    ostringstream gEveGeoInput;
    gEveGeoInput << "../output/a2mcEveApparatus-" << runNumber << ".root";
	TFile* geom = TFile::Open(gEveGeoInput.str().c_str());

	TIter next(gDirectory->GetListOfKeys());
	TKey* key;
	TString shape("TEveGeoShapeExtract");

	while ((key = (TKey*) next())){
		if (shape == key->GetClassName())
		{
			TEveGeoShapeExtract* gse = (TEveGeoShapeExtract*) key->ReadObj();
			TEveGeoShape* gGeomGentle = TEveGeoShape::ImportShapeExtract(gse, 0);
			gEve->AddGlobalElement(gGeomGentle);
		}
	}

	// Getting the volume of interest

	// Visualization 
	TEveElement* top = gEve->GetCurrentEvent();
	gEve->FullRedraw3D(kTRUE, kTRUE);
	auto gs = gEve->GetGlobalScene();
	gs->SetPickableRecursively(1);
	auto v = gEve->GetDefaultGLViewer();
//   	v->GetClipSet()->SetClipType(TGLClip::EType(1)); ///< For a "sliced" view (sezione)
    double pos[3] = {0., 0., 0.};
    v->SetGuideState(TGLUtil::kAxesEdge, kTRUE, kTRUE, pos);
   	v->RefreshPadEditor(v);

   	v->CurrentCamera().RotateRad(+0.5, -1.2);
	v->DoDraw();

}

Bool_t makeGeo(Int_t runNumber=0) {

/*****************************************************************************************/
//			       MAKING  SIMPLE  GEOMETRY
/*****************************************************************************************/

//-----------------------------------------------------------------------------------------
// Getting the geometry from output file and making the gentle geometry (symple geometry)
//-----------------------------------------------------------------------------------------

    gSystem->Load("libGeom");

// to run after 03/03/2016 (run with time-Nrun information)
    std::ostringstream sgeo;
    sgeo << "ls ../output/a2mcApparatus-" << runNumber << ".root";
    TString file_name(gSystem->GetFromPipe(sgeo.str().c_str()));
    string sfile = file_name.Data();
    if(strcmp(sfile.c_str(),"")==0) {
        cout << "Please check presence of file " << runNumber << endl;
        return false;
    }           
    gGeoManager = TGeoManager::Import(file_name);

    ostringstream gEveGeoOutput;
    gEveGeoOutput << "../output/a2mcEveApparatus-" << runNumber << ".root";

    gGeoManager->CheckOverlaps(0.01); 
    gGeoManager->PrintOverlaps(); 

    TGeoNode* topNode = gGeoManager->GetTopNode();
    TEveGeoTopNode* eveTopNode = new TEveGeoTopNode(gGeoManager, topNode);
    eveTopNode->SetVisOption(0);
    eveTopNode->SetVisLevel(6);
    eveTopNode->GetNode()->GetVolume()->SetVisibility(kFALSE);
    eveTopNode->GetNode()->GetVolume()->VisibleDaughters(kTRUE);
    gEve->AddElement(eveTopNode);  

//gEve->Redraw3D(kTRUE); // Reset camera after the first event has been shown.

    eveTopNode->ExpandIntoListTreesRecursively();
    eveTopNode->SaveExtract(gEveGeoOutput.str().c_str(),"Gentle",kFALSE); 

    gEve->GetCurrentEvent()->DestroyElements();
    return true;
}
