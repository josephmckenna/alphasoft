#include "TInfoSpill.h"

ClassImp(TInfoSpill)
TInfoSpill::TInfoSpill(): TSpill()
{
   IsDumpType =false; //By default, this is never a dump
   IsInfoType =true;
   Unixtime   =0;
}

TInfoSpill::TInfoSpill(int runno, uint32_t runstarttime, uint32_t unixtime): TSpill(runno, unixtime)
{
   fRunStartTime = runstarttime;
}

TInfoSpill::TInfoSpill(int runno, uint32_t runstarttime, uint32_t unixtime, const char* format, ...): TSpill(runno,unixtime)
{
   fRunStartTime = runstarttime;
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}

TInfoSpill::TInfoSpill(const TInfoSpill& a): TSpill(a)
{

}

double TInfoSpill::GetStartTime() const
{
   return Unixtime - fRunStartTime;
}

double TInfoSpill::GetStopTime() const
{
   return Unixtime - fRunStartTime;
}
