/*
 *  TSISEvent.cpp
 *  
 *
 *  Created by Sarah Seif El Nasr on 09/08/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "TSISEvent.h"

ClassImp(TSISEvent);

//Default Constructor
TSISEvent::TSISEvent()
{
  ClearSISEvent();
}
//Default Destructor
TSISEvent::~TSISEvent()
{
}

//Functions required for manipulating data in the tree
TSISEvent::TSISEvent(  ULong64_t clock, double time)
{
  ClearSISEvent();
  SetClock(clock);
  SetRunTime(time);
}

void TSISEvent::ClearSISEvent()
{
    for (int j=0; j<NUM_SIS_CHANNELS; j++)
       Counts[j]=0;
    SetClock(0);
    SetRunTime(-1.);
    SetRunNumber(-1);
    SetMidasUnixTime(-1.);
}

void TSISEvent::Print()
{
  printf("RunTime %f \n",RunTime);
     for (int j=0; j<NUM_SIS_CHANNELS*NUM_SIS_MODULES; j++)
     {
       if (Counts[j]) printf("Channel %d \t CountInChannel %d \t \n", j, Counts[j] ); 
     }

}
