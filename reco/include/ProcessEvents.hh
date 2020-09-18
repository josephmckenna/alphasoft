//
// Process Events
// (with one-click)
// 
// Author: A. Capra
// Date: June 2020
//

#ifndef __PROCEVT__
#define __PROCEVT__ 1


#include "TClonesArray.h"
#include "TVector3.h"

#include "AnaSettings.h"

#include "Deconv.hh"
#include "Ledge.hh"
#include "Match.hh"
#include "Reco.hh"
#include "Utils.hh"

class ProcessEvents
{
private:
   Deconv d;
   Ledge leaw;
   Ledge lepad;
   Match m;
   Reco r;
   Reco rMC;
   Utils u;

   finderChoice kFinder;
   int EventNo;
   bool kDraw;
   int kVerb;

public:

   ProcessEvents(AnaSettings*,double,std::string,bool sim=true);

   void SetFinder(finderChoice fc) {kFinder=fc;}
   void SetEventNumber(int n) {EventNo=n;}
   void SetDraw();
   void SetVerboseLevel(int v) {kVerb=v;}

   void ProcessWaveform_deconv(TClonesArray*,TClonesArray*);
   void ProcessWaveform_led(TClonesArray*,TClonesArray*);
   void ProcessPoints();
   void ProcessWaveform_2D(TClonesArray*);
   void ProcessTracks();
   void ProcessMonteCarlo(TClonesArray*,TVector3*);
   void ProcessVertex(TVector3*);

   void Finish();
   void Finish(TClonesArray*,TClonesArray*);


};
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
