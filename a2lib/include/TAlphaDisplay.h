#ifndef TAlphaDisplay_H
#define TAlphaDisplay_H

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//    TAlphaDisplay                                                      //
//                                                                       //
//    Utility class for the ALPHA   Event display (Hits & Tracks)        //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include <TNamed.h>
#include <TCanvas.h>
#include <TString.h>

#include "TAlphaEvent.h"
#include "TAlphaEventHelix.h"
#include "TAlphaEventCosmicHelix.h"
#include "TAlphaEventVertex.h"

class TCanvas;
class TPad;
class TButton;
class TAlphaEvent;

class TAlphaDisplay : public TNamed {
     
 private:
     TCanvas    *fCanvas;                  // pointer to display canvas
     TPad       *fPad;                     // pointer to event display main pad
     TPad       *fButtons;                 // pointer to buttons pad
     TPad       *fDetSel;                  // pointer to trigger pad
     
     TButton    *fButSup;
     Bool_t      fViewSup;
     TButton    *fButOne;
     Bool_t      fViewOne;
     TButton    *fButTwo;
     Bool_t      fViewTwo;
     TButton    *fButThree;
     Bool_t      fViewThree;
     TButton    *fButHOnly;
     Bool_t      fViewHOnly;
     TButton    *fButMCData;
     Bool_t      fViewMCData;
     TButton    *fButRecData;
     Bool_t      fViewRecData;
     TButton    *fButShowAllSil;
     Bool_t      fViewShowAllSil;
     TButton    *fButShowAllTracks;
     Bool_t      fViewShowAllTracks;
     Bool_t      fOGLColourScheme;

    int fAutoSaveOnDisplay;
    int fRunNo;

     TString     fCaption;
    TAlphaEvent* fCurrentEvent;
     virtual void DrawView(Float_t theta, Float_t phi, Float_t psi, Option_t *title=NULL);
     
 public:
     TAlphaDisplay() {}        
     TAlphaDisplay( TString text, Int_t autoSaveOnDisplay = 0, Int_t runNum = 0);  
     void SetEventPointer(TAlphaEvent* ev)
     {
         fCurrentEvent=ev;
     }
     virtual ~TAlphaDisplay(); 
     
     virtual void DrawAllViews();
     virtual void SetView(Float_t theta, Float_t phi, Float_t psi, Option_t *title=NULL);
     virtual void DrawViewX3D();
     virtual void DrawViewOGL();

     virtual void NextEvent();
     
     virtual void ToggleOne();
     virtual void ToggleTwo();  
     virtual void ToggleThree();
     virtual void ToggleSup();
     virtual void ToggleHitOnlyDet();
     virtual void ToggleMCData();
     virtual void ToggleRecData();
     virtual void ToggleShowAllSil();
     virtual void ToggleShowAllTracks();
     virtual void Update() { fCanvas->Update(); } 
     
     void SetCaption(TString caption) { fCaption = caption; }
     
     void DrawHelix( TAlphaEventHelix * Helix, Bool_t debug );
     void DrawHelix( TAlphaEventCosmicHelix * Helix, Bool_t debug );
     void DrawCosmicTrack(TAlphaEventTrack *cosmic);

     ClassDef(TAlphaDisplay,0);  //  Utility class for the ALPHA  Event display (Hits & Tracks)
};

#endif
