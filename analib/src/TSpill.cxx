#include "TSpill.h"

ClassImp(TSpillScalerData);
TSpillScalerData::TSpillScalerData()
{
   StartTime         =-1;
   StopTime          =-1;
   FirstVertexEvent  =-1;
   LastVertexEvent   =-1;
   VertexEvents      =-1;
   Verticies         =0;
   PassCuts          =0;
   PassMVA           =0;
   VertexFilled      =false;
}
TSpillScalerData::~TSpillScalerData()
{

}
TSpillScalerData::TSpillScalerData(int n_scaler_channels):DetectorCounts(n_scaler_channels,0),ScalerFilled(n_scaler_channels,0)
{
   StartTime         =-1;
   StopTime          =-1;
   FirstVertexEvent  =-1;
   LastVertexEvent   =-1;
   VertexEvents      =-1;
   Verticies         =0;
   PassCuts          =0;
   PassMVA           =0;
   VertexFilled      =false;
}
TSpillScalerData::TSpillScalerData(TSpillScalerData* a)
{
   StartTime         =a->StartTime;
   StopTime          =a->StopTime;
   DetectorCounts    =a->DetectorCounts;
   ScalerFilled      =a->ScalerFilled;
   
   FirstVertexEvent  =a->FirstVertexEvent;
   LastVertexEvent   =a->LastVertexEvent;
   VertexEvents      =a->VertexEvents;
   Verticies         =a->Verticies;
   PassCuts          =a->PassCuts;
   PassMVA           =a->PassMVA;
   VertexFilled      =a->VertexFilled;
}
template <class ScalerData>
ScalerData* TSpillScalerData::operator/(const ScalerData* b)
{
    ScalerData* c=new ScalerData(this->DetectorCounts.size());
 
    c->VertexFilled=this->VertexFilled;
    c->ScalerFilled=this->ScalerFilled;
 
   c->StartTime=b->StartTime;
   c->StopTime=this->StopTime;

   for (size_t i=0; i<DetectorCounts.size(); i++)
   {
       //std::cout<<this->DetectorCounts[i] << " / "<< b->DetectorCounts[i] <<std::endl;
       if (b->DetectorCounts[i])
          c->DetectorCounts[i]=100*(double)this->DetectorCounts[i]/(double)b->DetectorCounts[i];
       else
          c->DetectorCounts[i]=std::numeric_limits<int>::infinity();
   }
   if (b->VertexEvents)
      c->VertexEvents= 100*(double)this->VertexEvents / (double)b->VertexEvents;
   if (b->Verticies)
      c->Verticies   = 100*(double)this->Verticies  / (double)b->Verticies;
   if (b->PassCuts)
      c->PassCuts    = 100*(double)this->PassCuts   / (double)b->PassCuts;
   if (b->PassMVA)
      c->PassMVA     = 100*(double)this->PassMVA    / (double)b->PassMVA;
   return c;
}
template <class ScalerData>
ScalerData* TSpillScalerData::operator+(const ScalerData* b)
{
    ScalerData* c=new ScalerData(this->DetectorCounts.size());
 
    c->VertexFilled=this->VertexFilled;
    c->ScalerFilled=this->ScalerFilled;
 
   c->StartTime =b->StartTime;
   c->StopTime  =this->StopTime;

   for (size_t i=0; i<DetectorCounts.size(); i++)
   {
       //std::cout<<this->DetectorCounts[i] << " / "<< b->DetectorCounts[i] <<std::endl;
       if (b->DetectorCounts[i])
          c->DetectorCounts[i]=this->DetectorCounts[i]+b->DetectorCounts[i];
       else
          c->DetectorCounts[i]=std::numeric_limits<int>::infinity();
   }
   if (b->VertexEvents)
      c->VertexEvents= this->VertexEvents + b->VertexEvents;
   if (b->Verticies)
      c->Verticies = this->Verticies  + b->Verticies;
   if (b->PassCuts)
      c->PassCuts  = this->PassCuts   + b->PassCuts;
   if (b->PassMVA)
      c->PassMVA   = this->PassMVA    + b->PassMVA;
   return c;
}
bool TSpillScalerData::Ready(bool have_vertex_detector)
{
   bool SomeScalerFilled=false;
   for (auto chan: ScalerFilled)
   {
      if (chan)
      {
         SomeScalerFilled=true;
         break;
      }
   }
   if (StartTime>0 &&
        StopTime>0 &&
        //Some SIS channels filled test
       SomeScalerFilled )
   {  
      if ( VertexFilled || !have_vertex_detector)
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

ClassImp(TA2SpillScalerData);
/*TA2SpillScalerData::TA2SpillScalerData()
{

}*/

TA2SpillScalerData::~TA2SpillScalerData()
{

}

TA2SpillScalerData::TA2SpillScalerData(int n_scaler_channels): TSpillScalerData(n_scaler_channels)
{

}

TA2SpillScalerData::TA2SpillScalerData(TA2SpillScalerData* a): TSpillScalerData((TSpillScalerData*) a)
{

}

TA2SpillScalerData::TA2SpillScalerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_CHANNELS*NUM_SIS_MODULES>* d): TA2SpillScalerData()
{
   for (int i=0; i<NUM_SIS_MODULES; i++)
   {
      TSISEvent* e=d->IntegratedSISCounts[i];
      for (int j=i*NUM_SIS_CHANNELS; j<(i+1)*NUM_SIS_CHANNELS; j++)
      {
         DetectorCounts.at(j)=e->GetCountsInChannel(j);
         ScalerFilled[j]=true;
      }
   }

   if (d->StartDumpMarker)
      StartTime=d->StartDumpMarker->fRunTime;
   if (d->StopDumpMarker)
      StopTime=d->StopDumpMarker->fRunTime;

   FirstVertexEvent  =d->IntegratedSVDCounts.FirstVF48Event;
   LastVertexEvent   =d->IntegratedSVDCounts.LastVF48Event;
   VertexEvents      =d->IntegratedSVDCounts.VF48Events;
   Verticies         =d->IntegratedSVDCounts.Verticies;
   PassCuts          =d->IntegratedSVDCounts.PassCuts;
   PassMVA           =d->IntegratedSVDCounts.PassMVA;
   VertexFilled      =true;
}

ClassImp(TAGSpillScalerData)

/*TAGSpillScalerData::TAGSpillScalerData()
{

}*/
TAGSpillScalerData::~TAGSpillScalerData()
{

}
TAGSpillScalerData::TAGSpillScalerData(int n_scaler_channels)
{
   std::cout<<"JOE IMPLEMENT ME"<<std::endl;
}

TAGSpillScalerData::TAGSpillScalerData(TAGSpillScalerData* a)
{
   std::cout<<"JOE IMPLEMENT ME"<<std::endl;
}
/*TAGSpillScalerData* TAGSpillScalerData::operator/(const TAGSpillScalerData* b)
{
   std::cout<<"JOE IMPLEMENT ME"<<std::endl;
   return NULL;
}*/
TAGSpillScalerData::TAGSpillScalerData(DumpPair<TStoreEvent,TChrono_Event,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d)
{
   std::cout<<"JOE IMPLEMENT ME"<<std::endl;
}
void TAGSpillScalerData::Print()
{
   std::cout<<"LOLOLOL IMPLEMENT THIS YOU FOOL"<<std::endl;
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

ClassImp(TA2SpillSequencerData);
TA2SpillSequencerData::TA2SpillSequencerData(): TSpillSequencerData()
{
}
TA2SpillSequencerData::~TA2SpillSequencerData()
{
}
TA2SpillSequencerData::TA2SpillSequencerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_CHANNELS*NUM_SIS_MODULES>* d)
{
   fSequenceNum= d->StartDumpMarker->fSequencerID;
   fDumpID     = d->dumpID;
   fSeqName ="JOEFIXTHISVARIABLE";
   fStartState = d->StartDumpMarker->fonState;
   fStopState  = d->StopDumpMarker->fonState;
}
void TA2SpillScalerData::Print()
{
   std::cout<<"StartTime: "<<StartTime << " StopTime: "<<StopTime <<std::endl;
   std::cout<<"SISFilled: ";
   for (size_t i=0; i<ScalerFilled.size(); i++)
   {
      std::cout<<ScalerFilled.at(i);
   }
   std::cout  << " SVDFilled: "<<VertexFilled <<std::endl;
   int sum=0;
   for (int i=0; i<64; i++)
      sum+=DetectorCounts[i];
   std::cout<<"SISEntries:"<< sum << "\tSVD Events:"<<VertexEvents<<std::endl;
   for (int i=0; i<64; i++)
   {
      std::cout<<DetectorCounts[i]<<"\t";
   }
}


TA2SpillSequencerData::TA2SpillSequencerData(TA2SpillSequencerData* a)
{
   fSequenceNum  =a->fSequenceNum;
   fDumpID       =a->fDumpID;
   fSeqName      =a->fSeqName;
   fStartState   =a->fStartState;
   fStopState    =a->fStopState;
}

ClassImp(TSpill);
TSpill::TSpill()
{
   IsDumpType =true; //By default, expect this to be a dump
   IsInfoType =false;
   Unixtime   =0;
}
TSpill::TSpill(const char* format, ...)
{
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}
void TSpill::InitByName(const char* format, va_list args)
{
   char buf[256];
   vsnprintf(buf,255,format,args);
   Name       =buf;
   Unixtime   =0;
   IsDumpType =false; //By default, expect this to be a information if given a string at construction
   IsInfoType =false;
   std::cout<<"NewSpill:";
   Print();
}

TSpill::TSpill(TSpill* a)
{
   RunNumber    =a->RunNumber;
   Name         =a->Name;
   IsDumpType   =a->IsDumpType;
   Unixtime     =a->Unixtime;
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
bool TSpill::DumpHasMathSymbol() const
{
   char symbols[5]="+/*-";
   for (const char c: this->Name)
   {
      for (int i=0; i<4; i++)
      {
         //std::cout<<c<<"=="<<symbols[i]<<std::endl;
         if (c==symbols[i])
            return true;
      }
   }
   return false;
}

TSpill::~TSpill()
{

}


ClassImp(TA2Spill);
TA2Spill::TA2Spill()
{
   SeqData    =NULL;
   ScalerData =NULL;
}

TA2Spill::TA2Spill(const char* format, ...)
{
   SeqData    =NULL;
   ScalerData =NULL;
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}

TA2Spill::TA2Spill(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_CHANNELS*NUM_SIS_MODULES>* d ): TSpill(d->StartDumpMarker->Description.c_str())
{
   if (d->StartDumpMarker && d->StopDumpMarker) IsDumpType=true;
   ScalerData = new TA2SpillScalerData(d);
   SeqData = new TA2SpillSequencerData(d);
   //Print();
}

TA2Spill::TA2Spill(const TA2Spill* a): TSpill((TSpill*)a)
{
   if (a->ScalerData)
      ScalerData=new TA2SpillScalerData(a->ScalerData);
   else
      ScalerData=NULL;
   if (a->SeqData)
      SeqData=new TA2SpillSequencerData(a->SeqData);
   else
      SeqData=NULL;

}
TA2Spill::~TA2Spill()
{
   if (ScalerData)
      delete ScalerData;
}

#include "assert.h"


TA2Spill* TA2Spill::operator/( TA2Spill* b)
{
   //c=a/b
   TA2Spill* c=new TA2Spill();
   c->IsInfoType=true;
   assert(this->RunNumber == b->RunNumber);
   c->RunNumber=this->RunNumber;

   assert(this->ScalerData->ScalerFilled.size());
   assert(b->ScalerData->ScalerFilled.size());

   assert(this->ScalerData->VertexFilled);
   assert(b->ScalerData->VertexFilled);
   if (DumpHasMathSymbol())
      Name='('+Name+')';
   if (b->DumpHasMathSymbol())
   {
      b->Name='('+b->Name+')';
   }
   char dump_name[200];
   sprintf(dump_name,"%s / %s (%%)",this->Name.c_str(),b->Name.c_str());
   c->Name=dump_name;

   c->ScalerData =*ScalerData / b->ScalerData;

   c->IsDumpType=false;
   return c;
}
TA2Spill* TA2Spill::operator+( TA2Spill* b)
{
   //c=a/b
   TA2Spill* c=new TA2Spill();
   c->IsInfoType=true;
   assert(this->RunNumber == b->RunNumber);
   c->RunNumber=this->RunNumber;

   assert(this->ScalerData->ScalerFilled.size());
   assert(b->ScalerData->ScalerFilled.size());

   assert(this->ScalerData->VertexFilled);
   assert(b->ScalerData->VertexFilled);

   if (DumpHasMathSymbol())
      Name='('+Name+')';

   if (b->DumpHasMathSymbol())
      b->Name='('+b->Name+')';

   char dump_name[200];
   sprintf(dump_name,"%s + %s",this->Name.c_str(),b->Name.c_str());

   c->Name=dump_name;

   c->ScalerData =*ScalerData + b->ScalerData;

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
      int open=Name.find_last_of('(');
      int close=Name.find_last_of(')');
      if (open>0 &&       //Have open bracket
         close > open &&  //Have close bracket
         close - open<6)  //Units less than 6 characters long
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

ClassImp(TAGSpill)
TAGSpill::TAGSpill()
{

}
TAGSpill::~TAGSpill()
{
}
TAGSpill::TAGSpill(TAGSpill* a)
{

}
TString TAGSpill::Content(std::vector<std::pair<int,int>>* chrono_channels, int& n_chans)
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
         int board=chrono_channels->at(iDet).first;
         int chan=chrono_channels->at(iDet).second;
         //If valid channel number:
         if (chan>0)
            counts=ScalerData->DetectorCounts[board*CHRONO_N_CHANNELS+chan];
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