#include "../include/TSiliconModule.h"

// TSiliconModule Class =====================================================================================
//
// Class representing a single Si strip.
//
// JWS 10/10/2008
//
// ==========================================================================================================

ClassImp(TSiliconModule);

TSiliconModule::TSiliconModule()
{
  ModuleNumber=-999;
  VF48ModuleNumber=-999;
  VF48GroupNumber=-999;
  FRCNumber=-999;
  HitModule=false;
//  ASICs = new TObjArray();
}

TSiliconModule::TSiliconModule( Int_t  _ModuleNumber, Int_t _VF48ModuleNumber, Int_t _VF48GroupNumber,  Int_t _FRCNumber, Int_t _FRCPortNumber  )
{
  ModuleNumber = _ModuleNumber;
  VF48ModuleNumber = _VF48ModuleNumber;
  VF48GroupNumber = _VF48GroupNumber;
  FRCNumber = _FRCNumber;
  FRCPortNumber = _FRCPortNumber;
  HitModule=false;

  ASICs = new TObjArray();
}

TSiliconModule::TSiliconModule( TSiliconModule* & SiliconModule )
{
  ModuleNumber           = SiliconModule->GetModuleNumber();
  VF48ModuleNumber       = SiliconModule->GetVF48ModuleNumber();
  VF48GroupNumber        = SiliconModule->GetVF48GroupNumber();
  FRCNumber              = SiliconModule->GetFRCNumber();
  FRCPortNumber          = SiliconModule->GetFRCPortNumber();
  HitModule              = SiliconModule->IsAHitModule();
  ASICs->SetOwner(kTRUE);
  ASICs->Delete();
  delete ASICs;
  ASICs = new TObjArray( *(SiliconModule->GetASICs()) );
  //copy ASICs....

}

TSiliconModule::~TSiliconModule()
{
  if( ASICs )
    {
      ASICs->SetOwner(kTRUE);
      ASICs->Delete();
      delete ASICs;
    }
}

TSiliconVA* TSiliconModule::GetASIC( Int_t number )
{
  TSiliconVA* ASIC = NULL;
  if( ASICs == NULL )
    {
      return ASIC;
    }
  Int_t ASICEntries=ASICs->GetEntriesFast();
  
  if( number > ASICEntries )
    {
      //  return ASIC;
    }

  for( Int_t i = 0; i < ASICEntries; i++ )
    {
      ASIC = (TSiliconVA*) ASICs->At(i);
      if(!ASIC)
        continue;
      if( ASIC->GetASICNumber() == number )
        return ASIC;
    }
  
  return NULL;
}

void TSiliconModule::Print()
{
  for( Int_t i=0; i<ASICs->GetEntries(); i++ )
    {
      TSiliconVA* VA = (TSiliconVA*) ASICs->At(i);
      if(VA)
        {
          VA->Print();
        }
      VA=NULL;
    }
}

void TSiliconModule::PrintToFile( FILE * f )
{
  for( Int_t i=0; i<ASICs->GetEntries(); i++ )
    {
      TSiliconVA* VA = (TSiliconVA*) ASICs->At(i);
      if(VA)
        {
          //fprintf( f, "| %d ", ModuleNumber);
          VA->PrintToFile( f, ModuleNumber );
        }
      VA=NULL;
    }
}


void TSiliconModule::PrintVAORs()
{
  TSiliconVA* VA = NULL;

  for( Int_t i=0; i<ASICs->GetEntries(); i++ )
    {
      VA = (TSiliconVA*) ASICs->At(i);
      if( VA )
        {
          if( VA->IsAHitOR() ) printf("SiMOD %d VA %d OR TRUE \n", ModuleNumber, i );  
        }
      VA = NULL;
    }
}

void TSiliconModule::ClearVAs()
{
  Int_t NumberOfASICs = ASICs->GetEntriesFast();

  for( Int_t i=0; i<NumberOfASICs; i++ )
    {
      TSiliconVA* VA = (TSiliconVA*) ASICs->At(i);
      if( VA )
        {
          delete VA;
          VA = NULL;
        }
    }
  ASICs->SetOwner();
  ASICs->Delete();
  ASICs->Clear();
}

Int_t TSiliconModule::CompressVAs()
{

  TSiliconVA* VA;
  Int_t NumberOfASICs = ASICs->GetEntriesFast();
  ASICs->SetOwner(kTRUE);
  for( Int_t i=0; i<NumberOfASICs; i++ )
    {
      VA = (TSiliconVA*) ASICs->At(i);

      if( !VA ) continue;
      
      if(VA->IsAHitOR() )
        {           
          VA->CompressStrips();         
          HitModule = true;
        }
      else 
        {
          VA->Delete();
          ASICs->RemoveAt(i);
        }

    }//loop over ASICs

  ASICs->Compress();

  return 0;
}

Int_t TSiliconModule::CalcNRawHits()
{
  Int_t NRawHits(0);
  
  TSiliconVA* VA=new TSiliconVA();
  Int_t NumberOfASICs = ASICs->GetEntriesFast();

  for( Int_t i=0; i<NumberOfASICs; i++ )
    {
      VA = (TSiliconVA*) ASICs->At(i);
      if( VA ) NRawHits+=VA->CalcNRawHits();

    }//loop over ASICs
  delete VA;
  return NRawHits;
}
