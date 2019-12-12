#ifndef _TA2Spill_
#define _TA2Spill_
#include "TObject.h"
#include <iostream>
#include <bitset>
#include "TString.h"
#include "sqlite3.h"
#define MAXDET 8
#define N_COLUMNS MAXDET+2
/*
//RunNumber	EventID	data (string)	Unix Time
class LabviewString: public TObject
{
	
};

//RunNumber	Equipment ID	Spill ID	ADC count (double)	Unix time	Run time

class LabviewDouble: public TObject
{
   int Type; 
   int EventID; //Row number
   double Counts;
};*/
//RunNumber (int)	SeqNum (int)	Dump ID (int)	Name (str)	Unix Time	Start Time (seconds) (double)	Stop Time (seconds) (double)	Detector Counts[64] (int64)
class A2ScalerData: public TObject
{
   public:
   double         StartTime;
   double         StopTime;
   int            DetectorCounts[64];
   int            VF48Events;
   int            Verticies;
   int            PassCuts;
   int            PassMVA;
   unsigned long  SISFilled; //bit mask for each channel (64)
   bool           SVDFilled;
   A2ScalerData();
   A2ScalerData(A2ScalerData* a);
   A2ScalerData* operator/(const A2ScalerData* b);
   bool Ready( bool have_svd);
   using TObject::Print;
   virtual void Print();
   ~A2ScalerData();
   ClassDef(A2ScalerData,1);
};

class A2SeqData: public TObject
{
   public:
   int SequenceNum; //Sequence number 
   int DumpID; //Row number 
   std::string SeqName;
   A2SeqData();
   A2SeqData(A2SeqData* a);
   A2SeqData* operator/(const A2SeqData* b);
   using TObject::Print;
   virtual void Print();
   ~A2SeqData();
   ClassDef(A2SeqData,1);
};


class A2Spill: public TObject
{
   public:
   int           RunNumber;
   bool          IsDumpType;
   bool          IsInfoType;
   int           Unixtime;
   std::string   Name;
   A2SeqData*    SeqData;
   A2ScalerData* ScalerData;

   A2Spill();
   A2Spill(const char* name, int unixtime=0);
   A2Spill(A2Spill* a);
   A2Spill* operator/(const A2Spill* b);
   bool Ready( bool have_svd);

   using TObject::Print;
   virtual void Print();
   int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(std::vector<int>*, int& );
   //TString FormatDump();
   //TString Header(int TotalSeq);
   ~A2Spill(){};
   ClassDef(A2Spill,1);
};

#endif
