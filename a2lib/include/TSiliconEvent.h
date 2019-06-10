#ifndef __TSiliconEvent__
#define __TSiliconEvent__

// TSiliconEvent Class =====================================================================================
//
// Class representing a single event (analogue readout) of the Silicon detector. 
//
// JWS 10/10/2008
//
// ==========================================================================================================


#include "TNamed.h"
#include "TVector3.h"
#include "TSiliconModule.h"



class TSiliconEvent : public TNamed 
{
private:

  TVector3  Vertex;               // Vertex
  TVector3  ProjVertex;           // ProjVertex
  TVector3  CosmicVector;         // Vector of smallest Chi2
  Int_t     VertexType;           // Type of vertex
  Int_t	    NTracks;	          // Number of tracks
  Int_t     NVertices;            // Number of vertices
  Int_t     NHits;                // Number of clusters
  Int_t     NsideNRawHits;        // Number of above threshold hits
  Int_t     PsideNRawHits;        // Number of above threshold hits
  bool      PassedCuts;

  Int_t     VF48NEvent;           // VF48 event number
  Int_t     VF48NTrigger;         // VF48 trigger number
  Double_t  VF48Timestamp;        // VF48 timestamp

  Int_t     RunNumber;            // MIDAS runnumber

  Double_t  RunTime;	          // Time of event from start of MIDAS run ( time = time of corresponding ADC trigger into SIS )
  Double_t  ExptTime;             // Time of event from start of experiment ( time = time of corresponding ADC trigger into SIS ) 
  Double_t  TSRunTime;            // Time of event from start of MIDAS run ( time = timestamp + vf48offset (SIS time of 1st VF48 trigger) ) 
  Int_t     ExptNumber;           // Sequencer experiment number (aka. chain number) 

  Double_t DCA;
  Double_t Residual;

  Int_t     SisCounter;           // SIS counter
  Int_t     LabVIEWCounter;       // LabVIEWCounter
 
  std::vector<TSiliconModule*> SiliconModules;      // Silicon modules 
  Int_t TTCEventCounter[4];       //TTC event tree counter (per FPGA)


public:
  TSiliconEvent();
  TSiliconEvent( TSiliconEvent*& );
  virtual ~TSiliconEvent();

  // setters

  void SetProjVertex(TVector3* pvtx) {ProjVertex.SetXYZ(pvtx->X(),pvtx->Y(),pvtx->Z());}
  void SetVertex(TVector3* vtx){ Vertex.SetXYZ(vtx->X(), vtx->Y(), vtx->Z()); }
  void SetCosmicVector(TVector3* cv){CosmicVector.SetXYZ(cv->X(), cv->Y(), cv->Z());}
  void SetVertexType(Int_t type) { VertexType = type; }
  void SetNTracks( Int_t ntracks ){ NTracks = ntracks; }
  void SetNVertices( Int_t nvertices ){ NVertices = nvertices; }
  void SetNHits( Int_t nhits ){ NHits = nhits; }
  void SetNsideNRawHits( Int_t nrawhits ){ NsideNRawHits = nrawhits; }
  void SetPsideNRawHits( Int_t nrawhits ){ PsideNRawHits = nrawhits; }

  void SetVF48NEvent( Int_t nvf48event ){ VF48NEvent = nvf48event; }
  void SetVF48NTrigger( Int_t nVF48trigger ){ VF48NTrigger = nVF48trigger; }
  void SetVF48Timestamp( Double_t timestamp ){ VF48Timestamp = timestamp; }

  void SetRunNumber( Int_t number ){ RunNumber = number; }

  void SetRunTime( Double_t time ){ RunTime = time; }
  void SetExptTime( Double_t time ){ ExptTime = time; }
  void SetTSRunTime( Double_t time ){ TSRunTime = time; }

  void SetExptNumber( Int_t number ){ ExptNumber = number; }

  void SetDCA(Double_t dca ) { DCA = dca; }
  void SetResidual( Double_t res ) { Residual = res; }

  void SetSISCounter( Int_t event ){ SisCounter = event; }
  void SetLabVIEWCounter( Int_t event )	{ LabVIEWCounter = event; }
  void SetCounters( Int_t SisCounter, Int_t LabVIEWCounter) { SetSISCounter( SisCounter ); SetLabVIEWCounter( LabVIEWCounter); }
 
  void AddSiliconModule( TSiliconModule* SiliconModule ){ SiliconModules.push_back(SiliconModule); } 
  
  void SetTTCCounter( Int_t TTCtreeAddress, Int_t FPGA) { TTCEventCounter[FPGA]=TTCtreeAddress; }
  
  void ApplyCuts();

  // getters
 
  inline TVector3* GetProjVertex()   { return &ProjVertex; }
  inline TVector3* GetVertex()       { return &Vertex; }
  inline TVector3* GetCosmicVector() { return &CosmicVector; }
  
  Double_t GetVertexX() { return Vertex.X(); }
  Double_t GetVertexY() { return Vertex.Y(); }
  Double_t GetVertexZ() { return Vertex.Z(); }
  Double_t GetVertexR() { return Vertex.Perp(); }
  Double_t GetVertexPhi() { return Vertex.Phi()/3.14159*180.; }
 
  
  Int_t GetVertexType()              { return VertexType; }
  Int_t GetNTracks()		     { return NTracks; }
  Int_t GetNVertices()               { return NVertices; }
  Int_t GetNHits()                   { return NHits; }
  Int_t GetNRawHits()                { return (NsideNRawHits+PsideNRawHits); }
  Int_t GetNsideNRawHits()                { return NsideNRawHits; }
  Int_t GetPsideNRawHits()                { return PsideNRawHits; }
  bool  GetPassedCuts()                   { return PassedCuts; }

  Int_t GetVF48NEvent()	             { return VF48NEvent; }
  Int_t GetVF48NTrigger()            { return VF48NTrigger; }
  Double_t GetVF48Timestamp()        { return VF48Timestamp; }

  Int_t GetRunNumber()               { return RunNumber; }

  Double_t GetRunTime()		     { return RunTime; }
  Double_t GetExptTime()	     { return ExptTime; }
  Double_t GetTSRunTime()            { return TSRunTime; }

  Int_t GetExptNumber()              { return ExptNumber; }

  Double_t GetDCA()                  { return DCA; }
  Double_t GetResidual()             { return Residual; }

  Int_t GetSISCounter()              { return SisCounter; }
  Int_t GetLabVIEWCounter()	     { return LabVIEWCounter; }

  TSiliconModule* GetSiliconModule( Int_t ModuleNumber );
  std::vector<TSiliconModule*> GetSiliconModuleArray() { return SiliconModules; }

  Int_t CompressSiliconModules();
  
  TString PrintCSVData(Double_t RelativeTime=0.);
  TString PrintCSVTitle();
  void ClearEvent();
  using TNamed::Print;
  virtual void Print();
  void PrintToFile( FILE * f );

  // calculators
  //Int_t CalcNRawHits(); Redired function

  Int_t GetTTCCounter( Int_t FPGA) { return TTCEventCounter[FPGA]; }

  
  ClassDef(TSiliconEvent,2);
};

#endif
