#include "TA2Spill.h"

#ifdef BUILD_A2

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

TA2Spill::TA2Spill(int runno, uint32_t unixtime, const char* format, ...):
   TSpill(runno,unixtime)
{
   SeqData    =NULL;
   ScalerData =NULL;
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}

TA2Spill::TA2Spill(int runno,TDumpMarkerPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d ):
   TSpill(runno, d->fStartDumpMarker->fMidasTime, d->fStartDumpMarker->fDescription.c_str())
{
   if (d->fStartDumpMarker && d->fStopDumpMarker) IsDumpType=true;
   ScalerData = new TA2SpillScalerData(d);
   SeqData = new TA2SpillSequencerData(d);
   //Print();
}

TA2Spill::TA2Spill(const TA2Spill& a):
   TSpill(a)
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

TString TA2Spill::Content(const std::vector<TSISChannel> sis_channels)
{
   
   TString log;
   //if (indent){
    log += "   "; // indentation     
   //}
   std::string units="";
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
      char buf[200];
      sprintf(buf,"[%8.3lf-%8.3lf]=%8.3lfs |",
                 ScalerData->fStartTime,
                 ScalerData->fStopTime,
                 ScalerData->fStopTime-ScalerData->fStartTime
                 ); // timestamps 
      log += buf;
   }
   else
   {
      if (Unixtime)
      {
         char buf[80];
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
      log += std::string(" ") + Name;
   }
   if (ScalerData)
   {
      char buf[80];
      for (const TSISChannel& chan: sis_channels)
      {
         int counts=-1;
         //If valid channel number:
         if (chan.IsValid())
            counts=ScalerData->fDetectorCounts[chan.toInt()]; //toInt is a pretty unclean / unsafe solution. Look for [] operator overload
         sprintf(buf,"%9d",counts);
         log += buf;
         if (units.size())
            log += units;
         else
            log += " ";
      }
      sprintf(buf,"%9d ",ScalerData->fPassCuts);
      log += buf;
      sprintf(buf,"%9d ",ScalerData->fPassMVA);
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
   sqlstatement+= ScalerData->fStartTime;
   sqlstatement+= ",";
   sqlstatement+= ScalerData->fStopTime;
   for (int i=0;i<64; i++)
   {
      sqlstatement+= ",";
      sqlstatement+=ScalerData->fDetectorCounts[i];
   }
   sqlstatement+=",";
   sqlstatement+=ScalerData->fPassCuts;
   sqlstatement+=",";
   sqlstatement+=ScalerData->fPassMVA;
   sqlstatement+=");";
   std::cout<<"HELLO!!\t"<<sqlstatement<<std::endl;
   
   sqlite3_prepare( db, sqlstatement.Data(), -1, &stmt, NULL );//preparing the statement
   sqlite3_step( stmt );//executing the statement
   
   sqlite3_finalize(stmt);
   return 0;
}
#endif
