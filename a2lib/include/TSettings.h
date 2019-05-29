#ifndef _TSettings_
#define _TSettings_

#include "TObject.h"
#include <TString.h>
#include <TBuffer.h>
#include "sqlite3.h"

#include "SiMod.h"

// TSettings Class =========================================================================================
//
// Class providing access functions for the sqlite daatabase. 
//
// RAH/JWS 10/06/2010
//
// =========================================================================================================

class TSettings : public TObject
{
private:
  sqlite3 * fdb;
  Double_t fvf48freq[nVF48];
  Int_t    fvf48samples[nVF48];
  Int_t    fsoffset[nVF48];
  Double_t fsubsample[nVF48];
  Int_t    foffset[nVF48];

public:
  TSettings();
  TSettings( Char_t * dbname );
  TSettings( Char_t * dbname, Int_t run );
  ~TSettings();

  TString  ExeSQL_singlereturn( char * sql );

  Double_t GetVF48Frequency( Int_t run, Int_t modulenumber );
  Int_t    GetVF48Samples( Int_t run, Int_t modulenumber );
  Int_t    GetVF48soffset( Int_t run, Int_t modulenumber );
  Double_t GetVF48subsample( Int_t run, Int_t modulenumber );
  Int_t    GetVF48offset( Int_t run, Int_t modulenumber );

  Double_t GetVF48Frequency( Int_t modulenumber ) { return fvf48freq[modulenumber]; }
  Double_t GetVF48Samples( Int_t modulenumber )   { return fvf48samples[modulenumber]; }
  Int_t    GetVF48soffset( Int_t modulenumber )   { return fsoffset[modulenumber]; }
  Double_t GetVF48subsample( Int_t modulenumber ) { return fsubsample[modulenumber]; }
  Int_t    GetVF48offset( Int_t modulenumber )    { return foffset[modulenumber]; }

  TString GetVF48MapDir();
  TString GetSiRMSDir();
  TString GetDetectorGeoDir();

  TString GetVF48Map( Int_t run );
  TString GetSiRMS( Int_t run );
  TString GetDetectorGeo( Int_t run );
  TString GetDetectorEnv( Int_t run );
  TString GetDetectorMat( Int_t run );
  TString GetDumpName(Int_t run, Int_t dumpnum);

  // inline TBuffer & operator>>(TBuffer &,TSettings *&);
  // inline TBuffer & operator<<(TBuffer &,const TSettings *);

  ClassDef(TSettings,1)
};



#endif
