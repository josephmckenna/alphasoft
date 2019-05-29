/*
 *  TSisEvent.cpp
 *  
 *
 *  Created by Sarah Seif El Nasr on 09/08/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "TSisEvent.h"

ClassImp(TSisEvent);

//Default Constructor
TSisEvent::TSisEvent()
{
  ClearSisEvent();
}
//Default Destructor
TSisEvent::~TSisEvent()
{
}

//Functions required for manipulating data in the tree
void TSisEvent::SetSisEvent(Int_t counts, Int_t channel, ULong64_t clock, Double_t time)
{	
	SetChannel(channel);
	SetCountsInChannel(counts);
	SetClock(clock);
	SetRunTime(time);
}

void TSisEvent::ClearSisEvent()
{
	SetChannel(-1);
	SetCountsInChannel(-1);
	SetClock(0);
	SetRunTime(-1.);
    SetRunNumber(-1);
    SetExptNumber(-1);
    SetExptTime(-1.);
    SetVertexCounter(-1);
    SetLabVIEWCounter(-1);
}

void TSisEvent::SetAll(Int_t channel, Int_t counts, ULong64_t clock, Double_t time, Int_t runnumber, Int_t exptnumber, Double_t expttime, Int_t LabVIEWCounter, Int_t VertexCounter)
{	
	SetChannel(channel);
	SetCountsInChannel(counts);
	SetClock(clock);
	SetRunTime(time);
    SetRunNumber(runnumber);
    SetExptNumber(exptnumber);
    SetExptTime(expttime);
    SetVertexCounter(LabVIEWCounter);
    SetLabVIEWCounter(VertexCounter);
}

void TSisEvent::Print()
{
  printf("Channel %d \t CountInChannel %d \t RunTime %f \n", Channel, CountsInChannel, RunTime ); 
}
