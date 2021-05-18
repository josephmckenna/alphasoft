#ifndef _TSISEvent_
#define _TSISEvent_
/*
 *  TSISEvent.h for manalyzer
 *  JTK McKENNA
 *
 */ 

#include "TObject.h"
#include <iostream>
#include "assert.h"
#define NUM_SIS_MODULES 2
#define NUM_SIS_CHANNELS 32
// Define object of type SISTreeEvent , inherits from TObject
class TSISEvent : public TObject
{
private:
  
  int Counts[NUM_SIS_CHANNELS];
  int SISModule;
     //counts in this channel
  ULong64_t     Clock;               //10 MHz clks
  ULong64_t     VF48Clock;           //20 MHz clock from VF48?
  Double_t      RunTime;             //SIS time since the start of the MIDAS run 
  Int_t         RunNumber;           //MIDAS runnumber
  uint32_t      MidasTime;           //MIDAS Unix Timestamp
  unsigned long MidasEventID;
  //Maybe I could have a vector of strings to label some SIS triggers
  //std::vector<DumpMarker*> DumpEvents; //I do not own the DumpMarker pointer...

public:

  // setters  
  void SetCountsInChannel(int channel, int counts)  { Counts[channel] = counts; }
  void SetSISModuleNo(int module)                   { SISModule = module;  }
  void SetScalerModuleNo(int module)                { SISModule = module;  }
  void SetClock(ULong64_t clock)                    { Clock = clock; }
  void SetVF48Clock(ULong64_t clock)                { VF48Clock = clock; }
  void SetRunTime(Double_t time)                    { RunTime = time; }

  void SetRunNumber(Int_t runnumber)                { RunNumber = runnumber; }
  void SetMidasUnixTime(uint32_t midtime)           { MidasTime = midtime; }
  void SetMidasEventID(unsigned long id)            { MidasEventID = id; }

  void ClearSISEvent();
  
  // getters
  Int_t GetCountsInChannel( int i)
  {
     if ( SISModule==1)
        i=i-32;
     if (i<0) return 0;
     else  if (i>=NUM_SIS_CHANNELS) return 0;
     else 
     //assert(i>=0);
     //assert(i<NUM_SIS_CHANNELS);
     return Counts[i];
   }
  int    GetSISModule()          { return SISModule; }
  int    GetScalerModule()          { return SISModule; }
  ULong64_t GetClock()				{ return Clock; }
  ULong64_t GetVF48Clock()          { return VF48Clock; }
  Double_t  GetRunTime()		    { return RunTime; }

  Int_t     GetRunNumber()     	    { return RunNumber; }
  Double_t  GetMidasUnixTime()		    { return MidasTime; }
  unsigned long GetMidasEventID()            { return MidasEventID; }

  using TObject::Print;
  virtual void Print();
  
  // default class member functions
  TSISEvent( );
  TSISEvent( ULong64_t clock, Double_t time);
  TSISEvent* operator+=(TSISEvent* b);

  virtual ~TSISEvent(); 
  
  ClassDef(TSISEvent,1); 
};
#endif
