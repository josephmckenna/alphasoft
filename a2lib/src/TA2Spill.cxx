
#include "TA2Spill.h"
ClassImp(A2Spill);
A2Spill::A2Spill()
{
   IsDumpType=true; //By default, expect this to be a dump
   SequenceNum=-1;
   StartTime=-1.;
   StopTime=-1.;
   SISFilled=0;
   SVDFilled=false;
   for (int i=0; i<64; i++)
   {
      DetectorCounts[i]=0;
   }
   VF48Events=0;
   Verticies=0;
   PassCuts=0;
   PassMVA=0;
}

A2Spill::A2Spill(A2Spill* a)
{
   IsDumpType=a->IsDumpType;
   SequenceNum=a->SequenceNum;
   StartTime=a->StartTime;
   StopTime=a->StopTime;
   SISFilled=a->SISFilled;
   SVDFilled=a->SVDFilled;
   for (int i=0; i<64; i++)
   {
       DetectorCounts[i]=a->DetectorCounts[i];
   }
   VF48Events=a->VF48Events;
   Verticies=a->Verticies;
   PassCuts=a->PassCuts;
   PassMVA=a->PassMVA;
}

bool A2Spill::Ready( bool have_svd)
{
   if (IsDumpType)
   {
      if (StartTime>0 &&
           StopTime>0 &&
           SISFilled & (unsigned long) -1)
      {  
         if ( SVDFilled || !have_svd)
            return true;
         //If there is no SVD data, then SVDFilled is false... 
         //wait for the SIS data to be atleast 'data_buffer_time' seconds
         //ahead to check there is no SVD data...
         //if ( !SVDFilled && T>StopTime+data_buffer_time )
         //   return true;
         return false;
      }
      else
      {
         return false;
      }

   }
   else
      return true;
}
void A2Spill::Print()
{
   std::cout<<"Dump name:"<<Name<<"\t\tIsDumpType:"<<IsDumpType<<std::endl;
   std::cout<<"StartTime: "<<StartTime << " StopTime: "<<StopTime <<std::endl;
   std::cout<<"SISFilled: "<<(std::bitset<64>)SISFilled << " SVDFilled: "<<SVDFilled <<std::endl;
   std::cout<<"Ready? "<< Ready(true) << " " << Ready(false)<<std::endl;
   std::cout<<"Seq:"<<SequenceNum<<"\t";
   for (int i=0; i<N_COLUMNS; i++)
   {
      std::cout<<DetectorCounts[i]<<"\t";
   }
   
   
   std::cout<<std::endl;
}
TString A2Spill::Content(std::vector<int>* sis_channels, int& n_chans)
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

   for (int iDet = 0; iDet<n_chans; iDet++){
     int counts=-1;
     int chan=sis_channels->at(iDet);
     //If valid channel number:
     if (chan>0)
        counts=DetectorCounts[chan];
     sprintf(buf,"%9d ",counts);
     log += buf;
   }
   sprintf(buf,"%9d ",PassCuts);
   log += buf;
   sprintf(buf,"%9d ",PassMVA);
   log += buf;
   log += "";
   return log;
}


int A2Spill::AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt)
{

   TString sqlstatement = "INSERT INTO DumpTable VALUES ";
   sqlstatement+= "(";
   sqlstatement+= RunNumber;
   sqlstatement+= ",";
   sqlstatement+= SequenceNum;
   sqlstatement+= ",";
   sqlstatement+= DumpID;
   sqlstatement+= ",";
   sqlstatement+= Name;
   sqlstatement+= ",";
   sqlstatement+= Unixtime;
   sqlstatement+= ",";
   sqlstatement+= StartTime;
   sqlstatement+= ",";
   sqlstatement+= StopTime;
   for (int i=0;i<64; i++)
   {
      sqlstatement+= ",";
      sqlstatement+=DetectorCounts[i];
   }
   sqlstatement+=",";
   sqlstatement+=PassCuts;
   sqlstatement+=",";
   sqlstatement+=PassMVA;
   sqlstatement+=");";
   std::cout<<"HELLO!!\t"<<sqlstatement<<std::endl;
   
   sqlite3_prepare( db, sqlstatement.Data(), -1, &stmt, NULL );//preparing the statement
   sqlite3_step( stmt );//executing the statement
   
   sqlite3_finalize(stmt);
   return 0;
}
