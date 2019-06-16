#ifndef _TA2Spill_
#define _TA2Spill_
#include "TObject.h"
#include <iostream>
#define MAXDET 8
#define N_COLUMNS MAXDET+2


class A2Spill: public TObject
{
   public:
   int SequenceNum;
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
   ~A2Spill(){};
   ClassDef(A2Spill,1);
};

#endif
