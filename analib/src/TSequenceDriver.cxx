#include "TSequenceDriver.h"

ClassImp(TSequencerDriver)

TSequencerDriver::TSequencerDriver()
{
  SeqNum   =0;
  SeqName  ="";
  DriverVer=0;

  NumDO    =0;
  NumTrig  =0;
  NumAO    =0;
  NumHV    =0;

  DigitalMap  = new TSequencerDriverMap();
  TriggerMap  = new TSequencerDriverMap();
  AnalogueMap = new TSequencerDriverMap();
  HVMap       = new TSequencerDriverMap();
}

void TSequencerDriver::Parse(TXMLNode* node)
{
  this->Parse(node,this->DigitalMap ,"DOConfig"    ,"DOCfg"     ,"BitNum"    ,"id");
  this->Parse(node,this->AnalogueMap,"AOChnList"   ,"AOChn"     ,"name"      ,"physChn");
  this->Parse(node,this->TriggerMap ,"HVElectrodes","HVElect"   ,"ElectNum"  ,"HVBitNum");
  this->Parse(node,this->HVMap      ,"TrigInConfig","TrigInLine","BitNum"    ,"id");
}

void TSequencerDriver::Parse(TXMLNode* node,TSequencerDriverMap* map,const char* parent, const char* data, const char* name, const char* idname)
{

    //DOConfig node:
    if (strcmp(parent,node->GetNodeName())==0)
    {
      //std::cout<<"FUCK YEH"<<std::endl;
      //printf(" node: %s\n", node->GetNodeName());
      // display all child nodes
      TXMLNode* child = node->GetChildren();
      while (child!=0)
      {
        Parse(child, map,parent, data,  name, idname);
        child = child->GetNextNode();
      }
    }
    else if (strcmp(data,node->GetNodeName())==0)
    {
      //printf(" node: %s\n", node->GetNodeName());
      if (node->HasAttributes())
      {
        TList* attrList = node->GetAttributes();
        TIter next(attrList);
        TXMLAttr *attr;
        int Addr=-1;
        while ((attr =(TXMLAttr*)next()))
        {
  //        std::cout << attr->GetName() << ":" << attr->GetValue();
          Addr=std::atoi(attr->GetValue());
          map->ChannelNameMap.insert({attr->GetName(),Addr});
        }
        TXMLNode* child = node->GetChildren();
        if (child)
        {
          if (strcmp(child->GetName(),"description"))
             map->ChannelDescriptionMap.insert({(TString)child->GetText(),Addr});
             //std::cout<<"FUCK ME"<<child->GetText() <<std::endl;
          else child=child->GetNextNode();
         
         
          //if (child->GetNodeName()
//          std::cout<<child->GetNodeName()<<std::endl;
        }
        
      }
    }
    else  // display all child nodes
    {
      TXMLNode* child = node->GetChildren();
      //std::cout <<child<<std::endl;
      while (child!=0) {
        Parse(child, map,parent, data,  name, idname);
        child = child->GetNextNode();
      }
    }
    
    
    //Recalculate sizes now:
    NumDO=DigitalMap->ChannelNameMap.size();
    NumTrig=TriggerMap->ChannelNameMap.size();
    NumAO=AnalogueMap->ChannelNameMap.size();
    NumHV=HVMap->ChannelNameMap.size();
    
    return;
  }
