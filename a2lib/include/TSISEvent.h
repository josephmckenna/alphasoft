#ifndef _TSISEvent_
#define _TSISEvent_
/*
 *  TSISEvent.h for manalyzer
 *  JTK McKENNA
 *
 */ 

#include "TObject.h"
#include <iostream>

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
  Double_t      ExptTime;            //SIS time since the start of the experiment 
  
public:
  

  // setters  
  void SetCountsInChannel(int channel, int counts) { Counts[channel] = counts; }
  void SetSISModuleNo(int module)                    { SISModule = module;  }
  void SetClock(ULong64_t clock)			 { Clock = clock; }
  void SetVF48Clock(ULong64_t clock)			 { VF48Clock = clock; }
  void SetRunTime(Double_t time)			 { RunTime = time; }

  void SetRunNumber(Int_t runnumber)		 { RunNumber = runnumber; }
  void SetExptTime(Double_t expttime)		 { ExptTime = expttime; }

  void ClearSISEvent();
  
  // getters
  Int_t GetCountsInChannel( int i)
  {
     if ( SISModule==1)
        i=i-32;
     if (i<0) return 0;
     else  if (i>=NUM_SIS_CHANNELS) return 0;
     else return Counts[i];
   }
  
  ULong64_t GetClock()				{ return Clock; }
  ULong64_t GetVF48Clock()          { return VF48Clock; }
  Double_t  GetRunTime()		    { return RunTime; }

  Int_t     GetRunNumber()     	    { return RunNumber; }
  Double_t  GetExptTime()		    { return ExptTime; }

  using TObject::Print;
  virtual void Print();
  
  // default class member functions
  TSISEvent( );
  TSISEvent( ULong64_t clock, Double_t time);
  virtual ~TSISEvent(); 
  
  ClassDef(TSISEvent,1); 
};
#endif
