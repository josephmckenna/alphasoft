#include "TA2SpillSequencerData.h"
ClassImp(TA2SpillSequencerData)

TA2SpillSequencerData::TA2SpillSequencerData():
   TSpillSequencerData()
{
}
TA2SpillSequencerData::~TA2SpillSequencerData()
{
}
TA2SpillSequencerData::TA2SpillSequencerData(TDumpMarkerPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d)
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


TA2SpillSequencerData::TA2SpillSequencerData(const TA2SpillSequencerData& a):
   TSpillSequencerData(a)
{
   fSequenceNum  =a.fSequenceNum;
   fDumpID       =a.fDumpID;
   fSeqName      =a.fSeqName;
   fStartState   =a.fStartState;
   fStopState    =a.fStopState;
}