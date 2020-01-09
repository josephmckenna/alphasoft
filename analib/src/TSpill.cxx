#include "TSpill.h"

ClassImp(TSpillScalerData);
TSpillScalerData::TSpillScalerData()
{
   StartTime=-1;
   StopTime=-1;
}
TSpillScalerData::~TSpillScalerData()
{

}
ClassImp(TA2SpillScalerData);
TA2SpillScalerData::TA2SpillScalerData()
{
   SISFilled=0;
   SVDFilled=false;
   for (int i=0; i<64; i++)
   {
      DetectorCounts[i]=0;
   }
   FirstVF48Event=-1;
   LastVF48Event=-1;
   VF48Events=0;
   Verticies=0;
   PassCuts=0;
   PassMVA=0;
}
TA2SpillScalerData::TA2SpillScalerData(TA2SpillScalerData* a)
{
   StartTime    =a->StartTime;
   StopTime     =a->StopTime;
   SISFilled    =a->SISFilled;
   SVDFilled    =a->SVDFilled;
   for (int i=0; i<64; i++)
   {
       DetectorCounts[i]=a->DetectorCounts[i];
   }
   VF48Events   =a->VF48Events;
   Verticies    =a->Verticies;
   PassCuts     =a->PassCuts;
   PassMVA      =a->PassMVA;
}
TA2SpillScalerData* TA2SpillScalerData::operator/(const TA2SpillScalerData* b)
{
   TA2SpillScalerData* c=new TA2SpillScalerData();

   c->SVDFilled=this->SVDFilled;
   c->SISFilled=this->SISFilled;

   c->StartTime=b->StartTime;
   c->StopTime=this->StopTime;

   for (int i=0; i<64; i++)
   {
       //std::cout<<this->DetectorCounts[i] << " / "<< b->DetectorCounts[i] <<std::endl;
       if (b->DetectorCounts[i])
          c->DetectorCounts[i]=100*(double)this->DetectorCounts[i]/(double)b->DetectorCounts[i];
       else
          c->DetectorCounts[i]=std::numeric_limits<int>::infinity();
   }
   if (b->VF48Events)
      c->VF48Events= 100*(double)this->VF48Events / (double)b->VF48Events;
   if (b->Verticies)
      c->Verticies = 100*(double)this->Verticies  / (double)b->Verticies;
   if (b->PassCuts)
      c->PassCuts  = 100*(double)this->PassCuts   / (double)b->PassCuts;
   if (b->PassMVA)
      c->PassMVA   = 100*(double)this->PassMVA    / (double)b->PassMVA;
   return c;
}
void TA2SpillScalerData::AddData(const SVD_Counts& c)
{
   if (FirstVF48Event<0) FirstVF48Event=c.VF48EventNo;
   LastVF48Event=c.VF48EventNo;
   VF48Events++;
   Verticies+=c.has_vertex;
   PassCuts+=c.passed_cuts;
   PassMVA+=c.online_mva;
}
void TA2SpillScalerData::AddData(const SIS_Counts& c, const int &channel)
{
   DetectorCounts[channel]+=c.counts;
}

bool TA2SpillScalerData::Ready(bool have_svd)
{
   if (StartTime>0 &&
        StopTime>0 &&
        //Some SIS channels filled test
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

TA2SpillScalerData::~TA2SpillScalerData()
{

}
void TA2SpillScalerData::Print()
{
   std::cout<<"StartTime: "<<StartTime << " StopTime: "<<StopTime <<std::endl;
   std::cout<<"SISFilled: "<<(std::bitset<64>)SISFilled << " SVDFilled: "<<SVDFilled <<std::endl;
   int sum=0;
   for (int i=0; i<64; i++)
      sum+=DetectorCounts[i];
   std::cout<<"SISEntries:"<< sum << "\tSVD Events:"<<VF48Events<<std::endl;
   for (int i=0; i<64; i++)
   {
      std::cout<<DetectorCounts[i]<<"\t";
   }
}

ClassImp(TSpillSequencerData);
TSpillSequencerData::TSpillSequencerData()
{
   fSequenceNum=-1;
   fDumpID     =-1;
   fSeqName    ="";
   fStartState =-1;
   fStopState  =-1;
}
TSpillSequencerData::TSpillSequencerData(TSpillSequencerData* a)
{
   fSequenceNum  =a->fSequenceNum;
   fDumpID       =a->fDumpID;
   fSeqName      =a->fSeqName;
   fStartState   =a->fStartState;
   fStopState    =a->fStopState;
}

TSpillSequencerData::~TSpillSequencerData()
{
}

void TSpillSequencerData::Print()
{
   std::cout<<"SeqName:"       <<fSeqName
            <<"\tSeq:"         <<fSequenceNum
            <<"\tDumpID:"      <<fDumpID
            <<"\tstartState:"  <<fStartState
            <<"\tstopState:"   <<fStopState
            <<std::endl;
}

ClassImp(TSpill);
TSpill::TSpill()
{
   IsDumpType =true; //By default, expect this to be a dump
   IsInfoType =false;
   SeqData    =NULL;
   Unixtime   =0;
}
TSpill::TSpill(const char* name, int unixtime)
{
   Name       =name;
   Unixtime   =unixtime;
   IsDumpType =false; //By default, expect this to be a information if given a string at construction
   IsInfoType =false;
   SeqData    =NULL;
}

TSpill::TSpill(TSpill* a)
{
   RunNumber    =a->RunNumber;
   Name         =a->Name;
   IsDumpType   =a->IsDumpType;
   Unixtime     =a->Unixtime;
   if (a->SeqData)
      SeqData=new TSpillSequencerData(a->SeqData);
   else
      SeqData=NULL;
}

TSpill* TSpill::operator/(const TSpill* b)
{
   //Joe you need to set this up
   TSpill* a = new TSpill();
   return a;
}

void TSpill::Print()
{
   std::cout<<"RunNumber:"<<RunNumber<<"\t"
            <<"Name:"<<Name<<"\t"
            <<std::endl;
}

int TSpill::AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt)
{
   return -1;
}
TString TSpill::Content(std::vector<int>*, int& )
{
   return Name;
}


TSpill::~TSpill()
{

}


ClassImp(TA2Spill);
TA2Spill::TA2Spill()
{
   ScalerData =NULL;
}

TA2Spill::TA2Spill(const char* name, int unixtime): TSpill(name,unixtime)
{
   ScalerData =NULL;
}

TA2Spill::TA2Spill(const TA2Spill* a): TSpill((TSpill*)a)
{
   if (a->ScalerData)
      ScalerData=new TA2SpillScalerData(a->ScalerData);
   else
      ScalerData=NULL;
}
TA2Spill::~TA2Spill()
{
   if (ScalerData)
      delete ScalerData;
}

#include "assert.h"
TA2Spill* TA2Spill::operator/(const TA2Spill* b)
{
   //c=a/b
   TA2Spill* c=new TA2Spill();
   c->IsInfoType=true;
   assert(this->RunNumber == b->RunNumber);
   c->RunNumber=this->RunNumber;

   assert(this->ScalerData->SISFilled);
   assert(b->ScalerData->SISFilled);

   assert(this->ScalerData->SVDFilled);
   assert(b->ScalerData->SVDFilled);

   char dump_name[200];
   sprintf(dump_name,"%s / %s (%%)",this->Name.c_str(),b->Name.c_str());
   c->Name=dump_name;
   
   c->ScalerData =*ScalerData / b->ScalerData;

   c->IsDumpType=false;
   return c;
}

bool TA2Spill::Ready( bool have_svd)
{
   if (IsDumpType)
   {
      return ScalerData->Ready(have_svd);
   }
   else
   {
      return true;
   }
}
void TA2Spill::Print()
{
   std::cout<<"Dump name:"<<Name<<"\t\tIsDumpType:"<<IsDumpType<<std::endl;
   if (SeqData)
      SeqData->Print();
   if (ScalerData)
      ScalerData->Print();
   std::cout<<"Ready? "<< Ready(true) << " " << Ready(false)<<std::endl;
   std::cout<<std::endl;
}

TString TA2Spill::Content(std::vector<int>* sis_channels, int& n_chans)
{
   char buf[800];
   TString log;
   //if (indent){
    log += "   "; // indentation     
   //}
   TString units="";
   {
      int open=Name.find('(');
      int close=Name.find(')');
      if (open>0 && close > open)
      units=(Name.substr (open+1,close-open-1));
   }
   if (ScalerData)
   {
      sprintf(buf,"[%8.3lf-%8.3lf]=%8.3lfs |",
                 ScalerData->StartTime,
                 ScalerData->StopTime,
                 ScalerData->StopTime-ScalerData->StartTime
                 ); // timestamps 
      log += buf;
   }
   else
   {
      if (Unixtime)
      {
         struct tm * timeinfo = ( (tm*) &Unixtime);
         sprintf(buf,"[%d:%d:%d]", timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
         log += buf;
      }
   }
   if (SeqData)
   {
      if (SeqData->fSequenceNum==0)
        sprintf(buf," %-65s|",Name.c_str()); // description 
      else if (SeqData->fSequenceNum==1)
        sprintf(buf," %-16s%-49s|","",Name.c_str()); // description 
      else if (SeqData->fSequenceNum==2)
        sprintf(buf," %-32s%-33s|","",Name.c_str()); // description 
      else if (SeqData->fSequenceNum==3)
        sprintf(buf," %-48s%-17s|","",Name.c_str()); // description 
      log += buf;
   }
   else
   {
      if (ScalerData)
         sprintf(buf," %-65s|",Name.c_str());
      else
         sprintf(buf," %s",Name.c_str());
      log += buf;
   }
   if (ScalerData)
   {
      for (int iDet = 0; iDet<n_chans; iDet++)
      {
         int counts=-1;
         int chan=sis_channels->at(iDet);
         //If valid channel number:
         if (chan>0)
            counts=ScalerData->DetectorCounts[chan];
         sprintf(buf,"%9d%s ",counts,units.Data());
         log += buf;
      }
      sprintf(buf,"%9d ",ScalerData->PassCuts);
      log += buf;
      sprintf(buf,"%9d ",ScalerData->PassMVA);
      log += buf;
      log += "";
   }
   return log;
}


int TA2Spill::AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt)
{
   if (!SeqData) return -1;
   if (!ScalerData) return -1;
   TString sqlstatement = "INSERT INTO DumpTable VALUES ";
   sqlstatement+= "(";
   sqlstatement+= RunNumber;
   sqlstatement+= ",";
   sqlstatement+= SeqData->fSequenceNum;
   sqlstatement+= ",";
   sqlstatement+= SeqData->fDumpID;
   sqlstatement+= ",";
   sqlstatement+= Name;
   sqlstatement+= ",";
   sqlstatement+= Unixtime;
   sqlstatement+= ",";
   sqlstatement+= ScalerData->StartTime;
   sqlstatement+= ",";
   sqlstatement+= ScalerData->StopTime;
   for (int i=0;i<64; i++)
   {
      sqlstatement+= ",";
      sqlstatement+=ScalerData->DetectorCounts[i];
   }
   sqlstatement+=",";
   sqlstatement+=ScalerData->PassCuts;
   sqlstatement+=",";
   sqlstatement+=ScalerData->PassMVA;
   sqlstatement+=");";
   std::cout<<"HELLO!!\t"<<sqlstatement<<std::endl;
   
   sqlite3_prepare( db, sqlstatement.Data(), -1, &stmt, NULL );//preparing the statement
   sqlite3_step( stmt );//executing the statement
   
   sqlite3_finalize(stmt);
   return 0;
}
