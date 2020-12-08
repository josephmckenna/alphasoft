#include "RootUtils.h"
#include "TSeq_Event.h"

TString Get_Chrono_Name(Int_t runNumber, Int_t ChronoBoard, Int_t Channel);
TString Get_Chrono_Name(TSeq_Event* e);
TString SequenceQODDetectorLine(Int_t runNumber,Double_t tmin, Double_t tmax, Int_t* boards[], Int_t* channels[], Int_t nChannels);

TString MakeAutoPlotsFolder(TString subFolder);
TString MakeAutoPlotsFolder(TString subFolder,TString rootdir);
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
