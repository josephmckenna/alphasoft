#include "TAGSpill.h"


#ifdef BUILD_AG
ClassImp(TAGSpillScalerData)

/*TAGSpillScalerData::TAGSpillScalerData()
{

}*/
TAGSpillScalerData::~TAGSpillScalerData()
{

}
TAGSpillScalerData::TAGSpillScalerData(int n_scaler_channels): TSpillScalerData(n_scaler_channels)
{

}

TAGSpillScalerData::TAGSpillScalerData(TAGSpillScalerData* a): TSpillScalerData((TSpillScalerData*) a)
{

}
/*TAGSpillScalerData* TAGSpillScalerData::operator/(const TAGSpillScalerData* b)
{
   std::cout<<"JOE IMPLEMENT ME"<<std::endl;
   return NULL;
}*/
TAGSpillScalerData::TAGSpillScalerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d): TAGSpillScalerData()
{
   for (int i=0; i<CHRONO_N_BOARDS*CHRONO_N_CHANNELS; i++)
   {
      ChronoEvent* e=d->IntegratedSISCounts[i];
      DetectorCounts.at(i)=e->Counts;
      ScalerFilled[i]=true;
   }

   if (d->StartDumpMarker)
      StartTime=d->StartDumpMarker->fRunTime;
   if (d->StopDumpMarker)
      StopTime=d->StopDumpMarker->fRunTime;

   /*FirstVertexEvent  =d->IntegratedSVDCounts.FirstVF48Event;
   LastVertexEvent   =d->IntegratedSVDCounts.LastVF48Event;
   VertexEvents      =d->IntegratedSVDCounts.VF48Events;
   Verticies         =d->IntegratedSVDCounts.Verticies;
   PassCuts          =d->IntegratedSVDCounts.PassCuts;
   PassMVA           =d->IntegratedSVDCounts.PassMVA;*/
   VertexFilled      =true;
}
void TAGSpillScalerData::Print()
{
   std::cout<<"LOLOLOL IMPLEMENT THIS YOU FOOL"<<std::endl;
}

ClassImp(TAGSpillSequencerData);
TAGSpillSequencerData::TAGSpillSequencerData(): TSpillSequencerData()
{
}
TAGSpillSequencerData::~TAGSpillSequencerData()
{
}
TAGSpillSequencerData::TAGSpillSequencerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d)
{
   fSequenceNum= d->StartDumpMarker->fSequencerID;
   fDumpID     = d->dumpID;
   fSeqName    = SeqNames.at(fSequenceNum);
   fStartState = d->StartDumpMarker->fonState;
   fStopState  = d->StopDumpMarker->fonState;
}


TAGSpillSequencerData::TAGSpillSequencerData(TAGSpillSequencerData* a)
{
   fSequenceNum  =a->fSequenceNum;
   fDumpID       =a->fDumpID;
   fSeqName      =a->fSeqName;
   fStartState   =a->fStartState;
   fStopState    =a->fStopState;
}

ClassImp(TAGSpill)
TAGSpill::TAGSpill()
{

}
TAGSpill::TAGSpill(int runno, const char* format, ...): TSpill(runno)
{
   SeqData    =NULL;
   ScalerData =NULL;
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}

TAGSpill::TAGSpill(int runno, DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d ): TSpill(runno,d->StartDumpMarker->Description.c_str())
{
   if (d->StartDumpMarker && d->StopDumpMarker) IsDumpType=true;
   ScalerData = new TAGSpillScalerData(d);
   SeqData = new TAGSpillSequencerData(d);
   //Print();
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
#endif