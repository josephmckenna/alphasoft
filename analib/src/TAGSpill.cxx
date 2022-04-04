#include "TAGSpill.h"

#if BUILD_AG
ClassImp(TAGSpill)

TAGSpill::TAGSpill()
{
   SeqData    =NULL;
   ScalerData =NULL;
}
TAGSpill::TAGSpill(int runno, uint32_t unixtime): TSpill(runno, unixtime)
{
   SeqData    =NULL;
   ScalerData =NULL;
}

TAGSpill::TAGSpill(int runno, uint32_t unixtime, const char* format, ...):
   TSpill(runno,unixtime)
{
   SeqData    =NULL;
   ScalerData =NULL;
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}

TAGSpill::TAGSpill(int runno, TDumpMarkerPair<TAGDetectorEvent,TChronoBoardCounter,CHRONO_N_BOARDS>* d):
   TSpill(runno, d->fStartDumpMarker->fMidasTime, d->fStartDumpMarker->fDescription.c_str())
{
   
   if (d->fStartDumpMarker && d->fStopDumpMarker) IsDumpType=true;
   ScalerData = new TAGSpillScalerData(d);
   SeqData = new TAGSpillSequencerData(d);
   //Print();
}

TAGSpill::TAGSpill(const TAGSpill& a):
   TSpill(a)
{
   if (a.ScalerData)
      ScalerData=new TAGSpillScalerData(*a.ScalerData);
   else
      ScalerData=NULL;
   if (a.SeqData)
      SeqData=new TAGSpillSequencerData(*a.SeqData);
   else
      SeqData=NULL;
}

TAGSpill::~TAGSpill()
{
   if (ScalerData)
      delete ScalerData;
   ScalerData=NULL;
   if (SeqData)
      delete SeqData;
   SeqData=NULL;
}

#include "assert.h"


TAGSpill* TAGSpill::operator/( TAGSpill* b)
{
   //c=a/b
   TAGSpill* c=new TAGSpill(this->RunNumber, this->Unixtime);
   c->IsInfoType=true;
   assert(this->RunNumber == b->RunNumber);

   assert(this->ScalerData->ScalerFilled.size());
   assert(b->ScalerData->ScalerFilled.size());

   //assert(this->ScalerData->BVFilled);
   //assert(b->ScalerData->BVFilled);
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

TAGSpill* TAGSpill::operator+( TAGSpill* b)
{
   //c=a/b
   TAGSpill* c=new TAGSpill(this->RunNumber, this->Unixtime);
   c->IsInfoType=true;
   assert(this->RunNumber == b->RunNumber);

   assert(this->ScalerData->ScalerFilled.size());
   assert(b->ScalerData->ScalerFilled.size());

   //assert(this->ScalerData->BVFilled);
   //assert(b->ScalerData->BVFilled);

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

bool TAGSpill::Ready( bool have_BV)
{
   if (IsDumpType)
   {
      return ScalerData->Ready(have_BV);
   }
   else
   {
      return true;
   }
}

void TAGSpill::Print()
{
   std::cout<<"Dump name:"<<Name<<"\t\tIsDumpType:"<<IsDumpType<<std::endl;
   if (SeqData)
      SeqData->Print();
   if (ScalerData)
      ScalerData->Print();
   std::cout<<"Ready? "<< Ready(true) << " " << Ready(false)<<std::endl;
   std::cout<<std::endl;
}

TString TAGSpill::Content(std::vector<TChronoChannel> chrono_channels)
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
      for (const TChronoChannel& c: chrono_channels)
      {
         int counts=-1;
         //If valid channel number:
         if (c.GetChannel() >= 0)
            counts = ScalerData->fDetectorCounts.at(c.GetBoardNumber() * CHRONO_N_CHANNELS + c.GetChannel());
         sprintf(buf,"%9d",counts);
         log += buf;
         if (units.size())
            log += units;
         else
            log += " ";
      }
      //ALPHA G is not fast enought for vertex data yet
      //sprintf(buf,"%9d ",ScalerData->fPassCuts);
      //log += buf;
      //sprintf(buf,"%9d ",ScalerData->fPassMVA);
      //log += buf;
      log += "";
   }
   return log;
}
#endif
