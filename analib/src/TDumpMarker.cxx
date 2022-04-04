
#include "TDumpMarker.h"
#include <iostream>


TDumpMarker::TDumpMarker()
{
   fDescription   = "NULL";
   fSequencerID   = -1;
   fSequenceCount = -1;
   fDumpType      = -1;
   fonCount       = -1;
   fonState       = -1;
   fRunTime       = -1.;
   fMidasTime     = 0;
}

TDumpMarker::TDumpMarker(
   const char* _Description,
   Int_t _SequencerNum,
   Int_t _SeqCount,
   TDumpMarker::kDumpTypes _DumpType,
   Int_t _onCount,
   Int_t _onState,
   double runTime,
   uint32_t _MidasTime) 
{
   fDescription   = _Description;
   fSequencerID   = _SequencerNum;
   fSequenceCount = _SeqCount;
   fDumpType      = _DumpType;
   fonCount       = _onCount;
   fonState       = _onState;
   fRunTime       = runTime;
   fMidasTime     = _MidasTime;
}

TDumpMarker::TDumpMarker(const char* name,kDumpTypes type): TDumpMarker()
{
   fDescription=name;
   fDumpType=type;
}

TDumpMarker::TDumpMarker(const TDumpMarker& d)
{
   fDescription   = d.fDescription;
   fSequencerID   = d.fSequencerID;
   fSequenceCount = d.fSequenceCount;
   fDumpType      = d.fDumpType; //0= Start, 1=Stop, 2=AD spill?, 3=Positrons etc
   fonCount       = d.fonCount;
   fonState       = d.fonState;
   fRunTime       = d.fRunTime;
   fMidasTime     = d.fMidasTime;
}

void TDumpMarker::Print() const
{
    std::cout<<"SequencerID:"<<fSequencerID;
    std::cout <<"\tName:"<< GetSequencerName(fSequencerID);
    std::cout<<"\tSequenceCount:"<<fSequenceCount
            <<"\tDescription:"<<fDescription.c_str()
            <<"\tType:"<<fDumpType
            <<"\tonCount:"<<fonCount
            <<"\tonState:"<<fonState
            <<"\tRunTime:"<<fRunTime
            <<"\tSequenceStartTime:"<<fMidasTime<<std::endl;
}
