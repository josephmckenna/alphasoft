#ifndef _TSpillSequencerData_
#define _TSpillSequencerData_

#include "TObject.h"
#include <iostream>

class TSpillSequencerData: public TObject
{
   public:
   int          fSequenceNum; //Sequence number 
   int          fDumpID; //Row number 
   std::string  fSeqName;
   int          fStartState;
   int          fStopState;
   TSpillSequencerData();
   ~TSpillSequencerData();
   TSpillSequencerData(const TSpillSequencerData& a);
   TSpillSequencerData& operator =(const TSpillSequencerData& rhs);
//   TSpillSequencerData(TDumpMarkerPair* d);
   TSpillSequencerData* operator/(const TSpillSequencerData* b);
   using TObject::Print;
   virtual void Print();
   std::string ContentCSVTitle() const;
   std::string ContentCSV() const;
   ClassDef(TSpillSequencerData,1);
};

#endif