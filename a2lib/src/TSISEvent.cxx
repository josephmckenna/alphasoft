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
TSISEvent::TSISEvent():  fSISModule(0), fCounts{0}, fClock(0), fVF48Clock(0), fRunTime(-1), fRunNumber(-1),fMidasTime(0), fMidasEventID(0)
{

}

TSISEvent::TSISEvent(int SISModule):  fSISModule(SISModule), fCounts{0}, fClock(0), fVF48Clock(0), fRunTime(-1), fRunNumber(-1),fMidasTime(0), fMidasEventID(0)
{
assert ( SISModule < NUM_SIS_MODULES);
}


//Default Destructor
TSISEvent::~TSISEvent()
{

}

TSISEvent::TSISEvent(const TSISBufferEvent& event ): fSISModule(event.fSISModule), fClock(0), fVF48Clock(0), fRunTime(-1), fRunNumber(-1),fMidasTime(0), fMidasEventID(0)
{
   std::copy(event.fCounts.begin(),event.fCounts.end(), fCounts.begin());
}

//Functions required for manipulating data in the tree
TSISEvent::TSISEvent(  ULong64_t clock, double time): TSISEvent()
{
   SetClock(clock);
   SetRunTime(time);
}

void TSISEvent::ClearSISEvent()
{
   fSISModule = -1;
   for (uint32_t & c: fCounts)
      c = 0;
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
   for (int i = 0; i <  NUM_SIS_CHANNELS; i++)
      this->fCounts[i] += b.fCounts[i];
   return *this;
}
void TSISEvent::Print()
{
   printf("RunTime %f Module %d\n",fRunTime,fSISModule);
   for (int j=0; j<NUM_SIS_CHANNELS; j++)
   {
     if (fCounts[j]) printf("Channel %d \t CountInChannel %d \t \n", j, fCounts[j] ); 
   }
}
