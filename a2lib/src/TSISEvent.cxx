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
   //I do not own the pointers inside this vector... do not delete them
   //DumpEvents.clear();
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
   SetMidasUnixTime(0);
}
TSISEvent* TSISEvent::operator+=( TSISEvent* b)
{
   //Events from differnt SIS modules cannot be added!
   //this->Print();
   //std::cout <<"Adding module "<<b->GetSISModule() << " to "<<this->GetSISModule()<<std::endl;
   assert(this->GetSISModule()==b->GetSISModule());
   int i=0;
   for (int j=GetSISModule()*32; j<(GetSISModule()+1)*NUM_SIS_CHANNELS; j++)
   {
     this->Counts[i++]+=b->GetCountsInChannel(j);
   }
   return this;
}
void TSISEvent::Print()
{
   printf("RunTime %f \n",RunTime);
   for (int j=0; j<NUM_SIS_CHANNELS; j++)
   {
     if (Counts[j]) printf("Channel %d \t CountInChannel %d \t \n", j, Counts[j] ); 
   }
}
