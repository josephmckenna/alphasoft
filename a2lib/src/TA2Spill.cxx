
#include "TA2Spill.h"
ClassImp(A2Spill);
A2Spill::A2Spill()
{
   IsDumpType=true; //By default, expect this to be a dump
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
   IsDumpType=a->IsDumpType;
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
   if (IsDumpType)
   {
      if (StartTime>0 &&
           StopTime>0 &&
           SISFilled &&
           SVDFilled )
         return true;
      else
         return false;
   }
   else
      return true;
}
void A2Spill::Print()
{
   std::cout<<"Dump name:"<<Name<<"\t\tIsDumpType:"<<IsDumpType<<std::endl;
   std::cout<<"StartTime: "<<StartTime << " StopTime: "<<StopTime <<std::endl;
   std::cout<<"SISFilled: "<<SISFilled << " SVDFilled: "<<SVDFilled <<std::endl;
   std::cout<<"Ready? "<< Ready()<<std::endl;
   std::cout<<"Seq:"<<SequenceNum<<"\t";
   for (int i=0; i<N_COLUMNS; i++)
   {
      std::cout<<DetectorCounts[i]<<"\t";
   }
   
   
   std::cout<<std::endl;
}
TString A2Spill::Content()
{
   char buf[800];
   TString log;
   //if (indent){
    log += "   "; // indentation     
   //}
   
   sprintf(buf,"[%8.3lf-%8.3lf]=%8.3lfs |",StartTime,StopTime,StopTime-StartTime); // timestamps 
   log += buf;

   if (SequenceNum==0)
     sprintf(buf," %-65s|",Name.c_str()); // description 
   else if (SequenceNum==1)
     sprintf(buf," %-16s%-49s|","",Name.c_str()); // description 
   else if (SequenceNum==2)
     sprintf(buf," %-32s%-33s|","",Name.c_str()); // description 
   else if (SequenceNum==3)
     sprintf(buf," %-48s%-17s|","",Name.c_str()); // description 
   log += buf;

   for (int iDet = 0; iDet<N_COLUMNS; iDet++){
     sprintf(buf,"%9d ",DetectorCounts[iDet]);
     log += buf;
   }
   log += "";
   return log;
}
/*TString A2Spill::Header(int TotalSeq)
{

   char buf[300];
   sprintf(buf,"%33s |","Dump Time");
   *log += buf;
   for (int i=0; i<TotalSeq; i++)
   {
      int iSeq=TotalSeq[i];
      sprintf(buf,"%-10s",SeqNames[iSeq].Data());
      *log +=buf;
   }
   *log+="|";

   for (int iDet = 0; iDet<MAXDET; iDet++)
   {
      sprintf(buf,"%-9s ", detectorName[iDet].Data());
      *log += buf;
   }
   TString header=buf;
   return header;
}
TString A2Spill::FormatDump()
{
   
   return "Arr";
}
*/
