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

struct SIS_Counts
{
   double t;
   int counts;
};
struct SVD_Counts
{
   double t;
   int    VF48EventNo;
   bool   has_vertex;
   bool   passed_cuts;
   bool   online_mva;
   //SVD_Counts(){}
   //SVD_Counts(const SVD_Counts& c): t(c.t), VF48EventNo(c.VF48EventNo),has_vertex(c.has_vertex), passed_cuts(c.passed_cuts), online_mva(c.online_mva) {}
};

class A2ScalerData: public TObject
{
   public:
   double         StartTime;
   double         StopTime;

   int            DetectorCounts[64];
   unsigned long  SISFilled; //bit mask for each channel (64)

   int            FirstVF48Event;
   int            LastVF48Event;
   int            VF48Events;
   int            Verticies;
   int            PassCuts;
   int            PassMVA;
   bool           SVDFilled;

   A2ScalerData();
   A2ScalerData(A2ScalerData* a);
   A2ScalerData* operator/(const A2ScalerData* b);
   void AddData(const SVD_Counts& c);
   void AddData(const SIS_Counts& c,  const int &channel);
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
