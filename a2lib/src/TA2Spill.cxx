
#include "TA2Spill.h"
ClassImp(A2Spill);
A2Spill::A2Spill()
{
   SequenceNum=-1;
   StartTime=-1.;
   StopTime=-1.;
   SISFilled=false;
   SVDFilled=false;
   for (int i=0; i<N_COLUMNS; i++)
   {
      DetectorCounts[i]=0;
   }
}

A2Spill::A2Spill(A2Spill* a)
{
   SequenceNum=a->SequenceNum;
   StartTime=a->StartTime;
   StopTime=a->StopTime;
   SISFilled=a->SISFilled;
   SVDFilled=a->SVDFilled;
   for (int i=0; i<N_COLUMNS; i++)
   {
       DetectorCounts[i]=a->DetectorCounts[i];
   }
}

bool A2Spill::Ready()
{
   if (StartTime>0 &&
        StopTime>0 &&
        SISFilled &&
        SVDFilled )
      return true;
   else
      return false;
}
void A2Spill::Print()
{
   
   std::cout<<"Seq:"<<SequenceNum<<"\t";
   for (int i=0; i<N_COLUMNS; i++)
   {
      std::cout<<DetectorCounts[i]<<"\t";
   }
   std::cout<<"StartTime: "<<StartTime << " StopTime: "<<StopTime <<std::endl;
   std::cout<<"SISFilled: "<<SISFilled << " SVDFilled: "<<SVDFilled <<std::endl;
   std::cout<<"Ready? "<< Ready()<<std::endl;
   
   std::cout<<std::endl;
}
