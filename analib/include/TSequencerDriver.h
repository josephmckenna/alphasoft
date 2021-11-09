#ifndef _TSequencerDriver_
#define _TSequencerDriver_

#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#include <vector>
#include <map>

#include <TDOMParser.h>
#include "TXMLNode.h"
#include <TXMLAttr.h>
#include <TList.h>

struct TSequencerDriverMap
{
    std::map<TString,int> ChannelNameMap;
    std::map<TString,int> ChannelDescriptionMap;
    void PrintDriverMap()
    {
      std::map<TString,int>::iterator it;
      for (it = ChannelNameMap.begin(); it != ChannelNameMap.end(); it++)
      {
        std::cout << it->first    
              << ':'
              << it->second    
              << std::endl;
      }
      std::cout<<"------------------------------------------------------------"<<std::endl;
      for (it = ChannelDescriptionMap.begin(); it != ChannelDescriptionMap.end(); it++)
      {
        std::cout << it->first    
              << ':'
              << it->second    
              << std::endl;
      }
    };

    std::vector<int> FindSyncs()
    {
      std::vector<int> SyncChannels;
      std::map<TString,int>::iterator it;
      for (it = ChannelDescriptionMap.begin(); it != ChannelDescriptionMap.end(); it++)
      {
        if(it->first.EndsWith("sync",TString::kIgnoreCase))
          {  
            SyncChannels.push_back(it->second);
            std::cout << it->first    
              << ':'
              << it->second    
              << std::endl;
          }   
      }
      return SyncChannels;
    };

};

class TSequencerDriver : public TObject
{
  public:
    int SeqNum;
    TString SeqName;
    
    int DriverVer;
    
    int NumDO;
    int NumTrig;
    int NumAO;
    int NumHV;
    TSequencerDriverMap* DigitalMap;
    TSequencerDriverMap* TriggerMap;
    TSequencerDriverMap* AnalogueMap;
    TSequencerDriverMap* HVMap;
  
  TSequencerDriver();
  void PrintDatamembers();
  void FindSyncs();
  void Parse(TXMLNode* node);
 /* TSequencerDriver(std::map<TString,int>* _DigitalMap,
                   std::map<TString,int>* _TriggerMap,
                   std::map<TString,int>* _AnalogueMap,
                   std::map<TString,int>* _HVMap)
  {
    DigitalMap =_DigitalMap;
    NumDO=DigitalMap->size();

    TriggerMap =_TriggerMap;
    NumTrig=TriggerMap->size();

    AnalogueMap=_AnalogueMap;
    NumAO=AnalogueMap->size();

    HVMap=_HVMap;
    NumHV=HVMap->size();
  }*/
  ~TSequencerDriver()
  {
    if (DigitalMap)  delete DigitalMap;
    if (TriggerMap)  delete TriggerMap;
    if (AnalogueMap) delete AnalogueMap;
    if (HVMap)       delete HVMap;
  }
  void Clear(Option_t* /*option*/)
  {
    SeqNum   =0;
    SeqName  ="";
    DriverVer=0;

    NumDO    =0;
    NumTrig  =0;
    NumAO    =0;
    NumHV    =0;

    DigitalMap->ChannelNameMap.clear();
    DigitalMap->ChannelDescriptionMap.clear();
    TriggerMap->ChannelNameMap.clear();
    TriggerMap->ChannelDescriptionMap.clear();
    AnalogueMap->ChannelNameMap.clear();
    AnalogueMap->ChannelDescriptionMap.clear();
    HVMap->ChannelNameMap.clear();
    HVMap->ChannelDescriptionMap.clear();
  }
  void Parse(TXMLNode* node,TSequencerDriverMap* map,const char* parent, const char* data, const char* name, const char* idname);
 ClassDef(TSequencerDriver, 1);
};


#endif
