#ifndef _TSisModule_
#define _TSisModule_
/*
 *  TSisModule.h for manalyzer
 *  JTK McKENNA
 *
 */ 

#include "TSisEvent.h"
#include <iostream>

//Do not inherit from TObject... Dont bother ever saving this...
class SisModule 
{
private:
  
  int Counts[NUM_SIS_CHANNELS];
  int Module;
     //counts in this channel
  ULong64_t     Clock;               //10 MHz clks
  Double_t      RunTime;             //SIS time since the start of the MIDAS run 

  
public:
  

  // setters  
  void SetCountsInChannel(int channel, int counts) { Counts[channel] = counts; }
  void SetClock(ULong64_t clock)			 { Clock = clock; }
  void SetRunTime(Double_t time)			 { RunTime = time; }

  void ClearSisModule();
  
  // getters
  Int_t     GetCountsInChannel( int i) 	{ return Counts[i]; }
  ULong64_t GetClock()				{ return Clock; }
  Double_t  GetRunTime()		    { return RunTime; }

 void Print();
  
  SisModule( int mod, ULong64_t clock, Double_t time);
  ~SisModule(); 
  
};
#endif
