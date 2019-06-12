/*
 *  TSISModule.cpp
 *
 *  Created by JTK McKenna
 *
 */

#include "SISModule.h"


//Default Destructor
SISModule::~SISModule()
{
}

//Functions required for manipulating data in the tree
SISModule::SISModule( int mod, ULong64_t clock, double time)
{
  Module = mod;
  ClearSISModule();
  SetClock(clock);
  SetRunTime(time);
}

void SISModule::ClearSISModule()
{
    for (int j=0; j<NUM_SIS_CHANNELS; j++)
       Counts[j]=0;
    SetClock(0);
    SetRunTime(-1.);
}

void SISModule::Print()
{
  printf("RunTime %f \n",RunTime);
     printf("Sis: %d\n",Module);
     for (int j=0; j<NUM_SIS_CHANNELS; j++)
     {
       if (Counts[j]) printf("Channel %d \t CountInChannel %d \t \n", j, Counts[j] ); 
     }

}
