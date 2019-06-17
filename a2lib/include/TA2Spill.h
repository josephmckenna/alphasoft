#ifndef _TA2Spill_
#define _TA2Spill_
#include "TObject.h"
#include <iostream>
#include "TString.h"
#define MAXDET 8
#define N_COLUMNS MAXDET+2


class A2Spill: public TObject
{
   public:
   int SequenceNum;
   std::string SeqName;
   std::string Name;
   bool IsDumpType;
   double StartTime;
   double StopTime;
   bool SISFilled;
   bool SVDFilled;
   int DetectorCounts[N_COLUMNS];
   //std::string DetectorNames[N_COUMNS];

   A2Spill();
   A2Spill(A2Spill* a);
   bool Ready();
   void Print();
   TString Content();
   //TString FormatDump();
   //TString Header(int TotalSeq);
   ~A2Spill(){};
   ClassDef(A2Spill,1);
};

#endif
