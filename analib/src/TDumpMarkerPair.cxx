#include "TDumpMarkerPair.h"


template<typename VertexType, typename ScalerType, int NumScalers>
TDumpMarkerPair<VertexType,ScalerType,NumScalers>::TDumpMarkerPair()
{
   fDumpID=-1;
   fStartDumpMarker=NULL;
   fStopDumpMarker=NULL;
   for (int i=0; i<NumScalers; i++)
   {
      fIntegratedScalerCounts.emplace_back(i);
      fScalerFilled.push_back(NO_EQUIPMENT);
   }
   fVertexFilled=NO_EQUIPMENT;
   fIsPaired=false;
}

template<typename VertexType, typename ScalerType, int NumScalers>
TDumpMarkerPair<VertexType,ScalerType,NumScalers>::TDumpMarkerPair(const TDumpMarker& startDump): TDumpMarkerPair()
{
   AddStartDump(startDump);
}

template<typename VertexType, typename ScalerType, int NumScalers>
TDumpMarkerPair<VertexType,ScalerType,NumScalers>::~TDumpMarkerPair()
{
   clear();
   //Do I really have to own the dump markers? Possibily I could avoid making copies... 
   delete fStartDumpMarker;
   fStartDumpMarker = NULL;
   delete fStopDumpMarker;
   fStopDumpMarker = NULL;
}

template<typename VertexType, typename ScalerType, int NumScalers>
bool TDumpMarkerPair<VertexType,ScalerType,NumScalers>::Ready()
{
   if (!fIsPaired) return false;
   if (!fStartDumpMarker) return false;
   if (!fStopDumpMarker) return false;
   if (fStartDumpMarker->fRunTime<0) return false;
   if (fStopDumpMarker->fRunTime<0) return false;
   //FIX ME AGAIN FOR A2... we need refactor!!!
   return true;
   for ( size_t i=0; i<fScalerFilled.size(); i++)
      if (fScalerFilled[i]==NOT_FILLED) return false;
   if (fVertexFilled==NOT_FILLED) return false;
   return true;
}

template<typename VertexType, typename ScalerType, int NumScalers>
void TDumpMarkerPair<VertexType,ScalerType,NumScalers>::Print()
{
   std::cout<<"DumpID: "<<fDumpID<<std::endl;
   std::cout<<"IsPaired?:"<<fIsPaired<<std::endl;
   if (fStartDumpMarker)
   {
      std::cout<<"StartTime:"<<fStartDumpMarker->fRunTime<<std::endl;
      fStartDumpMarker->Print();
   }
   if (fStopDumpMarker)
   {
      std::cout<<"StopTime: "<<fStopDumpMarker->fRunTime<<std::endl;
      fStopDumpMarker->Print();
   }
   std::cout<<"Number of fStates:"<<fStates.size()<<std::endl;
}

//Maybe we should add MIDAS speaker announcements inside this 
//function, most of these failure modes are critical
template<typename VertexType, typename ScalerType, int NumScalers>
std::vector<std::string> TDumpMarkerPair<VertexType,ScalerType,NumScalers>::check(int DumpStart, int DumpStop)
{
    std::vector<std::string> errors;
    char buf[200];
    //Check if dumps are paired
    if (!fStartDumpMarker)
    {
       sprintf(buf,"Dump %d is a pair with no start dump... this should never happen",fDumpID);
       errors.push_back(buf);
    }
    else
    {
       if (fStartDumpMarker->fDescription.front() != '"')
       {
          sprintf(buf,"Start dump %s doesn't start with a \"... ",fStartDumpMarker->fDescription.c_str());
          errors.push_back(buf);
       }
       if (fStartDumpMarker->fDescription.back() != '"')
       {
          sprintf(buf,"Start dump %s doesn't end with a \"... look for whitespace?",fStartDumpMarker->fDescription.c_str());
          errors.push_back(buf);
       }
    }
    if (!fStopDumpMarker)
    {
       if (!fStartDumpMarker)
          sprintf(buf,"Dump %d is a pair with no start or stop dump... this should never happen",fDumpID);
       else
          sprintf(buf,"Dump %s is a pair with no stop dump... ",fStartDumpMarker->fDescription.c_str());
       errors.push_back(buf);
    }
    else
    {
       if (fStopDumpMarker->fDescription.front() != '"')
       {
          sprintf(buf,"Stop dump %s doesn't start with a \"... ",fStopDumpMarker->fDescription.c_str());
          errors.push_back(buf);
       }
       if (fStopDumpMarker->fDescription.back() != '"')
       {
          sprintf(buf,"Stop dump %s doesn't end with a \"... look for whitespace?",fStopDumpMarker->fDescription.c_str());
          errors.push_back(buf);
       }
    }
    //We need fStates to investigate the Digital output of the 
    //sequencer (to the SIS/ Chronoboxes)
    if (!fStates.size())
    {
       sprintf(buf,"Dump %d has no fStates... this should never happen",fDumpID);
       errors.push_back(buf);
    }
    else //Check that the SIS is triggered
    {
       const TSequencerState& SISStartMarker = fStates.front();
       if (!SISStartMarker.GetDigitalOut()->Channels[DumpStart])
       {
          char buf[100];
          sprintf(
             buf,
             "Warning: Start dump %s (%s) has no SIS / ChronoBox trigger yet!",
             fStartDumpMarker->fDescription.c_str(),
             GetSequencerName(fStartDumpMarker->fSequencerID).c_str()
          );
          errors.push_back(buf);
       }
       const TSequencerState& SISStopMarker = fStates.back();
       if (!SISStopMarker.GetDigitalOut()->Channels[DumpStop])
       {
          char buf[100];
          if (fStopDumpMarker)
          {
             sprintf(
                buf,
                "Warning: Stop dump %s (%s) has no SIS / ChronoBox trigger yet!",
                fStopDumpMarker->fDescription.c_str(),
                GetSequencerName(fStopDumpMarker->fSequencerID).c_str()
             );
          }
          else
          {
             if (fStartDumpMarker)
                 sprintf(
                    buf,
                    "Warning: Start dump %s (%s) has no stop dump yet!",
                    fStartDumpMarker->fDescription.c_str(),
                    GetSequencerName(fStartDumpMarker->fSequencerID).c_str()
                 );
             else
                 sprintf(buf,"Warning: No start dump, no stop dump...!");
          }
          errors.push_back(buf);
       }
    }
    return errors;
}

template<typename VertexType, typename ScalerType, int NumScalers>
void TDumpMarkerPair<VertexType,ScalerType,NumScalers>::clear()
{
   fStates.clear();
   fIntegratedScalerCounts.clear();
}

template<typename VertexType, typename ScalerType, int NumScalers>
void TDumpMarkerPair<VertexType,ScalerType,NumScalers>::AddStartDump(const TDumpMarker& d)
{
   fStartDumpMarker = new TDumpMarker(d);
}

template<typename VertexType, typename ScalerType, int NumScalers>
bool TDumpMarkerPair<VertexType,ScalerType,NumScalers>::AddStopDump(const TDumpMarker& d)
{
   //std::cout<<d->fDescription <<"\t==\t"<<fStartDumpMarker->fDescription<<"\t?"<<std::endl;
   if (strcmp(d.fDescription.c_str(),fStartDumpMarker->fDescription.c_str())==0)
   {
      fStopDumpMarker = new TDumpMarker(d);
      fIsPaired = true;
      return true;
   }
   else
   {
      //Descriptions did not match... reject
      fStopDumpMarker = NULL;
      return false;
   }
}

template<typename VertexType, typename ScalerType, int NumScalers>
int TDumpMarkerPair<VertexType,ScalerType,NumScalers>::AddState(const TSequencerState& s)
{
   //State before dump starts... do not add
   //std::cout<<fStartDumpMarker->fonState << " <= "<<s->GetState() <<" <= "<< StopDumpMarker->fonState <<std::endl;
   if (fStartDumpMarker)
   {
      if (s.GetState()<fStartDumpMarker->fonState && fStates.size()==0)
         return -1;
   }
   else
   {
      return 0;
   }
   if (fStopDumpMarker)
   {
      //State after dump stops... do not add
      if (s.GetState()>fStopDumpMarker->fonState)
         return 1;
      //Create copy of state put into vector
      fStates.emplace_back(s);
      //std::cout<<"Added state to "<<fStartDumpMarker->fDescription<<std::endl;
      return 0;
   }
   else
   {
      if (fStates.size()%1000==0)
         std::cout << "Warning, adding fStates to unpaired dump(" << 
            fStartDumpMarker->fDescription.c_str() << 
            ") either a bug or a dump that spans multiple sequences (I will ignore the next 1000 fStates in this dump)"<<std::endl;
      fStates.emplace_back(s);
      
   }
   return 0;
}

template<typename VertexType, typename ScalerType, int NumScalers>
int TDumpMarkerPair<VertexType,ScalerType,NumScalers>::AddScalerEvent(const ScalerType& s)
{
   //For ALPHA 2, ScalerModule is SIS channel 1 or 2 (total = 2)
   //For ALPHA g, ScalerModule is the Board* NChannels+ Channel (total = 120)
   const int ScalerModule = s.GetScalerModule();
   if (ScalerModule < 0)
   {
      std::cout<<"JOE, this is an invalid module number!: " << ScalerModule<<std::endl;
      return 0;
   }
   if ( int( fIntegratedScalerCounts.size() ) < ScalerModule)
   {
      std::cout<<"JOE, this is an invalid module number!: " << ScalerModule<<std::endl;
      return 0;
   }
   
   //std::cout<<"MODULE:"<<SISModule<<std::endl;
   //Record that there are SIS events...
//JOE! LUKAS! PUT THIS BACK AFTER REFACTOR
//#ifdef _TSISEvent_
//     if (std::is_same<ScalerType,TSISEvent>::value)
//     {
//        //This breaks ALPHAg chronoflow where channels have no counts...
//        fScalerFilled[ScalerModule]=NOT_FILLED;
//     }
//#endif
   const double t = s.GetRunTime();
   if (fStartDumpMarker)
   {
      if (fStartDumpMarker->fRunTime < 0)
         return -2;
      if (t < fStartDumpMarker->fRunTime)
         return -1;
   }
   if (fStopDumpMarker)
      if (fStopDumpMarker->fRunTime > 0)
      {
         //Dump is definitely filled, return that we can break parent loop
         if (t > fStopDumpMarker->fRunTime + 2)
         {
            fScalerFilled.at(ScalerModule) = FILLED;
            return 1;
         }
         //Event is after dump, but this dump might not yet be filled (TChronoEvents aren't in exact order)
         if (t > fStopDumpMarker->fRunTime  )
            return 0;
      }  
   //s.Print();
   fIntegratedScalerCounts.at(ScalerModule) += s;
   return 0;
}

template<typename VertexType, typename ScalerType, int NumScalers>
int TDumpMarkerPair<VertexType,ScalerType,NumScalers>::AddSVDEvent(const VertexType& s)
{
   fVertexFilled=NOT_FILLED;
   const double t = s.GetTimeOfEvent();
   if (fStartDumpMarker)
   {
      if (fStartDumpMarker->fRunTime < 0)
         return -2;
      if (t < fStartDumpMarker->fRunTime)
         return -1;
   }
   if (fStopDumpMarker)
      if (fStopDumpMarker->fRunTime > 0)
         if (t > fStopDumpMarker->fRunTime)
         {
            fVertexFilled = FILLED;
            return 1;
         }
   fIntegratedVertexCounts.AddEvent(s);
   return 0;
}


#if BUILD_AG
#include "TStoreEvent.hh"
#include "TChronoBoardCounter.h"
template class TDumpMarkerPair<TStoreEvent, TChronoBoardCounter, CHRONO_N_BOARDS>;
#endif

#if BUILD_A2
#include "TSISEvent.h"
#include "TSVD_QOD.h"
template class TDumpMarkerPair<TSVD_QOD,TSISEvent, NUM_SIS_MODULES>;
#endif