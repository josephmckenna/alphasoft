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
TSISEvent::TSISEvent():  fSISModule(0), fClock(0), fVF48Clock(0), fRunTime(-1), fRunNumber(-1),fMidasTime(0), fMidasEventID(0), fCounts(32,0)
{

}
//Default Destructor
TSISEvent::~TSISEvent()
{

}

TSISEvent::TSISEvent( TSISBufferEvent* event ): fSISModule(0), fClock(0), fVF48Clock(0), fRunTime(-1), fRunNumber(-1),fMidasTime(0), fMidasEventID(0)
{
   fCounts = event->Move();
}

//Functions required for manipulating data in the tree
TSISEvent::TSISEvent(  ULong64_t clock, double time): TSISEvent()
{
   SetClock(clock);
   SetRunTime(time);
}

void TSISEvent::ClearSISEvent()
{
   fCounts.clear();
   SetClock(0);
   SetRunTime(-1.);
   SetRunNumber(-1);
   SetMidasUnixTime(0);
}
TSISEvent& TSISEvent::operator+=( const TSISEvent& b)
{
   //Events from differnt SIS modules cannot be added!
   //this->Print();
   //std::cout <<"Adding module "<<b->GetSISModule() << " to "<<this->GetSISModule()<<std::endl;
   assert(this->GetSISModule()==b.GetSISModule());
   if (this->fCounts.size() == 0)
      fCounts = std::vector<uint32_t>(NUM_SIS_CHANNELS,0);
   int i=0;
   for (int j=GetSISModule()*32; j<(GetSISModule()+1)*NUM_SIS_CHANNELS; j++)
   {
     this->fCounts[i++]+=b.GetCountsInChannel(j);
   }
   return *this;
}
void TSISEvent::Print()
{
   printf("RunTime %f \n",fRunTime);
   for (int j=0; j<NUM_SIS_CHANNELS; j++)
   {
     if (fCounts[j]) printf("Channel %d \t CountInChannel %d \t \n", j, fCounts[j] ); 
   }
}
