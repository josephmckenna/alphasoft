#include "TA2Spill.h"

#ifdef BUILD_A2
ClassImp(TA2SpillScalerData)
/*TA2SpillScalerData::TA2SpillScalerData()
{

}*/

TA2SpillScalerData::~TA2SpillScalerData()
{

}

TA2SpillScalerData::TA2SpillScalerData(int n_scaler_channels): TSpillScalerData(n_scaler_channels)
{

}

TA2SpillScalerData::TA2SpillScalerData(const TA2SpillScalerData& a): TSpillScalerData(a)
{

}

TA2SpillScalerData::TA2SpillScalerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d): TA2SpillScalerData()
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

ClassImp(TA2SpillSequencerData)

TA2SpillSequencerData::TA2SpillSequencerData(): TSpillSequencerData()
{
}
TA2SpillSequencerData::~TA2SpillSequencerData()
{
}
TA2SpillSequencerData::TA2SpillSequencerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d)
{
   fSequenceNum= d->StartDumpMarker->fSequencerID;
   fDumpID     = d->dumpID;
   if ( fSequenceNum < 0 )
      fSeqName = "Sequencer Unknown";
   else
      fSeqName    = SeqNames.at(fSequenceNum);
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


TA2SpillSequencerData::TA2SpillSequencerData(const TA2SpillSequencerData& a) : TSpillSequencerData(a)
{
   fSequenceNum  =a.fSequenceNum;
   fDumpID       =a.fDumpID;
   fSeqName      =a.fSeqName;
   fStartState   =a.fStartState;
   fStopState    =a.fStopState;
}

ClassImp(TA2Spill)

TA2Spill::TA2Spill()
{
   SeqData    =NULL;
   ScalerData =NULL;
}
TA2Spill::TA2Spill(int runno, uint32_t unixtime): TSpill(runno, unixtime)
{
   SeqData    =NULL;
   ScalerData =NULL;
}

TA2Spill::TA2Spill(int runno, uint32_t unixtime, const char* format, ...): TSpill(runno,unixtime)
{
   SeqData    =NULL;
   ScalerData =NULL;
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}

TA2Spill::TA2Spill(int runno,DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d ):
   TSpill(runno, d->StartDumpMarker->fMidasTime, d->StartDumpMarker->fDescription.c_str())
{
   if (d->StartDumpMarker && d->StopDumpMarker) IsDumpType=true;
   ScalerData = new TA2SpillScalerData(d);
   SeqData = new TA2SpillSequencerData(d);
   //Print();
}

TA2Spill::TA2Spill(const TA2Spill& a): TSpill(a)
{
   if (a.ScalerData)
      ScalerData=new TA2SpillScalerData(*a.ScalerData);
   else
      ScalerData=NULL;
   if (a.SeqData)
      SeqData=new TA2SpillSequencerData(*a.SeqData);
   else
      SeqData=NULL;
}
TA2Spill::~TA2Spill()
{
   if (ScalerData)
      delete ScalerData;
   ScalerData=NULL;
   if (SeqData)
      delete SeqData;
   SeqData=NULL;
}

#include "assert.h"


TA2Spill* TA2Spill::operator/( TA2Spill* b)
{
   //c=a/b
   TA2Spill* c=new TA2Spill(this->RunNumber, this->Unixtime);
   c->IsInfoType=true;
   assert(this->RunNumber == b->RunNumber);

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
   TA2Spill* c=new TA2Spill(this->RunNumber, this->Unixtime);
   c->IsInfoType=true;
   assert(this->RunNumber == b->RunNumber);

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
      std::string dump_name;
      for (int i = 0; i < SeqData->fSequenceNum; i++)
         dump_name += std::string(4,' ');
      dump_name += Name; // dump description
      if (dump_name.size() > DUMP_NAME_WIDTH)
         dump_name = dump_name.substr(0,DUMP_NAME_WIDTH);
      else if (dump_name.size() < DUMP_NAME_WIDTH)
         dump_name.insert(dump_name.size(), DUMP_NAME_WIDTH - dump_name.size(), ' ');
      log += dump_name + std::string("|");
   }
   else
   {
      if (ScalerData)
         log += std::string(DUMP_NAME_WIDTH,' ') + std::string("|") + Name;
      else
         log += std::string(" ") + Name;
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
#endif
