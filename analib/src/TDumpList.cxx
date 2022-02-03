#include "TDumpList.h"

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
TDumpList<SpillType, VertexType, ScalerType, NumScalers>::TDumpList()
{
   fRunNo=-1;
   fSeqcount=-1;
   fSequencerID=-1;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
TDumpMarkerPair<VertexType,ScalerType,NumScalers>* TDumpList<SpillType, VertexType, ScalerType, NumScalers>::GetPairOfStart(TDumpMarker* d)
{
   for ( auto &pair : fDumps )
   {
      if (!pair) continue;
      if (pair->fStartDumpMarker==d)
      return pair;
   }
   return NULL;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
TDumpMarkerPair<VertexType,ScalerType,NumScalers>* TDumpList<SpillType, VertexType, ScalerType, NumScalers>::GetPairOfStop(TDumpMarker* d)
{
   for ( auto &pair : fDumps )
   {
      if (!pair) continue;
      if (pair->fStopDumpMarker==d)
      return pair;
   }
   return NULL;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
SpillType* TDumpList<SpillType, VertexType, ScalerType, NumScalers>::GetError()
{
   if (!fErrorQueue.size()) return NULL;
   SpillType* s=fErrorQueue.front();
   fErrorQueue.pop_front();
   return s;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
bool TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddStartDump(const TDumpMarker& d)
{
   //Construct a new dump at the back of fDumps
   fDumps.push_back(new TDumpMarkerPair<VertexType,ScalerType,NumScalers>(d));
   fOrderedStarts.push_back(fDumps.back()->fStartDumpMarker);
   //For now no error checking...
   return true;
}

//Return true if dump is paired
template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
bool TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddStopDump(const TDumpMarker& d)
{
   for (size_t i=0; i<fDumps.size(); i++)
   {
      if (!fDumps.at(i)) continue;
      //Find incomplete fDumps, skip paired ones
      if (fDumps.at(i)->fIsPaired) continue;
      //Add stop dump (if the dump descriptions match)
      if (fDumps.at(i)->AddStopDump(d))
      {
         fOrderedStops.push_back(fDumps.at(i)->fStopDumpMarker);
         return true;
      }
   }
   //No pair found!
   std::cout<<"ERROR! I did not pair a dump!"<<std::endl;
   fErrorQueue.push_back(new SpillType(fRunNo,
                                       d.fMidasTime,
                                       "ERROR! Stop dump:%s did not find a pair",
                                       d.fDescription.c_str()));
   return false;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddSequencerStartTime(const TDumpMarker& d)
{
   //If I have a invalid midas timestamp, do nothing
   if (!d.fMidasTime) return;
   if (d.fDumpType == TDumpMarker::kDumpTypes::Info) return;
   if (fSequenceStartTimes.size())
   {
      uint32_t last_time = fSequenceStartTimes.back().second;
      //And I am larger than the previous start (I came afterwood
      if (last_time < d.fMidasTime)
      {
         fSequenceStartTimes.push_back({d.fSequenceCount,d.fMidasTime});
         return;
      }
      if (last_time > d.fMidasTime)
      {
         fErrorQueue.push_back(new SpillType(fRunNo,
                                             d.fMidasTime,
                                             "Sequence started before the last one? This should never happen"));
         return;
      }
   }
   else 
   {
      fSequenceStartTimes.push_back({d.fSequenceCount,d.fMidasTime});
      return;
   }
   return;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
bool TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddDump(const TDumpMarker& d)
{
   AddSequencerStartTime(d);
   switch(d.fDumpType)
   {
      case TDumpMarker::kDumpTypes::Info:
         std::cout<<"Info:"; d.Print();
         return true;
      case TDumpMarker::kDumpTypes::Start:
         return AddStartDump(d);
      case TDumpMarker::kDumpTypes::Stop:
         return AddStopDump(d);
   }
   fErrorQueue.push_back(new SpillType(fRunNo,
                                       d.fMidasTime,
                                       "Attempted to add dump maker that was neither start not stop..."));
   return false;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddStates(std::vector<TSequencerState> s)
{
   for ( auto &pair : fDumps )
   {
      //pair->Print();
      for ( const auto& state: s )
      {
         //state->Print();
         if (!pair) continue;
         //If the state is after the dump ends.. break this loop
         if (pair->AddState(state)>0) break;
      }
   }
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::RemoveAndCleanAbortedSequence(int SequenceCount)
{
   assert(fSequenceStartTimes.front().first< SequenceCount);
   //     for (int i=0; i<fSequenceStartTimes.size(); i++)
   while (1)
if (fSequenceStartTimes.front().first>= SequenceCount) return;
int badseq=fSequenceStartTimes.front().first;
   fSequenceStartTimes.pop_front();
   for ( auto &pair : fDumps )
   {
      if (!pair) continue;
      if (!pair->fStartDumpMarker) continue;
      if (pair->fStartDumpMarker->fSequenceCount == badseq )
      {
         fErrorQueue.push_back(new SpillType(fRunNo,
                                             pair->fStartDumpMarker->fMidasTime,
                                             "Delete pair %s", 
                                             pair->fStartDumpMarker->fDescription.c_str()));
  //            delete pair;
         pair=NULL;
      }
   }
   for ( auto &start : fOrderedStarts )
   {
      if (!start) continue;
      if (start->fSequenceCount == badseq )
      {
         fErrorQueue.push_back(new SpillType(fRunNo,
                                             start->fMidasTime,
                                             "Delete start %s",
                                             start->fDescription.c_str()));
  //            delete start;
         start=NULL;
      }
   }
   for (auto &stop : fOrderedStops )
   {
      if (!stop) continue;
      if (stop->fSequenceCount == badseq )
      {
         fErrorQueue.push_back(new SpillType(fRunNo,
                                             stop->fMidasTime,
                                             "Delete stop %s",
                                             stop->fDescription.c_str()));
  //            delete stop;
         stop=NULL;
      }
   }
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddStartTime(uint32_t midas_time, double t)
{
   if (!fOrderedStarts.size())
   {
      fErrorQueue.push_back(new SpillType(fRunNo,
                                          midas_time,
                                          "Error, start dump time stamp given with no dump, did the sequencer start before the run?"));
      AddStartDump(TDumpMarker("NO NAME DUMP",TDumpMarker::kDumpTypes::Start));
      //return;
   }
   // Clear up pointers at front set to NULL... 
   // we dont care about them any more, they've been nuked elsewhere,
   // and when nuked, and error was logged
   if (!fOrderedStarts.front())
   {
      fOrderedStarts.pop_front();
      return AddStartTime(midas_time,t);
   }
   // Find sequence start time stamp that closes matches this start 
   // dump... note, negative values will be ignored as are 
   // sequences in the future
   
   int best_i = fSequenceStartTimes.front().first;
   int best_diff = fSequenceStartTimes.front().second;
   for (size_t i = 0; i < fSequenceStartTimes.size(); i++)
   {
      int dt = midas_time - fSequenceStartTimes.at(i).second;
      if (dt<=2) continue;
      if (dt<best_diff)
      {
         best_diff=dt;
         best_i=fSequenceStartTimes.at(i).first;
      }
      //std::cout<<"iiii  "<< i<<"\t"<<midas_time<<" - "<<fSequenceStartTimes.at(i).second<<" = "<< dt <<std::endl;
   }
   if (best_i!=fSequenceStartTimes.front().first)
   {
      RemoveAndCleanAbortedSequence(best_i);
      return AddStartTime(midas_time,t);
   }
   if (fOrderedStarts.front()->fMidasTime > midas_time)
   {
      fErrorQueue.push_back(new SpillType(fRunNo,
                                          midas_time,
                                          "Error, bad unix time of dump... Aborted sequence detected? Skipping dump"));
      fOrderedStarts.front()->Print();
      std::cout<<fOrderedStarts.front()->fMidasTime <<" > "<< midas_time <<std::endl;
      fOrderedStarts.pop_front();
      return AddStartTime(midas_time,t);
   }
   fOrderedStarts.front()->fRunTime = t;
   fOrderedStarts.pop_front();
   return;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddStopTime(uint32_t midas_time, double t)
{
   if (!fOrderedStops.size())
   {
      fErrorQueue.push_back(new SpillType(fRunNo,
                                          midas_time,
                                          "Error, stop dump time stamp given with no dump, did the sequencer start before the run?"));
      AddStopDump(TDumpMarker("NO NAME DUMP",TDumpMarker::kDumpTypes::Stop));
      return;
   }
   if (!fOrderedStops.front())
   {
       fOrderedStops.pop_front();
       return AddStopTime(midas_time,t);
   }
   // Find sequence start time stamp that closes matches this start 
   // dump... note, negative values will be ignored as are 
   // sequences in the future
   
   int best_i=fSequenceStartTimes.front().first;
   int best_diff=99999;
   for (size_t i=0; i<fSequenceStartTimes.size(); i++)
   {
      int dt=midas_time-fSequenceStartTimes.at(i).second;
      if (dt<=2) continue;
      if (dt<best_diff)
      {
         best_diff=dt;
         best_i=fSequenceStartTimes.at(i).first;
      }
      //std::cout<<"iiii  "<< i<<"\t"<<midas_time<<" - "<<fSequenceStartTimes.at(i).second<<" = "<< dt <<std::endl;
   }
   if (best_i!=fSequenceStartTimes.front().first)
   {
      RemoveAndCleanAbortedSequence(best_i);
      return AddStopTime(midas_time,t);
   }
   if (fOrderedStops.front()->fMidasTime > midas_time && fOrderedStops.front()->fMidasTime != 0)
   {
      fErrorQueue.push_back(
         new SpillType(
            fRunNo,
            midas_time,
            "Error, bad unix time of dump... Aborted sequence detected? Skipping dump"
         )
      );
      fOrderedStops.front()->Print();
      std::cout<<fOrderedStops.front()->fMidasTime <<" > "<< midas_time <<std::endl;
      fOrderedStops.pop_front();
      return AddStopTime(midas_time,t);
   }
   TDumpMarkerPair<VertexType,ScalerType,NumScalers>* pair = GetPairOfStop(fOrderedStops.front());
   if (pair)
   {
      if (pair->fStartDumpMarker)
      {
         if (pair->fStartDumpMarker->fRunTime > t)
         {
            fErrorQueue.push_back(
               new SpillType(fRunNo,
                  pair->fStartDumpMarker->fMidasTime,
                  "XXXX Error... stop dump (%s) from %s happened before start?",
                  fOrderedStops.front()->fDescription.c_str(),
                  GetSequencerName(fOrderedStops.front()->fSequencerID).c_str()
               )
            );
            int BadSeq = -1;
            if (fOrderedStarts.size())
            {
               BadSeq = fOrderedStarts.front()->fSequenceCount;
               std::cout<<"Deleteing bad sequence:"<<BadSeq;
            }
            for (size_t i=0; i<fOrderedStarts.size(); i++)
            {
               if (fOrderedStarts.at(i) && BadSeq >= 0)
               if (fOrderedStarts.at(i)->fSequenceCount == BadSeq)
               {
                  std::cout<<"REMOVING START:"<<fOrderedStarts.at(i)->fDescription.c_str()<<std::endl;
                  TDumpMarkerPair<VertexType,ScalerType,NumScalers>* bad_pair=GetPairOfStart(fOrderedStarts.at(i));
                  //JOE DO SOME PROPER DELETING!!!
                  bad_pair->fStartDumpMarker=NULL;
                  bad_pair->fStopDumpMarker=NULL;
                  fOrderedStarts.at(i)=NULL;
               }
            }
            for (size_t i=0; i<fOrderedStops.size(); i++)
            {
               if (fOrderedStops.at(i))
               if (fOrderedStops.at(i)->fSequenceCount == BadSeq)
               {
                  std::cout<<"REMOVING STOP:"<<fOrderedStops.at(i)->fDescription.c_str()<<std::endl;
                  TDumpMarkerPair<VertexType,ScalerType,NumScalers>* bad_pair=GetPairOfStop(fOrderedStops.at(i));
                  //JOE DO SOME PROPER DELETING!!!
                  bad_pair->fStartDumpMarker=NULL;
                  bad_pair->fStopDumpMarker=NULL;
                  fOrderedStops.at(i)=NULL;
               }
            }
            return;
         }
         if (pair->fStartDumpMarker->fRunTime<0)
         {
            if (pair->fStopDumpMarker)
               fErrorQueue.push_back(
                  new SpillType(fRunNo,
                     pair->fStartDumpMarker->fMidasTime,
                     "XXXX %s (%s) has no start time!... deleting %s start dump",
                     pair->fStartDumpMarker->fDescription.c_str(),
                     GetSequencerName(pair->fStartDumpMarker->fSequencerID).c_str(),
                     pair->fStopDumpMarker->fDescription.c_str()
                  )
               );
            else
               fErrorQueue.push_back(
                  new SpillType(fRunNo,
                     pair->fStartDumpMarker->fMidasTime,
                     "XXXX %s (%s) has no start time!... It also has not stop dump marker",
                     pair->fStartDumpMarker->fDescription.c_str(),
                     GetSequencerName(pair->fStartDumpMarker->fSequencerID).c_str()
                  )
               );
            //fOrderedStarts.pop_front();
            //fOrderedStops.pop_front();
            //return AddStopTime(midas_time,t);
            return;
         }
      }
   }
   fOrderedStops.front()->fRunTime=t;
   fOrderedStops.pop_front();
   //pair->Print();
   return;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddAndSortScalerEvents(std::vector<std::vector<ScalerType>> events)
{
   const size_t scaler_channels = events.size();
   std::vector<int> front_event(scaler_channels,0);
   std::vector<int> back_event(scaler_channels,0);
   std::vector<std::vector<double>> runtime;
   for (int i = 0; i < scaler_channels; i++)
   {
      if (events.at(i).empty())
         continue;
      runtime.emplace_back(std::vector<double>());
      back_event.at(i) = events.at(i).size() - 1;
      assert(back_event[i] == back_event[0]);
      for ( const ScalerType &s : events[i] )
      {
         runtime[i].emplace_back(s.GetRunTime());
      }
   }
   
   for ( auto &pair : fDumps )
   {
      
      if (!pair) continue;
      while (true)
      //for ( const ScalerType &s : events )
      //for ( ; *std::min_element(front_event.begin(), front_event.end()) < back_event[0]; )
      {
         ScalerType* s;
         double tmin = 1E99;
         int next_channel = -1;
         for (int i = 0; i < scaler_channels; i++)
         {
            if (front_event[i] == back_event[i])
               continue;
            if (runtime[i][front_event[i]] < tmin)
            {
               tmin = runtime[i][front_event[i]];
               next_channel = i;
            }
         }
         if (next_channel == -1)
            break;
         //std::cout<<next_channel<<"\t"<<events[next_channel][front_event[next_channel]].GetRunTime() <<std::endl;
         pair->AddScalerEvent(events[next_channel][front_event[next_channel]]);
         front_event[next_channel]++;
         //if (pair->AddScalerEvent(s)>0) break;
        
      }
   }
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddScalerEvents(std::vector<ScalerType> events)
{
   for ( auto &pair : fDumps )
   {
      if (!pair) continue;
      for ( const ScalerType &s : events )
      {
         pair->AddScalerEvent(s);
         //if (pair->AddScalerEvent(s)>0) break;
      }
   }
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddScalerEvents(std::vector<ScalerType*> events)
{
   for ( auto &pair : fDumps )
   {
      if (!pair) continue;
      for ( const ScalerType* s : events )
      {
         pair->AddScalerEvent(*s);
         //if (pair->AddScalerEvent(s)>0) break;
      }
   }
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::AddSVDEvents(std::vector<VertexType*>* events)
{
   for ( auto &pair : fDumps )
   {
      if (!pair) continue;
      for ( auto &s : *events )
      {
         if (pair->AddSVDEvent(*s)>0) break;
      }
   }
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::check(TSequencerDriver* d)
{
   //std::cout<<"SIS start dump channel:"<<d->DigitalMap->ChannelDescriptionMap["Dump start"]<<std::endl;
   //std::cout<<"SIS stop dump channel:"<<d->DigitalMap->ChannelDescriptionMap["Dump end"]<<std::endl;
   int start=d->DigitalMap->ChannelDescriptionMap["Dump start"];
   int stop=d->DigitalMap->ChannelDescriptionMap["Dump end"];
   for(auto pair: fDumps)
   {
      if (!pair) continue;
      std::vector<std::string> err=pair->check(start,stop);
      //collect errors and create SpillTypes... 
      if (err.size())
         for(auto error: err)
            fErrorQueue.push_back(new SpillType(fRunNo,
                                               pair->fStartDumpMarker->fMidasTime,
                                               error.c_str()));
   }
   return;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::Print()
{
   std::cout<<"SequencerCount:"<<fSeqcount<<std::endl;
   for(auto pair: fDumps)
   {
      if (!pair) continue;
      pair->Print();
   }
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
int TDumpList<SpillType, VertexType, ScalerType, NumScalers>::countIncomplete()
{
   int incomplete=0;
   for (size_t i=0;i<fDumps.size();i++)
   {
     if (!fDumps.at(i)) continue;
     if (fDumps.at(i)->fIsPaired) continue;
     if (fDumps.at(i)->fIsFinished) continue;
     incomplete++;
   }
   return incomplete;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
std::vector<SpillType*> TDumpList<SpillType, VertexType, ScalerType, NumScalers>::flushComplete()
{
   std::vector<SpillType*> complete;
   for (size_t i=0;i<fDumps.size();i++)
   {
     TDumpMarkerPair<VertexType,ScalerType,NumScalers>* pair=fDumps.at(i);
     if (!pair) continue;
     if (!pair->Ready()) continue;
     if (pair->fIsFinished) continue;
     pair->fIsFinished=true;
     SpillType* spill = new SpillType(fRunNo,pair);
     complete.push_back(spill);
     //Item flushed... delete it
     delete pair;
     fDumps.at(i) = NULL;
   }
   if (fOrderedStarts.size()==0 && 
       fOrderedStops.size()==0 &&
       complete.size())
   {
      complete.push_back(
         new SpillType(fRunNo,
            0,
            "%s Sequencer: Sequence %d fDumps completed",
            SEQ_NAMES.at(fSequencerID).c_str(), 
            fSeqcount
         )
      );
   }
   return complete;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::clear()
{
   for (size_t i=0;i<fDumps.size(); i++)
   {
      if (fDumps.at(i))
         fDumps.at(i)->clear();
   }
   fDumps.clear();
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::setup(uint32_t unixtime)
{
   fSeqcount++;
   fErrorQueue.push_back(
      new SpillType(fRunNo,
         unixtime,
         "%s Sequencer: Sequence %d queued",
         SEQ_NAMES.at(fSequencerID).c_str(),
         fSeqcount
      )
   );
   return;
}

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>
void TDumpList<SpillType, VertexType, ScalerType, NumScalers>::finish()
{
   for (size_t i=0; i<fDumps.size(); i++)
   {
      if (!fDumps.front())
      {
         fDumps.pop_front();
         continue;
      }
      if (fDumps.front()->fIsFinished)
      {
         delete fDumps.front();
         fDumps.pop_front();
      }
      else
      {
         break;
      }
   }
   if (fDumps.size())
   {
      //Extra warning to show the dummps were not empty and we need to run tests
      fErrorQueue.push_back(new SpillType(fRunNo,0,"ERROR DUMPS POTENTIALLY THROWN AWAY! Possible aborted sequence detected"));
      for (size_t i=0; i<fDumps.size(); i++)
      {
         if (!fDumps.at(i)) continue;
         //If we have a start dump in this pair (we should always have one)
         if (fDumps.at(i)->fStartDumpMarker)
         {
            //If the start of the dump happened (as marker from SIS)
            if (fDumps.at(i)->fStartDumpMarker->fRunTime>0)
            {
               //If start of dump happend, but there is no stop dump
               if (!fDumps.at(i)->fStopDumpMarker)
               {
                  fErrorQueue.push_back(new SpillType(fRunNo,
                                                      fDumps.at(i)->fStartDumpMarker->fMidasTime,
                                                      "Warning, start dump (%s) being carried from the previous sequence, not paired yet... OK",
                                                      fDumps.at(i)->fStartDumpMarker->fDescription.c_str()));
               }
               //Else if there is a valid start dump AND stop dump
               else
               {
                  //Check stop dump for valid time... if invalid, sequence was aborted (or buggy)
                  if(fDumps.at(i)->fStopDumpMarker->fRunTime<0)
                  {
                     fErrorQueue.push_back(new SpillType(fRunNo,
                                                         fDumps.at(i)->fStartDumpMarker->fMidasTime,
                                                         "ERROR DUMPS THROWN AWAY! Aborted sequence detected"));
                  }
                  //Else is ok... throw a warning anyway
                  else
                  {
                     fErrorQueue.push_back( new SpillType(fRunNo,
                                                         fDumps.at(i)->fStartDumpMarker->fMidasTime,
                                                         "Warning, dump pair good (%s and %s) in memory, but should have been cleared... this should never happen",
                                                          fDumps.at(i)->fStartDumpMarker->fDescription.c_str(),
                                                          fDumps.at(i)->fStopDumpMarker->fDescription.c_str()));
                  }
               }
            }
         }
      }
   }
   clear();
   return;
}

#if BUILD_AG
#include "TStoreEvent.hh"
#include "TChronoBoardCounter.h"
#include "TAGSpill.h"
template class TDumpList<TAGSpill,TStoreEvent, TChronoBoardCounter, CHRONO_N_BOARDS>;
#endif

#if BUILD_A2
#include "TSISEvent.h"
#include "TSVD_QOD.h"
#include "TA2Spill.h"
template class TDumpList<TA2Spill,TSVD_QOD,TSISEvent, NUM_SIS_MODULES>;
#endif