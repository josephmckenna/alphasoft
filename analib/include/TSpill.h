#ifndef _TSpill_
#define _TSpill_
#include "TObject.h"
#include <iostream>
#include <bitset>
#include "TString.h"
#include "sqlite3.h"
#include "TDumpList.h"
#include "Sequencer_Channels.h"

class TSpill: public TObject
{
public:
   int                   RunNumber;
   bool                  IsDumpType;
   bool                  IsInfoType;
   uint32_t              Unixtime;
   std::string           Name;

   TSpill();
   TSpill(int runno, uint32_t unixtime);
   ~TSpill();
   //TSpill(const char* name);
   void InitByName(const char* format, va_list args);
   TSpill(int runno, uint32_t unixtime, const char* format, ...);
   TSpill(const TSpill& a);
   TSpill& operator =(const TSpill& rhs);
private:
   //Recursive function to compare strings with wildcard support (called by IsMatchForDumpName)
   bool MatchWithWildCards(const char *first, const char * second);
public:
   bool IsMatchForDumpName(const std::string& dumpname);
   virtual double GetStartTime() const = 0;
   virtual double GetStopTime() const = 0;
   bool DumpHasMathSymbol() const;
   using TObject::Print;
   virtual void Print();


   virtual int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(std::vector<int>*, int& );
   std::string ContentCSVTitle() const;
   std::string ContentCSV() const;
   ClassDef(TSpill,1);
};

#endif
