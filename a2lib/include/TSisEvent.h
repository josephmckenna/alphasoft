#ifndef _TSisEvent_
#define _TSisEvent_
/*
 *  TSisEvent.h
 *  
 *
 *  Created by Sarah Seif El Nasr on 09/08/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 *  SSN: Header file used to define the new root library
 *       containing the Tobject SisTreeEvent 
 */ 

#include "TObject.h"
#include <iostream>

// Define object of type SisTreeEvent , inherits from TObject
class TSisEvent : public TObject
{
private:
  
  Int_t 		Channel;             //SIS channel number
  Int_t 	    CountsInChannel;     //counts in this channel
  ULong64_t 	Clock;               //10 MHz clks

  Double_t    	RunTime;             //SIS time since the start of the MIDAS run 
  Int_t         RunNumber;           //MIDAS runnumber
  Int_t         ExptNumber;          //Sequencer experiment number (aka. chain number)
  Double_t      ExptTime;            //SIS time since the start of the experiment 

  Int_t         VertexCounter;       //Link to the vertex tree
  Int_t         LabVIEWCounter;      //Link to the labview tree
  
public:
  

  // setters  
  void SetChannel(Int_t channel)			 { Channel = channel; }
  void SetCountsInChannel(Int_t counts)      { CountsInChannel = counts; }
  void SetClock(ULong64_t clock)			 { Clock = clock; }
  void SetRunTime(Double_t time)			 { RunTime = time; }

  void SetRunNumber(Int_t runnumber)		 { RunNumber = runnumber; }
  void SetExptNumber(Int_t exptnumber)    { ExptNumber = exptnumber; }
  void SetExptTime(Double_t expttime)		 { ExptTime = expttime; }
  
  void SetVertexCounter( Int_t event ) 	{ VertexCounter = event; }
  void SetLabVIEWCounter( Int_t event )	{ LabVIEWCounter = event; }
  void SetCounters( Int_t LabVIEWCounter, Int_t VertexCounter) { SetLabVIEWCounter( LabVIEWCounter ); SetVertexCounter( VertexCounter); }
  void SetSisEvent(Int_t counts, Int_t channel, ULong64_t clock, Double_t time);

  void SetAll( Int_t channel, Int_t counts, ULong64_t clock, Double_t time, Int_t runnumber, Int_t exptnumber, Double_t expttime, Int_t LabVIEWCounter, Int_t VertexCounter ); 
  void ClearSisEvent();
  
  // getters
  Int_t     GetChannel()		    { return Channel; }
  Int_t     GetCountsInChannel() 	{ return CountsInChannel; }
  ULong64_t GetClock()				{ return Clock; }
  Double_t  GetRunTime()		    { return RunTime; }

  Int_t     GetRunNumber()     	    { return RunNumber; }
  Int_t     GetExptNumber()         { return ExptNumber; }
  Double_t  GetExptTime()		    { return ExptTime; }

  Int_t GetVertexCounter() 	{ return VertexCounter; }
  Int_t GetLabVIEWCounter()	{ return LabVIEWCounter; }
  
  using TObject::Print;
  virtual void Print();
  
  // default class member functions
  TSisEvent();  
  virtual ~TSisEvent(); 
  
  ClassDef(TSisEvent,1); 
};
#endif
