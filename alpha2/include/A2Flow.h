//
// A2Flow.h
//
// manalyzer flow objects for ALPHA-g events
// JTK McKenna
//



#ifndef A2Flow_H
#define A2Flow_H
#include "UnpackVF48.h"
#include "SiMod.h"

#include <iostream>


#include <array>

template<typename T>
class PointerRecycler
{
   private:
      const std::string fRecyclerName;
      const int fMaxBufferedEvents;
      std::deque<T*> fRecycleBin;
      int fNEventsRecycled;
      int fNEventsCreatedNew;
      int fNEventsThrownAway;
      int fNEventsQueued;
   private:
      T* GrabObject()
      {
         T* newObject = fRecycleBin.front();
         fRecycleBin.pop_front();
         newObject->Reset();
         return newObject;
      }
   public:
      PointerRecycler(const int buffer_events, const char* nick_name):
         fRecyclerName(nick_name),
         fMaxBufferedEvents(buffer_events)
      {
         fNEventsRecycled = 0;
         fNEventsCreatedNew = 0;
         fNEventsThrownAway = 0;
         fNEventsQueued = 0;
      }
      ~PointerRecycler()
      {
          std::cout << fRecyclerName.c_str() << " PointerRecycler\n";
          std::cout << "\t" << fNEventsRecycled << " events recycled\n";
          std::cout << "\t" << fNEventsCreatedNew << " events created\n";
          std::cout << "\t" << fNEventsThrownAway << " events thrown away (queue full)\n";
          std::cout << "\t" << fNEventsQueued << " events queued\n";
          for (T* p: fRecycleBin)
             delete p;
          fRecycleBin.clear();
      }
      T* NewObject()
      {
         if (fRecycleBin.size())
         {
            fNEventsRecycled++;
            return GrabObject();
         }
         else
         {
            fNEventsCreatedNew++;
            return new T();
         }
      }
      void RecycleObject(T* data)
      {
         if (fRecycleBin.size() < fMaxBufferedEvents)
         {
            fNEventsQueued++;
            fRecycleBin.push_back(data);
         }
         else
         {
            fNEventsThrownAway++;
            delete data;
         }
      }
};

class VectorRecyclerTableWriter
{
   public:
   VectorRecyclerTableWriter() {
      std::cout <<"PLOOP TABLE"<<std::endl;
   };
   ~VectorRecyclerTableWriter()
   {
      std::cout <<"MY TABLE"<<std::endl;
   };
};

#define ENABLE_VECTOR_RECYCLING 1

template<typename T>
class VectorRecycler
{
   private:
      const std::string fRecyclerName;
      const int fMaxBufferedEvents;
      std::deque<std::vector<T>> fRecycleBin;
      std::mutex fMutex;

      int fNEventsRecycled;
      int fNEventsCreatedNew;
      int fNEventsThrownAway;
      int fNEventsQueued;

      int fMemInUse;
      int fMaxMemUse;

   private:
      std::vector<T> GrabVector()
      {
         while (fRecycleBin.front().capacity() == 0)
         {
            fRecycleBin.pop_front();
            if (fRecycleBin.empty())
               break;
         }
         if (fRecycleBin.size())
         {
            fNEventsRecycled++;
            std::vector<T> data = std::move(fRecycleBin.front());
            fMemInUse -= data.capacity();
            return data;
         }
         else
         {
            fNEventsCreatedNew++;
            return std::vector<T>();
         }
      }
   public:
      VectorRecycler(const int buffer_events, const char* nick_name):
         fRecyclerName(nick_name),
         fMaxBufferedEvents(buffer_events)
      {

      }
      ~VectorRecycler()
      {
#if ENABLE_VECTOR_RECYCLING
          std::cout << fRecyclerName.c_str() << " Container Recycler\n";
          std::cout << "\t" << fNEventsRecycled << " events recycled\n";
          std::cout << "\t" << fNEventsCreatedNew << " events created\n";
          std::cout << "\t" << fNEventsThrownAway << " events thrown away (queue full)\n";
          std::cout << "\t" << fNEventsQueued << " events queued\n";
          if (fMemInUse > 1024 * 1024)
          {
             std::cout << "\t" << fMemInUse / 1024 / 1024 << "MB mem in use\n";
          } else if (fMemInUse > 1024 ) {
             std::cout << "\t" << fMemInUse / 1024 << "kB mem in use\n";
          } else {
             std::cout << "\t" << fMemInUse << "B mem in use\n";
          }
          //std::cout << "\t" << fMemInUse << " mem in use ("<<fMaxMemUse<<")\n";
          fRecycleBin.clear();
#endif
      }
      std::vector<T> NewVector()
      {
#if ENABLE_VECTOR_RECYCLING
         std::lock_guard<std::mutex> guard(fMutex);
         if (fRecycleBin.size())
            return GrabVector();
         else
#endif
            return std::vector<T>();
      }
      void RecycleVector(std::vector<T> data)
      {
#if ENABLE_VECTOR_RECYCLING
         std::lock_guard<std::mutex> guard(fMutex);
         // Dont recycle empty containers
         if (!data.capacity())
            return;
         if (fRecycleBin.size() < fMaxBufferedEvents)
         {
            fNEventsQueued++;
            data.clear();
            fMemInUse += data.capacity();
            fRecycleBin.emplace_back(std::move(data));
         }
         else
         {
            //std::cout << "VECTOR BUFFER FULL!" << typeid(T).name()<<std::endl;
            if (fMemInUse > fMaxMemUse) 
               fMaxMemUse = fMemInUse;
            fNEventsThrownAway++;
            data.clear();
         }
#else 
         data.clear();
#endif
      }
};


class VF48data
{
  public:
     static VectorRecycler<uint32_t> gVF48dataRecycler;
     std::array<std::vector<uint32_t>,nVF48> data32;
  VF48data()
  {
    for (int i=0; i<nVF48; i++)
    {
      data32[i] = gVF48dataRecycler.NewVector();
    }
  }
  /*void Reset()
  {
    for (int i=0; i<nVF48; i++)
    {
      data32[i].clear();
    }
  }*/
  void AddVF48data(const int unit, const uint32_t* data, const int size)
  {

    //std::cout<<"VFModule:"<< unit<<" size:"<<_size<<std::endl;
    //int       size32 = size;
    if (!size) return;
    const size_t old_size = data32[unit].size();
    data32[unit].resize(size + data32[unit].size() );
    std::copy( data , data + size, data32[unit].begin() + old_size );
    return;
  }
  ~VF48data()
  {
     for (int i=0; i<nVF48; i++)
     {
        gVF48dataRecycler.RecycleVector(data32[i]);
     }
  }
};

class VF48DataFlow: public TAFlowEvent
{
   public:
   VF48data VF48dataQueue;
     public:
  VF48DataFlow(TAFlowEvent* flow)
       : TAFlowEvent(flow)
  {

  }

  ~VF48DataFlow()
  {
 
  }
};


class VF48EventFlow: public TAFlowEvent
{
  public:
     VF48event* vf48event;
  public:
  VF48EventFlow(TAFlowEvent* flow, VF48event* e)
       : TAFlowEvent(flow)
  {
    vf48event=e;
  }
  ~VF48EventFlow()
  {
     if (vf48event)
          delete vf48event;
  }
};
#include "TSiliconEvent.h"
#include "TAlphaEvent.h"



class SilEventFlow: public TAFlowEvent
{
  static PointerRecycler<TSiliconEvent> gTSiliconEventRecycleBin;
  static PointerRecycler<TAlphaEvent> gTAlphaEventRecycleBin;

  public:
     TSiliconEvent* silevent;
     TAlphaEvent* alphaevent;
  public:
  SilEventFlow(TAFlowEvent* flow, TSiliconEvent* s)
       : TAFlowEvent(flow)
  {
    silevent=s;
    alphaevent=NULL;
  }
  ~SilEventFlow()
  {
    if (silevent)
      gTSiliconEventRecycleBin.RecycleObject(silevent);
    if (alphaevent)
      gTAlphaEventRecycleBin.RecycleObject(alphaevent);
  }
};


struct OnlineMVAStruct
{
float nhits,residual,r,S0rawPerp,S0axisrawZ,phi_S0axisraw,nCT,nGT,tracksdca,curvemin,curvemean,lambdamin,lambdamean,curvesign,phi;
};
class A2OnlineMVAFlow: public TAFlowEvent
{
  public:
  OnlineMVAStruct* dumper_event;
  double rfout;
  bool pass_online_mva;
  public:
  A2OnlineMVAFlow(TAFlowEvent* flow, OnlineMVAStruct* e) // ctor
   : TAFlowEvent(flow)
   {
     dumper_event=e;
   }
  ~A2OnlineMVAFlow()
 {
    if (dumper_event)
      delete dumper_event;
 }
};


#include "TSISEvent.h"


//Flow for passing SIS data from Analyze function (main thread) to the AnalyzeFlowEvent (side thread)
class SISModuleFlow: public TAFlowEvent
{
  public:
      static VectorRecycler<TSISBufferEvent> gTSISBufferEventRecycleBin;
      std::vector<TSISBufferEvent> fSISBufferEvents[NUM_SIS_MODULES];
      unsigned long MidasEventID=0;
      uint32_t MidasTime=0;

   void AddData(const int module, char* data,const int size)
   {
      if (!size) return;
      uint32_t* ptr = (uint32_t*) data;
      const int nevents = size / 32;
      fSISBufferEvents[module].reserve(nevents);
      for ( int i = 0; i < nevents; i++ )
      {
         fSISBufferEvents[module].emplace_back(module,ptr);
         ptr += NUM_SIS_CHANNELS;
      }
      return;
   }
   void Clear()
   {
      for (int i = 0; i < NUM_SIS_MODULES; i++)
      {
         gTSISBufferEventRecycleBin.RecycleVector(fSISBufferEvents[i]);
      }
   }
   SISModuleFlow(TAFlowEvent* flow): TAFlowEvent(flow)
   {
      for (int i = 0; i < NUM_SIS_MODULES; i++)
      {
         fSISBufferEvents[i] = gTSISBufferEventRecycleBin.NewVector();
      }
   }
   ~SISModuleFlow()
   {
      Clear();
   }
};

class SISEventFlow: public TAFlowEvent
{
  public:
  static VectorRecycler<TSISEvent> gTSISEventRecycleBin;
  std::array<std::vector<TSISEvent>,NUM_SIS_MODULES> sis_events;
  SISEventFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
     for ( int i = 0; i < NUM_SIS_MODULES; i++)
     {
        sis_events[i] = gTSISEventRecycleBin.NewVector();
     }
  }
  ~SISEventFlow()
  {
     for (int j=0; j<NUM_SIS_MODULES; j++)
     {
        gTSISEventRecycleBin.RecycleVector(sis_events[j]);
     }
  }
};

#include "TA2Spill.h"

class A2SpillFlow: public TAFlowEvent
{
  public:
  std::vector<TA2Spill*> spill_events;

  A2SpillFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~A2SpillFlow()
  {
     for (size_t i=0; i<spill_events.size(); i++)
        delete spill_events[i];
     spill_events.clear();
  }

};

#include "TSVD_QOD.h"
class SVDQODFlow: public TAFlowEvent
{
  public:
  std::vector<TSVD_QOD*> SVDQODEvents;
  SVDQODFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~SVDQODFlow()
  {
     for (size_t i=0; i<SVDQODEvents.size(); i++)
        delete SVDQODEvents[i];
     SVDQODEvents.clear();
  }
};

class felabviewFlowEvent: public TAFlowEvent
{
private:
    std::string BankName;
    std::vector<double> m_data;
    uint32_t MIDAS_TIME;
    double run_time;
    double labview_time;
public:
   //Setters and Getters
   virtual std::string            GetBankName()                           { return BankName; }
   virtual std::vector<double>*   GetData()                               { return &m_data; }
   virtual uint32_t               GetMIDAS_TIME()                         { return MIDAS_TIME; }
   virtual double                 GetRunTime()                            { return run_time; }
   virtual double                 GetLabviewTime()                        { return labview_time; }
   
   felabviewFlowEvent(TAFlowEvent* flowevent)
      : TAFlowEvent(flowevent)
   {
   }

    felabviewFlowEvent(TAFlowEvent* flowevent, std::string m_BankName, std::vector<double> m_pdata, 
       uint32_t m_MIDAS_TIME, double m_run_time, double m_labview_time)
      : TAFlowEvent(flowevent), BankName(m_BankName), m_data(m_pdata), 
      MIDAS_TIME(m_MIDAS_TIME), run_time(m_run_time), labview_time(m_labview_time)
   {
      
   }

      felabviewFlowEvent(TAFlowEvent* flowevent, felabviewFlowEvent* flow)
      : TAFlowEvent(flowevent), BankName(flow->BankName), m_data(flow->m_data), 
      MIDAS_TIME(flow->MIDAS_TIME), run_time(flow->run_time), labview_time(flow->labview_time)
   {
   }

   ~felabviewFlowEvent()
   {
   }
};


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */


