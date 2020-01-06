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
  ASICs.reserve(4);
//  ASICs = new TObjArray();
}

TSiliconModule::TSiliconModule(const Int_t  _ModuleNumber, const Int_t _VF48ModuleNumber, const Int_t _VF48GroupNumber,  const Int_t _FRCNumber, const Int_t _FRCPortNumber  )
{
  ModuleNumber = _ModuleNumber;
  VF48ModuleNumber = _VF48ModuleNumber;
  VF48GroupNumber = _VF48GroupNumber;
  FRCNumber = _FRCNumber;
  FRCPortNumber = _FRCPortNumber;
  HitModule=false;
  ASICs.reserve(4);
}

TSiliconModule::TSiliconModule( TSiliconModule* & SiliconModule )
{
  ModuleNumber           = SiliconModule->GetModuleNumber();
  VF48ModuleNumber       = SiliconModule->GetVF48ModuleNumber();
  VF48GroupNumber        = SiliconModule->GetVF48GroupNumber();
  FRCNumber              = SiliconModule->GetFRCNumber();
  FRCPortNumber          = SiliconModule->GetFRCPortNumber();
  HitModule              = SiliconModule->IsAHitModule();
  ASICs=SiliconModule->GetASICs();
  //copy ASICs....

}

TSiliconModule::~TSiliconModule()
{
  uint size=ASICs.size();
  TSiliconVA* ASIC;
  for (uint i=0; i<size; i++)
  {
      ASIC=ASICs.at(i);
      if (ASIC) delete ASIC;
  }
  ASICs.clear();
}

TSiliconVA* TSiliconModule::GetASIC( const Int_t number )
{
  TSiliconVA* ASIC = NULL;

  Int_t ASICEntries=ASICs.size();
  
  if( number > ASICEntries )
      return NULL;

  for( Int_t i = 0; i < ASICEntries; i++ )
    {
      ASIC = (TSiliconVA*) ASICs.at(i);
      if(!ASIC)
        continue;
      if( ASIC->GetASICNumber() == number )
        return ASIC;
    }
  
  return NULL;
}

void TSiliconModule::Print()
{
  for( uint i=0; i<ASICs.size(); i++ )
    {
      TSiliconVA* VA = (TSiliconVA*) ASICs.at(i);
      if(VA)
        {
          VA->Print();
        }
      VA=NULL;
    }
}

void TSiliconModule::PrintToFile( FILE * f )
{
  for( uint i=0; i<ASICs.size(); i++ )
    {
      TSiliconVA* VA = (TSiliconVA*) ASICs.at(i);
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

  for( uint i=0; i<ASICs.size(); i++ )
    {
      VA = (TSiliconVA*) ASICs.at(i);
      if( VA )
        {
          if( VA->IsAHitOR() ) printf("SiMOD %d VA %d OR TRUE \n", ModuleNumber, i );  
        }
      VA = NULL;
    }
}

void TSiliconModule::ClearVAs()
{
  Int_t NumberOfASICs = ASICs.size();

  for( Int_t i=0; i<NumberOfASICs; i++ )
    {
      TSiliconVA* VA = (TSiliconVA*) ASICs.at(i);
      if( VA )
        {
          delete VA;
          VA = NULL;
        }
    }
  ASICs.clear();
}

Int_t TSiliconModule::CompressVAs()
{

  TSiliconVA* VA;
  Int_t NumberOfASICs = ASICs.size();

  for( Int_t i=0; i<NumberOfASICs; i++ )
    {
      VA = (TSiliconVA*) ASICs.at(i);

      if( !VA ) continue;
      
      if(VA->IsAHitOR() )
        {           
          VA->CompressStrips();         
          HitModule = true;
        }
      else 
        {
          delete ASICs.at(i);
          ASICs.at(i)=NULL;
        }

    }//loop over ASICs

  //ASICs->Compress();

  return 0;
}

Int_t TSiliconModule::CalcNRawHits()
{
  Int_t NRawHits(0);
  
  TSiliconVA* VA;
  Int_t NumberOfASICs = ASICs.size();

  for( Int_t i=0; i<NumberOfASICs; i++ )
    {
      VA = (TSiliconVA*) ASICs.at(i);
      if( VA ) NRawHits+=VA->CalcNRawHits();

    }//loop over ASICs
  return NRawHits;
}
