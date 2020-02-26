//
// A2Flow.h
//
// manalyzer flow objects for ALPHA-g events
// JTK McKenna
//



#include "AgFlow.h"
#ifndef A2Flow_H
#define A2Flow_H
#include "UnpackVF48.h"
class VF48data
{
  public:
     int       size32[nVF48];
     uint32_t *data32[nVF48];
    //char* data[NUM_VF48_MODULES];
    //int size[NUM_VF48_MODULES];
  VF48data()
  {
    for (int i=0; i<nVF48; i++)
    {
      size32[i]=0;
      data32[i]=NULL;
    }
  }
  void AddVF48data(int unit, const void* _data, int _size)
  {

    //std::cout<<"VFModule:"<< unit<<" size:"<<_size<<std::endl;
    //int       size32 = size;
  //32const uint32_t *data32 = (const uint32_t*)data;
    if (!_size) return;
    size32[unit]=_size;
    data32[unit]=(uint32_t*) malloc(_size*sizeof(uint32_t));
    memcpy(data32[unit], _data, _size*sizeof(uint32_t));
    return;
  }
  ~VF48data()
  {
     for (int i=0; i<nVF48; i++)
     {
       // if (data32[i])
           free( data32[i] );
     }
  }
};
//Main thread copies VF48event bank data so unpacking can be done in its own thread
class VF48DataFlow: public TAFlowEvent
{
  public:
   VF48data* data;
   VF48DataFlow(TAFlowEvent* flow)
       : TAFlowEvent(flow)
  {
     data=NULL;
  }
  VF48DataFlow(TAFlowEvent* flow,VF48data* _data)
       : TAFlowEvent(flow)
  {
     data=_data;
  }
  ~VF48DataFlow()
  {
     if (data)
        delete data;
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
class SilEventsFlow: public TAFlowEvent
{
  public:
     TSiliconEvent* silevent;
  public:
  SilEventsFlow(TAFlowEvent* flow, TSiliconEvent* s)
       : TAFlowEvent(flow)
  {
    silevent=s;
  }
  ~SilEventsFlow()
  {
     if (silevent)
        delete silevent;
  }
};
#include "TAlphaEvent.h"
class AlphaEventFlow: public TAFlowEvent
{
   public:
      TAlphaEvent* alphaevent;
      AlphaEventFlow(TAFlowEvent* flow, TAlphaEvent* a)
       : TAFlowEvent(flow)
     {
        alphaevent=a;
     }
     ~AlphaEventFlow()
     {
        if (alphaevent)
            delete alphaevent;
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
class SISModuleFlow: public TAFlowEvent
{
  public:
  char* xdata[NUM_SIS_MODULES];
  int xdata_size[NUM_SIS_MODULES]={0};
  unsigned long MidasEventID=0;
  uint32_t MidasTime=0;


  void AddData(int module, char* data, int size)
  {
    //std::cout<<"Module:"<< module<<" size:"<<size<<std::endl;
    xdata_size[module]=size;
    xdata[module]=(char*) malloc(size*4);
    memcpy(xdata[module], data, size*4);
    return;
  }
  void Clear()
  {
    for (int i=0; i<NUM_SIS_MODULES; i++)
    {
      if (xdata_size[i])
      {
        free(xdata[i]);
        xdata_size[i]=0;
      }
    }
  }
  SISModuleFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~SISModuleFlow()
  {
    Clear();
  }
};


class SISEventFlow: public TAFlowEvent
{
  public:
  std::vector<TSISEvent*> sis_events[NUM_SIS_MODULES];
  SISEventFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~SISEventFlow()
  {
     for (int j=0; j<NUM_SIS_MODULES; j++)
     {
        for (size_t i=0; i<sis_events[j].size(); i++)
        {
           delete sis_events[j].at(i);
        }
        sis_events[j].clear();
     }
  }
};

#include "TSpill.h"

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

#include "TStoreA2Event.hh"
class A2AnalysisFlow: public TAFlowEvent
{
 public:
   TStoreA2Event* analyzed_event;

 public:
 A2AnalysisFlow(TAFlowEvent* flow) // ctor
   : TAFlowEvent(flow)
   {  }
   ~A2AnalysisFlow()
  {
    if (analyzed_event)
       delete analyzed_event;
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


