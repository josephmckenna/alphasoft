#include "TSpillSequencerData.h"

ClassImp(TSpillSequencerData)


TSpillSequencerData::TSpillSequencerData()
{
   fSequenceNum=-1;
   fDumpID     =-1;
   fSeqName    ="";
   fStartState =-1;
   fStopState  =-1;
}
TSpillSequencerData::TSpillSequencerData(const TSpillSequencerData& a) : TObject(a)
{
   fSequenceNum  =a.fSequenceNum;
   fDumpID       =a.fDumpID;
   fSeqName      =a.fSeqName;
   fStartState   =a.fStartState;
   fStopState    =a.fStopState;
}


TSpillSequencerData::~TSpillSequencerData()
{
}

void TSpillSequencerData::Print()
{
   std::cout<<"SeqName:"       <<fSeqName
            <<"\tSeq:"         <<fSequenceNum
            <<"\tDumpID:"      <<fDumpID
            <<"\tstartState:"  <<fStartState
            <<"\tstopState:"   <<fStopState
            <<std::endl;
}


std::string TSpillSequencerData::ContentCSVTitle() const
{
   std::string title = "Sequencer ID,Sequencer Name, Dump ID, Start State, Stop State,";
   return title;
}

std::string TSpillSequencerData::ContentCSV() const
{
   std::string line = std::to_string(fSequenceNum) + "," +
                      fSeqName + "," +
                      std::to_string(fDumpID) + "," +
                      std::to_string(fStartState) + "," +
                      std::to_string(fStopState) + ",";
   return line;
}
