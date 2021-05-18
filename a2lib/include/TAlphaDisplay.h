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

#include "TSiliconEvent.h"
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

     int fRunNo;

     TString     fCaption;
     TAlphaEvent* fCurrentTAlphaEvent;
     TSiliconEvent* fCurrentTSilEvent;
     virtual void DrawView(Float_t theta, Float_t phi, Float_t psi, Option_t *title=NULL);
     
 public:
     TAlphaDisplay() {}        
     TAlphaDisplay( Int_t runNum = 0);  
     void SetEventPointer(TAlphaEvent* ev, TSiliconEvent* sil)
     {
         //Used for render
         fCurrentTAlphaEvent = ev;
         //Used for event number / VF48 time
         fCurrentTSilEvent = sil;
     }
     virtual ~TAlphaDisplay(); 
     
     virtual void UpdateText();
     virtual void DrawAllViews();
     virtual void Save(std::string FileName = "");
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
