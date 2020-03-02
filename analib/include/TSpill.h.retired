#ifndef _TSpill_
#define _TSpill_

#include <time.h>

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif

#include "TSeq_Dump.h"
#include "TCanvas.h"
#include "TText.h"

#define MAXDET 10

class TSpill : public TObject
{
private:
  Int_t     fRunNumber;
  Int_t     fNum;
  TObjArray fDumps;
  TObjArray fFlaggedDumps;
  Double_t  fTransformer;
  
  Bool_t    fIsMessage;
  TString   fMessage;
  time_t    fTime;
  
  Double_t    fYStep;
  Int_t    fNDet;

public:
  
  TSpill();
  TSpill( Int_t runnumber, Int_t num, time_t time, int ndet=5);
  TSpill( const char * message );
  virtual ~TSpill();
  
  void AddText(TCanvas * c, double x, double y, double textsize, int colour, const char* format, ...);
  void PrintSpill( TCanvas *c, double x, double y, Int_t colour );

  Bool_t IsMessage() { return fIsMessage; }
  
  void SetYStep(Double_t ystep) { fYStep = ystep; }
  Double_t GetYStep() {return fYStep;} 

  void SetNDet(Int_t ndet) { fNDet = ndet; }
  Int_t GetNDet() {return fNDet;} 

  Int_t GetRunNumber() { return fRunNumber; }
  Int_t GetNum() { return fNum; }
  Double_t GetTransformer() { return fTransformer; }
  TString GetMessage() { return fMessage; }
  time_t  *GetTime() { return &fTime; }

  void SetNum( Int_t num ) { fNum = num; }
  void SetTransformer( Double_t trans ) { fTransformer = trans; }
  void SetRunNumber( Int_t run ) { fRunNumber = run; }

  void AddDump( TSeq_Dump * dump ) { fDumps.AddLast( dump ); }
  TSeq_Dump * GetDump( Int_t i ) { return (TSeq_Dump*) fDumps.At(i); }
  Int_t GetNumDump() { return fDumps.GetEntries(); }

  void AddFlaggedDump( TSeq_Dump * dump ) { fFlaggedDumps.AddLast( dump ); }
  TSeq_Dump * GetFlaggedDump( Int_t i ) { return (TSeq_Dump*) fFlaggedDumps.At(i); }
  Int_t GetFlaggedNumDump() { return fFlaggedDumps.GetEntries(); }

  void FormatADInfo(TString* log);
  void FormatDumpInfo(TString* log, TSeq_Dump* d, Bool_t indent);

  ClassDef( TSpill, 1 )
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
