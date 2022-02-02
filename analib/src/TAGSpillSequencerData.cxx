#include "TAGSpillSequencerData.h"

#if BUILD_AG

ClassImp(TAGSpillSequencerData)

TAGSpillSequencerData::TAGSpillSequencerData():
   TSpillSequencerData()
{
}
TAGSpillSequencerData::~TAGSpillSequencerData()
{
}
TAGSpillSequencerData::TAGSpillSequencerData(TDumpMarkerPair<TStoreEvent,TChronoBoardCounter,CHRONO_N_BOARDS>* d)
{
   fSequenceNum= d->fStartDumpMarker->fSequencerID;
   fDumpID     = d->fDumpID;
   if ( fSequenceNum < 0 )
      fSeqName = "Sequencer Unknown";
   else
   fSeqName    = SeqNames.at(fSequenceNum);
   fStartState = d->fStartDumpMarker->fonState;
   fStopState  = d->fStopDumpMarker->fonState;
}


TAGSpillSequencerData::TAGSpillSequencerData(const TAGSpillSequencerData& a):
   TSpillSequencerData(a)
{
   fSequenceNum  =a.fSequenceNum;
   fDumpID       =a.fDumpID;
   fSeqName      =a.fSeqName;
   fStartState   =a.fStartState;
   fStopState    =a.fStopState;
}

#endif