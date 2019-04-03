#include "TSiliconEvent.h" 
#include "TVector.h"
#include "TVector3.h"


// TSiliconEvent Class =====================================================================================
//
// Class representing a single event (analogue readout) of the Silicon detector. 
//
// JWS 10/10/2008
//
// ==========================================================================================================

ClassImp(TSiliconEvent);


TSiliconEvent::TSiliconEvent()
{
  Vertex.SetXYZ(0,0,0);
  ProjVertex.SetXYZ(0,0,0);
  CosmicVector.SetXYZ(0,0,0);
  VertexType = 0; 
  NTracks = 0;
  NVertices = 0;
  NHits = 0;
  NsideNRawHits = 0;
  PsideNRawHits = 0;
  VF48NEvent = -1;
  VF48NTrigger = -1;
  VF48Timestamp = -1.0;
  RunTime = -1.;
  ExptTime = -1.;
  TSRunTime = -1.;
  ExptNumber = -1;
  RunNumber = -1;
  SisCounter = 0;
  LabVIEWCounter = 0;
  DCA = -1.;
  Residual = -1.;
  for (Int_t i=0; i<4; i++)
  {
    TTCEventCounter[i]=-1;
  }
  SiliconModules = new TObjArray();
 
}

TSiliconEvent::~TSiliconEvent()
{
  ClearEvent();
  //if ( ASICs ) delete ASICs;
  //if( SiliconModules ) delete SiliconModules;
  
  if( SiliconModules ) 
  {
    if (SiliconModules->GetEntriesFast())
    {
      SiliconModules->SetOwner( kTRUE );
      SiliconModules->Delete();
    }
    delete SiliconModules;
  }
}

void TSiliconEvent::ClearEvent()
{
  Vertex.SetXYZ(0,0,0);
  ProjVertex.SetXYZ(0,0,0);
  CosmicVector.SetXYZ(0,0,0);
  VertexType = 0;
  NTracks = 0;
  NVertices = 0;
  NHits = 0;
  NsideNRawHits = 0;
  PsideNRawHits = 0;
  VF48NEvent = -1;
  VF48NTrigger = -1;
  VF48Timestamp = -1.0;
  RunTime = -1.;
  ExptTime = -1.;
  TSRunTime = -1.;
  ExptNumber = -1;
  RunNumber = -1;
  SisCounter = 0;
  LabVIEWCounter = 0;
  DCA = -1.;
  Residual = -1.;
  for (Int_t i=0; i<4; i++)
  {
    TTCEventCounter[i]=-1;
  }
  //if( SiliconModules != NULL )
  //   {
  //     for( Int_t i=0; i < SiliconModules->GetEntries(); i++ )
  //       {
   //        TSiliconModule* SiliconMod = (TSiliconModule*) SiliconModules->At(i);
  //         if( SiliconMod ) delete SiliconMod;
  //         SiliconMod = NULL;
   //      }

  //     delete SiliconModules;
   //  }
  
  if( SiliconModules ) 
  {
    if (SiliconModules->GetEntries())
    {
      SiliconModules->SetOwner(kTRUE);
      SiliconModules->Delete();
    }
    delete SiliconModules;
  }
  SiliconModules = new TObjArray();
}

TSiliconEvent::TSiliconEvent( TSiliconEvent*& event )
{
  Vertex          = *(event->GetVertex()); 
  ProjVertex      = *(event->GetProjVertex());
  CosmicVector    = *(event->GetCosmicVector());
  VertexType      = event->GetVertexType();
  NTracks         = event->GetNTracks();
  NVertices       = event->GetNVertices();
  NHits           = event->GetNHits();
  VF48NEvent      = event->GetVF48NEvent();
  VF48NTrigger    = event->GetVF48NTrigger();
  VF48Timestamp   = event->GetVF48Timestamp();
  RunNumber       = event->GetRunNumber();
  RunTime         = event->GetRunTime();
  ExptTime        = event->GetExptTime();
  TSRunTime       = event->GetTSRunTime();
  ExptNumber      = event->GetExptNumber();
  SisCounter      = event->GetSISCounter();
  LabVIEWCounter  = event->GetLabVIEWCounter();
  DCA             = event->GetDCA();
  SiliconModules  = event->GetSiliconModuleArray();
}

TSiliconModule* TSiliconEvent::GetSiliconModule( Int_t ModuleNumber )
{
  TSiliconModule* SiliconModule = NULL;

  //loop over SiliconModules
  for( Int_t i=0; i<SiliconModules->GetEntriesFast(); i++ )
    {
      SiliconModule = (TSiliconModule*) SiliconModules->At(i);
      if( SiliconModule->GetModuleNumber() == ModuleNumber )
        {
          return SiliconModule;
        }
    }

  return NULL;
}

Int_t TSiliconEvent::CompressSiliconModules()
{
  Int_t SiliconModuleEntries = SiliconModules->GetEntries();

  for( Int_t i=0; i<SiliconModuleEntries; i++ )
    {
      TSiliconModule* Module = (TSiliconModule*) SiliconModules->At(i);
      if( !Module ) continue;

      Module->CompressVAs();  
      if( !Module->IsAHitModule() )
        {
          
          //delete Module;
          SiliconModules->RemoveAt(i);
          Module->Delete();
        }
    } 

  SiliconModules->Compress();
    
  return 0;
}

void TSiliconEvent::Print()
{
  printf( "VF48NEvent = %d \t VF48NTrigger = %d \t VF48Timestamp = %f \t RunTime = %f \t TSRunTime = %f \t ExptTime = %f \t ExptNumber = %d \t NVertices = %d \n",
          VF48NEvent, VF48NTrigger, VF48Timestamp, RunTime, TSRunTime, ExptTime, ExptNumber, NVertices );

  for( Int_t i=0; i<SiliconModules->GetEntries(); i++ )
    {
      TSiliconModule* Module = (TSiliconModule*) SiliconModules->At(i);
      if( !Module ) continue;

      Module->Print();
    } 

} 

TString TSiliconEvent::PrintCSVData(Double_t RelativeTime)
{
//VF48 Number,VF48 time (s),RunTime (official),TSRunTime,ExptTime,RunTime - TSRunTime,Time from given start (ms),N Tracks,NVertices,X,Y,Z,R,Phi,Residual,";
  TString Data="";
  Data+=GetVF48NEvent();
  Data+=",";
  Data+=GetVF48Timestamp();
  Data+=",";
  Data+=GetRunTime();
  Data+=",";
  Data+=GetTSRunTime();
  Data+=",";
  Data+=GetExptTime();
  Data+=",";
  Data+=GetRunTime() - GetTSRunTime();
  Data+=",";
  Data+=(GetRunTime()-RelativeTime)*1000.;
  Data+=",";
  Data+=GetNTracks();
  Data+=",";
  Data+=GetNVertices();
  Data+=",";
  Data+=GetVertexX();
  Data+=",";
  Data+=GetVertexY();
  Data+=",";
  Data+=GetVertexZ();
  Data+=",";
  Data+=GetVertexR();
  Data+=",";
  Data+=GetVertexPhi();
  Data+=",";
  Data+=GetResidual();
  Data+=",";

  std::cout << GetVF48NEvent()<<"\t"<<
  GetVF48Timestamp()<<"\t"<<
  GetRunTime()<<"\t"<<
  GetTSRunTime()<<"\t"<<
  GetRunTime() - GetTSRunTime()<<"\t"<<
  (GetRunTime()-RelativeTime)*1000.<<"\t"<<
  GetNTracks()<<"\t"<<
  GetVertexX()<<"\t"<<
  GetVertexY()<<"\t"<<
  GetVertexZ()<<"\t"<<
  GetVertexR()<<"\t"<<
  GetVertexPhi()<<"\t"<<
  GetResidual()<<"\t";
  
  return Data;
}
TString TSiliconEvent::PrintCSVTitle()
{
  TString title="VF48 Number,VF48 time (s),RunTime (official),TSRunTime,ExptTime,RunTime - TSRunTime,Time from given start (ms),N Tracks,NVertices,X,Y,Z,R,Phi,Residual,";
  std::cout <<"VF48 Number	Unix time	 VF48 time (s)	 RunTime (official)	TSRunTime	ExptTime	 RunTime - TSRunTime	 Time from given start (ms)	N Tracks	NVertices	X	Y	Z	R	Phi	Residual"<<std::endl;

  return title;
}

void TSiliconEvent::PrintToFile( FILE * f )
{
  for( Int_t i=0; i<SiliconModules->GetEntries(); i++ )
    {
      TSiliconModule* Module = (TSiliconModule*) SiliconModules->At(i);
      if( !Module ) continue;

      Module->PrintToFile( f );
    }   
  fprintf( f, "\n" );
}

/*
Int_t TSiliconEvent::CalcNRawHits()
{
  Int_t NRawHits(0);
  Int_t SiliconModuleEntries = SiliconModules->GetEntries();

  for( Int_t i=0; i<SiliconModuleEntries; i++ )
    {
      TSiliconModule* Module = (TSiliconModule*) SiliconModules->At(i);
      if( Module ) NRawHits += Module->CalcNRawHits();
    } 

  return NRawHits;
}*/
