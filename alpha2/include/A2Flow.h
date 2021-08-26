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
#include "TTree.h"
#include "TMVA/Reader.h"

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

class VF48DataFlow: public TAFlowEvent
{
   public:
   std::deque<VF48data*> VF48dataQueue;
     public:
  VF48DataFlow(TAFlowEvent* flow)
       : TAFlowEvent(flow)
  {
  }
  void AddData(VF48data* e)
  {
     VF48dataQueue.push_back(e);
  }
  //Do not delete the pointers... we don't own them
  ~VF48DataFlow()
  {
     VF48dataQueue.clear();
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
      delete silevent;
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

class MVAFlowEvent/*: public TAFlowEvent*/ //Turning this thing being a flow event off, this may be an error but it can't be initialised outside of the flow otherwise. Which is what we want I think. Dont forget to change LINE-LMG01 when changing this back.
{
  public:
  //Needs some data here, and the methods to populate the thing. 
  //THen we need a way for this to interact with the offline MVA analysis. 
  //For now lets assume we are interested in 3 variables. x, y, z.

  Float_t fX;
  Float_t fY;
  Float_t fZ;

  //We then need a way for these to be populated.

  bool UpdateVariables(TAFlowEvent* flow, int currentEventNumber)
  {
    SilEventFlow* siliconEventFlow=flow->Find<SilEventFlow>();
    if (siliconEventFlow)
    {
      TSiliconEvent* siliconEvent=siliconEventFlow->silevent;
      Int_t eventNumber = siliconEvent->GetVF48NEvent();

      if(eventNumber == currentEventNumber)
      {
        std::cout << "current event Number = " << currentEventNumber << ". Matched with found eventNumber " << eventNumber << std::endl;
        std::cout << "(x, y, z) = (" << fX << ", " << fY << ", " << fZ << ", " << ")" << std::endl; 
              
        //Update members
        fX = siliconEvent->GetVertexX();
        fY = siliconEvent->GetVertexY();
        fZ = siliconEvent->GetVertexZ();

        return true;
      }
    }
    return false;
  };

  void LinkTree(TTree *tree)
  {
    tree->Branch("x", &fX, "x/D");
    tree->Branch("y", &fY, "y/D");
    tree->Branch("z", &fZ, "z/D");
  };

  void LoadVariablesToReader(TMVA::Reader* reader)
  {
    reader->AddVariable( "x", &fX );
    reader->AddVariable( "y", &fY );
    reader->AddVariable( "z", &fZ );
  }

  void WriteToTree();

  MVAFlowEvent()/*: TAFlowEvent(flow)*/ //LINE-LMG01 
  {
  }
  ~MVAFlowEvent()
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


