#ifndef __TSiliconModule__
#define __TSiliconModule__

// TSiliconModule Class =========================================================================================
//
// Class representing a single Silicon module.
//
// JWS 10/10/2008
//
// ==============================================================================================================

#include "TSiliconVA.h"

class TSiliconModule : public TObject {

private:  
  Int_t ModuleNumber;
  Int_t VF48ModuleNumber;
  Int_t VF48GroupNumber;
  Int_t FRCNumber;
  Int_t FRCPortNumber;
  Bool_t HitModule;

  std::vector<TSiliconVA*> ASICs;

public:
  TSiliconModule();
  TSiliconModule( const Int_t _ModuleNumber, const Int_t _VF48ModuleNumber, const Int_t _VF48GroupNumber, const Int_t _FRCNumber, const Int_t FRCPortNumber );
  TSiliconModule( TSiliconModule* & );
  virtual ~TSiliconModule();

  // getters
  Int_t GetModuleNumber( ){ return ModuleNumber; }
  Int_t GetVF48ModuleNumber( ){ return VF48ModuleNumber; }
  Int_t GetVF48GroupNumber( ){ return VF48GroupNumber; }
  Int_t GetFRCNumber( ){ return FRCNumber; }
  Int_t GetFRCPortNumber( ){ return FRCPortNumber; }
  TSiliconVA* GetASIC( const Int_t number );
  Int_t GetNumberOfASICs( ){ return ASICs.size(); }
  Bool_t IsAHitModule( ){ return HitModule; }
  std::vector<TSiliconVA*>  GetASICs( ){ return ASICs; }
  
  // setters
  void SetModuleNumber( Int_t _ModuleNumber ){ ModuleNumber = _ModuleNumber; }
  void SetVF48ModuleNumber( Int_t _VF48ModuleNumber ){ VF48ModuleNumber = _VF48ModuleNumber; }
  void SetVF48GroupNumber( Int_t _VF48GroupNumber ){ VF48GroupNumber = _VF48GroupNumber; }
  void SetFRCNumber( Int_t _FRCNumber ){ FRCNumber = _FRCNumber; }
  void SetFRCPortNumber( Int_t _FRCPortNumber ){ FRCPortNumber = _FRCPortNumber; }
  void AddASIC( TSiliconVA* ASIC ){ ASICs.push_back(ASIC); }

  using TObject::Print;
  virtual void Print();
  void PrintToFile( FILE * f );
  void PrintVAORs();
  void ClearVAs();
  Int_t CompressVAs();
  Int_t CalcNRawHits();

  ClassDef(TSiliconModule,2)
};

#endif
