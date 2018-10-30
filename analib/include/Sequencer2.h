#ifndef SEQUENCER2_H
#define SEQUENCER2_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

// Required for SeqXML


#include <TObject.h>
#include <TDOMParser.h>
#include <TXMLAttr.h>
#include <TXMLNode.h>
#include <TMap.h>
#include <TString.h>
#include <TObjString.h>
#include <TObjArray.h>
#include <TList.h>
#include <TString.h>
// #define SEQDEBUG

#ifdef SEQDEBUG
#define SQDG(X) cout << X;
#endif

#ifndef SEQDEBUG
#define SQDG(X)
#endif

//#define SEQDEBUG2

#ifdef SEQDEBUG2
#define SQDG2(X) cout << "SQDG2** " << X;
#else
#define SQDG2(X)
#endif


typedef int (*parseFPtr)(void* dataObj, TXMLNode* n);

class SeqXML_Obj : public TObject
{

  public:

  static void AutomaticParseList(void* dataObj, TXMLNode* startnode, TMap* tfm, std::vector<parseFPtr> pfcns);
  static TXMLNode* FindNode(TXMLNode*node,const char*name);
  static Int_t TextNodeToIntT(TXMLNode *node);
  static TString TextNodeToTString(TXMLNode *node);

  static Int_t AttrToInt_t(TXMLNode *n, TObjString *name);
  static TString AttrToTString(TXMLNode *n, TObjString *name);
  static  Double_t AttrToDouble_t(TXMLNode *n, TObjString *name);
  //  static DoubleT TextNodeToDoubleT(TXMLNode *node);
  
  ClassDef(SeqXML_Obj,1)
};



/* SequenceDriverConsts Classes */  
 
class Seq_DriverConsts {

protected:

  int driverVers;
  int numDOLines;
  int numTrigInLines;
  int numAOTrigLines;
  int _XMLVer;

public:

  Seq_DriverConsts ();
  virtual ~Seq_DriverConsts() {  };

  // accessors
  int getNumDOLines(){return numDOLines;}
  int getDriverVers(){return driverVers;} 
  int getNumTrigInLines() {return numTrigInLines;} 
  int getNumAOTrigLines() {return numAOTrigLines;}  
  int getXMLVer() { return _XMLVer;}

  // utils
  friend std::ostream& operator<<(std::ostream& o, const Seq_DriverConsts& sdc);

  ClassDef(Seq_DriverConsts,1)
};

class SeqXML_DriverConsts : public Seq_DriverConsts, public SeqXML_Obj{

private:

  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

  static int Parse_DriverVers(void* dataObj, TXMLNode* n);
  static int Parse_NumDOLines(void* dataObj, TXMLNode* n);
  static int Parse_NumTrigInLines(void* dataObj, TXMLNode* n);
  static int Parse_NumAOTrigLines(void* dataObj, TXMLNode* n);


public:

  static TObjString TAG_DriverConstsNode ;
  static TObjString TAG_DriverVers ;
  static TObjString TAG_NumDOLines ;
  static TObjString TAG_NumTrigInLines;
  static TObjString TAG_NumAOTrigLines;

  // AO Config
  static TObjString TAG_AOConfig;


  SeqXML_DriverConsts(TXMLNode* n);
  SeqXML_DriverConsts(){;}

  ClassDef(SeqXML_DriverConsts,1)
};


/* Sequencer Classes */

class Seq{
  
public:
  // accessors
  Seq_DriverConsts* getSeq_DriverConsts() {return NULL;};
  
  // constructors
  Seq();
  // destructor
  virtual ~Seq() {};
  
  ClassDef(Seq,1)
};

class SeqXML;

/* BEGIN class SeqXML_AOChn */
class SeqXML_AOChn : public SeqXML_Obj{

private:

  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

  static int Parse_AOChn(void* dataObj, TXMLNode* n);
  
  Int_t _ind;
  Int_t _AOBank;
  Double_t _maxV;
  Double_t _minV;
  TString _name;
  Int_t _num;
  Int_t _physChn;
  Char_t _type;

public:

  // Head tag
  static TObjString TAG_AOChn;
  static TObjString ATT_ind;
  static TObjString ATT_AOBank;
  static TObjString ATT_maxV;
  static TObjString ATT_minV;
  static TObjString ATT_name;
  static TObjString ATT_num;
  static TObjString ATT_physChn;
  static TObjString ATT_type;

  // constructor
  SeqXML_AOChn(SeqXML* seq, TXMLNode* n);
  SeqXML_AOChn(){;}
  ~SeqXML_AOChn() { SQDG("deleting AO Chn ind: " << _ind << endl);}

  // accessors
  Int_t getInd() { return _ind;}
  Int_t getAOBank() { return _AOBank;}
  Double_t getMaxV() { return _maxV;}
  Double_t getMinV() { return _minV;}
  TString getChnName() { return _name;}
  Int_t getNum() { return _num;}
  Int_t getPhysChn() { return _physChn;}
  Char_t getType() { return _type;}

  ClassDef(SeqXML_AOChn,1)

};
/* END class SeqXML_AOChn */

/* BEGIN class SeqXML_HVElec */
class SeqXML_HVElec : public SeqXML_Obj{

private:

  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

  static int Parse_AOChnList(void* dataObj, TXMLNode* n);
  static int Parse_HVElectrodes(void* dataObj, TXMLNode* n);

  int _bitNum;
  int _elecNum;
  int _ind;

public:

  // Head tag
  static TObjString TAG_HVElec;
  static TObjString ATT_ElecNum;
  static TObjString ATT_HVBitNum;
  static TObjString ATT_ind;

  // constructor
  SeqXML_HVElec(SeqXML* seq, TXMLNode* n);
  SeqXML_HVElec(){;}

  // destructor
  ~SeqXML_HVElec(){SQDG("Deleting HVElec num: " << _ind << endl);}

  // accessors
  int getBitNum() { return _bitNum;}
  int getElecNum() { return _elecNum;}
  int getInd() {return _ind;}

  ClassDef(SeqXML_HVElec,1)
}; 
/* END class SeqXML_HVElec*/


/* BEGIN class SeqXML_AOConfig */
class SeqXML_AOConfig : public SeqXML_Obj{

private:

  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

  static int Parse_AOChnList(void* dataObj, TXMLNode* n);
  static int Parse_HVElectrodes(void* dataObj, TXMLNode* n);
  static int Parse_AOChn(void* dataObj, TXMLNode* n);
  static int Parse_HVElec(void* dataObj, TXMLNode* n);

  std::vector<SeqXML_AOChn*>* _AOChns;
  std::vector<SeqXML_HVElec*>* _HVElecs;
  SeqXML* _thisSeq;
  
public:

  // constructor
  SeqXML_AOConfig(SeqXML* seq, TXMLNode* n);
  SeqXML_AOConfig();

  // Head tag
  static TObjString TAG_AOConfig;
  static TObjString TAG_AOChnList;
  static TObjString TAG_HVElectrodes;

  // accessors
  Int_t getNumAOChns() { return _AOChns->size();}
  

  // destructor
  ~SeqXML_AOConfig();

  ClassDef(SeqXML_AOConfig,1)

};
/* END class SeqXML_AOConfig */


/* BEGIN class SeqXML */

class SeqXML : public Seq, public SeqXML_Obj {
private:


  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

  TString _sequencerName;
  SeqXML_DriverConsts* driverConsts;
  SeqXML_AOConfig* _AOConfig;
  TObjArray* _chainLinks;

public:
  static TObjString TAG_SequenceHead;  
  static TObjString TAG_SequencerName;
  static TObjString TAG_Chain;
  static TObjString TAG_ChainLink;

  // constructors
  SeqXML(TXMLNode* head);
  SeqXML();

  // accessors
  SeqXML_DriverConsts* getSeqXML_DriverConsts() { return driverConsts;}
  Seq_DriverConsts* getSeq_DriverConsts() {return (Seq_DriverConsts*) driverConsts;}
  TString getSequencerName() { return _sequencerName;}
  TObjArray* getChainLinks() {return _chainLinks;}
  SeqXML_AOConfig* getAOConfig() {return _AOConfig;}

  // TObject
  using TObject::Print;
  virtual void Print() {
    std::cout << "Sequencer " << _sequencerName << std::endl;
    std::cout << "Sequence Chain Links: " << std::endl;
    _chainLinks->Print();
  }
  

  // parse functions
  static int Parse_SequencerName(void* dataObj, TXMLNode* n);
  static int Parse_Chain(void* dataObj, TXMLNode* n);

  // destructor
  ~SeqXML();

  ClassDef(SeqXML,1)
};

/* END class SeqXML */

//predeclaration
class SeqXML_ChainLink;

// begin class SeqXML_State //

class SeqXML_State : public SeqXML_Obj 
{
private:

  friend class SeqXML_ChainLink;

  Int_t _id;
  Double_t _time;
  Bool_t _isLoopReturn;
  Int_t _loopCnt;
  std::vector<Bool_t>* _DO;
  std::vector<Double_t>* _AOi;
  std::vector<Double_t>* _AOf;
  TString* _comment;
  SeqXML* _thisSeq;

  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

protected:
  // Data for count finding
  Int_t _cntPerIteration;
  Int_t _fullCnt;
  Int_t _loopHead;
  Int_t _returnState;
  Int_t _firstCnt;

public:

  static TObjString TAG_Time;
  static TObjString TAG_DO;

  // loop
  static TObjString TAG_Loop;
  static TObjString ATT_count;
  static TObjString ATT_isReturn;

  // loop child tags
  static TObjString TAG_LoopCnt;

  static TObjString TAG_AO;
  static TObjString TAG_Vf;
  static TObjString TAG_Vi;
  static TObjString ATT_StateID;
  static TObjString TAG_Comment;

  // accessors
  Int_t getID(){return _id;}
  Double_t getTime(){return _time;}
  Bool_t isLoopReturn() {return _isLoopReturn;}
  Int_t getLoopCnt() {return _loopCnt;}
  TString* getComment() {return _comment;}
  Bool_t getDO(Int_t);
  
  // parsers
  static int Parse_Time(void* dataObj, TXMLNode* n);
  static int Parse_DO(void* dataObj, TXMLNode* n);
  static int Parse_Loop(void* dataObj, TXMLNode* n);
  static int Parse_AO(void* dataObj, TXMLNode* n);
  static int Parse_Comment(void* dataObj, TXMLNode* n);

  // utils
  static void ParseVList(TString Vs, std::vector<Double_t> *V, Int_t maxLvls);
  void PrintDO();

  // TObject
  using TObject::Print;
  virtual void Print() ;

  // constructor
  SeqXML_State(SeqXML* seq, TXMLNode* n);
  SeqXML_State() {;}

  // destructor
  ~SeqXML_State();

  ClassDef(SeqXML_State,1)

};
// End class SeqXML_State //

/* BEGIN class SeqXML_Event */

class SeqXML_Event : public SeqXML_Obj
{
  friend class SeqXML_ChainLink;

private:
  Int_t _id;
  TString _name;
  TString _description;
  Int_t _onState;
  Int_t _onCnt;
  
  SeqXML* _thisSeq;
  
  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

public:

  static TObjString TAG_Name;
  static TObjString TAG_Description;
  static TObjString TAG_OnState;
  static TObjString ATT_ID;

  // accessors
  Int_t GetID() {return _id;}
  TString GetNameTS() { return _name;}
  using TObject::GetName;
  const char * GetName () { return (_name.Data());}
  TString GetDescription() { return _description;}
  Int_t GetStateID(){ return _onState;}
  Int_t GetCount(){ return _onCnt;}

  // parsers
  static int Parse_Name(void* dataObj, TXMLNode* n);
  static int Parse_Description(void* dataObj, TXMLNode* n);
  static int Parse_OnState(void* dataObj, TXMLNode* n);

  // util
  using TObject::Print;
  virtual void Print() ;

  // constructor
  SeqXML_Event(SeqXML* seq, TXMLNode* n);
  SeqXML_Event(){;}

  // destructor
  ~SeqXML_Event();

  ClassDef(SeqXML_Event,1)
};
// End class SeqXML_Event //




// Begin class SeqXML_ChainLink //

class SeqXML_ChainLink : public SeqXML_Obj {
private:

  static TObjString TAG_State;
  static TObjString ATT_StateID;
  static TObjString TAG_NumStates;
  static TObjString TAG_EventTable;

  // Actually sub-tags of TAG_EventTable
  static TObjString TAG_Event;
  static TObjString TAG_NumEvents;

  TObjArray* _states;
  TObjArray* _events;
  Int_t _parsedNumEvents;
  Int_t _parsedNumStates;

  Int_t _numEvents;
  Int_t _numStates;

  SeqXML* _thisSeq;

  // Variables for counting state
  Int_t _totalCnt;
  

  static TMap* TagFunctionMap;
  static std::vector<parseFPtr> parseFunctions;

  // utils
  void InitCntSearch();
  void CalcCntsPerIteration(Int_t ind, Int_t *next_ind, Int_t *current_Cnt, Int_t loopHead, Int_t loopDepth);
  Bool_t FindState(Int_t* cnt, Int_t ind, Int_t *next_ind);

  // pasrsers
  static int Parse_State(void* dataObj, TXMLNode* n);
  static int Parse_EventTable(void* dataObj, TXMLNode* n);
  static int Parse_NumStates(void* dataObj, TXMLNode* n);
  static int Parse_Event(void* dataObj, TXMLNode* n);
  static int Parse_NumEvents(void* dataObj, TXMLNode* n);

public:

  //constructor
  SeqXML_ChainLink(SeqXML* seq, TXMLNode* n);
  SeqXML_ChainLink() {;}

  //destructor
  ~SeqXML_ChainLink();

  // accessors
 
  Int_t gettotalCnt() {return _totalCnt; }
  TObjArray* getStates(){return _states;}
  // array of SeqXML_State*
  SeqXML_State* getState(Int_t ind){   return (SeqXML_State*)_states->At(ind);}
  Int_t getNumStates() {return _numStates;}

  TObjArray* getEvents(){return _events;}
  // array of SeqXML_Event*
  SeqXML_Event* getEvent(Int_t ind) {  return (SeqXML_Event*)_events->At(ind);}
  Int_t getNumEvents() {return _numEvents;}

  // utils
  Int_t getStateFromCount(Int_t cnt);
  Int_t getCountFromState(Int_t state);

  Int_t getTotalCount() {return _totalCnt;}
  void printEventTable();

  // TObject
  using TObject::Print;
  virtual void Print() 
  { 
    std::cout << "ChainLink States: " << std::endl;  
    _states->Print();
    std::cout << std::endl << "Chainlink Events: " << std::endl;
    _events->Print();
  }

  ClassDef(SeqXML_ChainLink,1)

};

// End class SeqXML_ChainLink //

#endif
