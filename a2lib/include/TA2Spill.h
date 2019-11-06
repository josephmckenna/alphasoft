#ifndef _TA2Spill_
#define _TA2Spill_
#include "TObject.h"
#include <iostream>
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

class A2Spill: public TObject
{
   public:
   int RunNumber;
   int SequenceNum; //Sequence number 
   int DumpID; //Row number 
   std::string Name;
   bool IsDumpType;
   int Unixtime;
   double StartTime;
   double StopTime;
   int DetectorCounts[64];
   int VF48Events;
   int Verticies;
   int PassCuts;
   int PassMVA;
   //std::string DetectorNames[N_COUMNS];

   A2Spill();
   A2Spill(A2Spill* a);
   std::string SeqName;
   bool SISFilled;
   bool SVDFilled;
   
   bool Ready(double T, double data_buffer_time=2.);
   using TObject::Print;
   virtual void Print();
   int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content();
   //TString FormatDump();
   //TString Header(int TotalSeq);
   ~A2Spill(){};
   ClassDef(A2Spill,1);
};

#endif
