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
TSisEvent::TSisEvent( int mod, ULong64_t clock, double time)
{
  SisModule = mod;
  ClearSisEvent();
  SetClock(clock);
  SetRunTime(time);
}

void TSisEvent::ClearSisEvent()
{
    for (int j=0; j<NUM_SIS_CHANNELS; j++)
       Counts[j]=0;
    SetClock(0);
    SetRunTime(-1.);
    SetRunNumber(-1);
    SetExptTime(-1.);
}

void TSisEvent::Print()
{
  printf("RunTime %f \n",RunTime);
     printf("Sis: %d\n",SisModule);
     for (int j=0; j<NUM_SIS_CHANNELS; j++)
     {
       if (Counts[j]) printf("Channel %d \t CountInChannel %d \t \n", j, Counts[j] ); 
     }

}
