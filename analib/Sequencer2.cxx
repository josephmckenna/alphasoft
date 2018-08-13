using namespace std;

#include <assert.h>
#include <stdexcept>
#include <vector>
#include "Sequencer2.h"
#include <iostream>



class SeqInt : public TObject{
public :

  Int_t val;
  Int_t getVal() {
    return val;
  }

  void setVal(Int_t newVal) {
    val = newVal;
  }

  SeqInt(int i)
  {
    val = i;
  }

  ULong_t Hash()
  {
    return (ULong_t) val;
  }
};

Seq_DriverConsts::Seq_DriverConsts(){
  // cout << "called default constructor" << endl ;
  driverVers = -1;
  numDOLines = -1;
  numTrigInLines = -1;
  numAOTrigLines = -1;
}

// Definition of << operator Seq_DriverConsts  Woot.

inline ostream& operator<<(ostream& o, const Seq_DriverConsts& sdc) {
  o << "Seq_DriverConsts:" << endl  
      << " driverVers: " << sdc.driverVers << endl 
      << " NumDOLines: " << sdc.numDOLines << endl 
      << " NumDOLines: "  << sdc.numTrigInLines << endl 
      << " NumAOTrigLines: " << sdc.numAOTrigLines << endl;

    return o;
  }

ClassImp(Seq_DriverConsts)

// Definite of SeqXML_DriverConsts

TObjString SeqXML_DriverConsts::TAG_DriverConstsNode = TObjString("SequencerDriverConsts");
TObjString SeqXML_DriverConsts::TAG_DriverVers = TObjString("DriverVers");
TObjString SeqXML_DriverConsts::TAG_NumDOLines = TObjString("NumDOLines");
TObjString SeqXML_DriverConsts::TAG_NumTrigInLines = TObjString("NumTrigInLines");
TObjString SeqXML_DriverConsts::TAG_NumAOTrigLines = TObjString("NumAOTrigLines");
TObjString SeqXML_DriverConsts::TAG_AOConfig = TObjString("AOConfig");
TMap* SeqXML_DriverConsts::TagFunctionMap=NULL;
vector<parseFPtr> SeqXML_DriverConsts::parseFunctions;

SeqXML_DriverConsts::SeqXML_DriverConsts(TXMLNode* n)
{
  _XMLVer = 1;
  // node n should be the DriverConsts node

  // cout << "init SeqXML_DriverConsts" << endl; 
  // cout << "a constant: " << TAG_DriverConstsNode.String() << endl;
  
  if(!TagFunctionMap) {
    TagFunctionMap = new TMap(10,0);
    
    TagFunctionMap->Add(&TAG_DriverVers, new SeqInt(0));
    TagFunctionMap->Add(&TAG_NumDOLines, new SeqInt(1));
    TagFunctionMap->Add(&TAG_NumTrigInLines, new SeqInt(2));
    TagFunctionMap->Add(&TAG_NumAOTrigLines, new SeqInt(3));

    parseFunctions.push_back(&SeqXML_DriverConsts::Parse_DriverVers); 
    parseFunctions.push_back(&SeqXML_DriverConsts::Parse_NumDOLines); 
    parseFunctions.push_back(&SeqXML_DriverConsts::Parse_NumTrigInLines); 
    parseFunctions.push_back(&SeqXML_DriverConsts::Parse_NumAOTrigLines); 

    // cout << "done with the function list" << endl;
  }
  
  if(TString(n->GetNodeName()) == TAG_DriverConstsNode.String())
    {
      AutomaticParseList(this, n->GetChildren(), TagFunctionMap, parseFunctions);
    }
  // Didn't find the node. default values will be used.
}

int SeqXML_DriverConsts::Parse_DriverVers(void* dataObj, TXMLNode* n)
{

  SeqXML_DriverConsts* dc = (SeqXML_DriverConsts*) dataObj;
  // cout << "Parse driver vers" << endl;
  // cout << " content: " << n->GetText() << endl;

  dc->driverVers = TextNodeToIntT(n);
  return 0;
}

int SeqXML_DriverConsts::Parse_NumDOLines(void* dataObj, TXMLNode* n)
{
  SeqXML_DriverConsts* dc = (SeqXML_DriverConsts*) dataObj;
  //cout << "parse NumDOLines" << endl;
  dc->numDOLines = TextNodeToIntT(n);
  return 0;
}

int SeqXML_DriverConsts::Parse_NumTrigInLines(void* dataObj, TXMLNode* n)
{
  SeqXML_DriverConsts* dc = (SeqXML_DriverConsts*) dataObj;
  //cout << "parse NumTrigInLines" << endl;
  dc->numTrigInLines = TextNodeToIntT(n);
  return 0;
}

int SeqXML_DriverConsts::Parse_NumAOTrigLines(void* dataObj, TXMLNode* n)
{
  SeqXML_DriverConsts* dc = (SeqXML_DriverConsts*) dataObj;
  //cout << "parse NumAOTrigLines" << endl;
  dc->numAOTrigLines = TextNodeToIntT(n);
  return 0;
}

/* END class SeqXML_DriverConsts */

/* BEGIN class SeqXML_Obj */
void SeqXML_Obj::AutomaticParseList(void* dataObj, TXMLNode* startNode, TMap* tfm, vector<parseFPtr> pfcns)
{
  TXMLNode* currentNode = startNode;
  while(currentNode)
    { 
      TObjString name(currentNode->GetNodeName());
      SQDG ( "parsing node : " << name.String() << endl);
      //cout << "content : " << currentNode->GetText() << endl;
      TObject* ind_o = tfm->FindObject(&name);
      
      // if we find this string in the list of strings, run its parse function. 
      // Otherwise, we have no automatic action for the node, so do nothing.
      
      if(ind_o)
        {
          //cout << "found function " << endl;
          SeqInt* ind = (SeqInt*)((TPair*)ind_o)->Value();
          pfcns[ind->getVal()](dataObj,currentNode);
        }

      // done with this node. Get the next sibling node.
      currentNode = currentNode->GetNextNode();
    }
}

 TXMLNode* SeqXML_Obj::FindNode(TXMLNode* node,const char* name)
{
  for (; node != NULL; node = node->GetNextNode())
    {
      //printf("node name: \"%s\"\n",node->GetNodeName());
      if (strcmp(node->GetNodeName(),name) == 0)
        return node;
      
      if (node->HasChildren())
        {
          TXMLNode* found = FindNode(node->GetChildren(),name);
          if (found)
            return found;
        }
    }
  
  return NULL;
} 

Int_t SeqXML_Obj::TextNodeToIntT(TXMLNode *node)
{
  if(node)
    {
      TString val(node->GetText());
      SQDG( "converting string: " << val << endl);
      if(val.IsDigit())
        {
          return val.Atoi();

        }
    }

  //ERROR - NO INTEGER IN THIS NODE
  return -2;

}

TString SeqXML_Obj::TextNodeToTString(TXMLNode *node)
{
  if(node) {
    return TString(node->GetText());
  }

  //ERROR - Not a valid node
  return TString("");
}

Int_t SeqXML_Obj::AttrToInt_t(TXMLNode *n, TObjString *name)
{
  TList* atts = n->GetAttributes();
  TXMLAttr* att;
  if((att = (TXMLAttr*)(atts->FindObject(name->String()))))
    {
      return((Int_t) atoi(att->GetValue()));
    }
  else return (0);
} 

TString SeqXML_Obj::AttrToTString(TXMLNode *n, TObjString *name)
{
  TList* atts = n->GetAttributes();
  TXMLAttr* att;
  if((att = (TXMLAttr*)(atts->FindObject(name->String()))))
    {
      return(TString(att->GetValue()));
    }
  else return (TString(""));
} 

Double_t SeqXML_Obj::AttrToDouble_t(TXMLNode *n, TObjString *name)
{
  TList* atts = n->GetAttributes();
  TXMLAttr* att;
  if((att = (TXMLAttr*)(atts->FindObject(name->String()))))
    {
      return((Double_t) atof(att->GetValue()));
    }
  else return (0.0);
} 

ClassImp(SeqXML_Obj)
/* END class SeqXML_Obj */

/* BEGIN class SeqXML_Event */
TObjString SeqXML_Event::ATT_ID = "id";
TObjString SeqXML_Event::TAG_Name = "name";
TObjString SeqXML_Event::TAG_Description = "description";
TObjString SeqXML_Event::TAG_OnState  = "onState";

TMap* SeqXML_Event::TagFunctionMap=NULL;
vector<parseFPtr> SeqXML_Event::parseFunctions;

//constructor

SeqXML_Event::SeqXML_Event(SeqXML* seq, TXMLNode* n) {
  if(!TagFunctionMap) {
    TagFunctionMap = new TMap(10,0);
    TagFunctionMap->Add(&TAG_Name, new SeqInt(0));
    TagFunctionMap->Add(&TAG_Description, new SeqInt(1));
    TagFunctionMap->Add(&TAG_OnState, new SeqInt(2));

    
    parseFunctions.push_back(&SeqXML_Event::Parse_Name);
    parseFunctions.push_back(&SeqXML_Event::Parse_Description);
    parseFunctions.push_back(&SeqXML_Event::Parse_OnState);
  }
  _thisSeq = seq;

TList* atts = n->GetAttributes();
  TXMLAttr* id_att;

  if((id_att = (TXMLAttr*)(atts->FindObject(ATT_ID.String()))))
    {
      // found an id;
      _id = atoi(id_att->GetValue());
    }
  else
    {
      cout << "Event parse error: no id found" << endl;
    }

  AutomaticParseList(this, n->GetChildren(), TagFunctionMap, parseFunctions);
}

int SeqXML_Event::Parse_Name(void* dataObj, TXMLNode* n)
{
  SeqXML_Event* e = (SeqXML_Event*)dataObj;
  e->_name = TextNodeToTString(n);
  SQDG("parsed event name: " << e->_name << endl);
  return 0;
}

int SeqXML_Event::Parse_Description(void* dataObj, TXMLNode* n)
{
  SeqXML_Event* e = (SeqXML_Event*)dataObj;
  e->_description = TextNodeToTString(n);
  SQDG("parsed event description: " << e->_description);
  return 0;
}

int SeqXML_Event::Parse_OnState(void* dataObj, TXMLNode* n)
{
  SeqXML_Event* e = (SeqXML_Event*)dataObj;
  e->_onState = TextNodeToIntT(n);
  SQDG("parsed event OnState: " << e->_onState << endl);
  return 0;
}

void SeqXML_Event::Print(Option_t* opt)
{
  cout << "Event ID: " << _id <<
    " name: " << _name <<
    " description: " << _description <<
    " onCnt: " << _onCnt <<
    " onState: " << _onState << endl;
}

SeqXML_Event::~SeqXML_Event()
{
}

ClassImp(SeqXML_Event)

/* END class SeqXML_Event */

/* BEGIN class SeqXML_AOChn */
TObjString SeqXML_AOChn::TAG_AOChn = "AOChn";
TObjString SeqXML_AOChn::ATT_ind = "ind";
TObjString SeqXML_AOChn::ATT_AOBank = "AOBank";
TObjString SeqXML_AOChn::ATT_maxV = "maxV";
TObjString SeqXML_AOChn::ATT_minV = "minV";
TObjString SeqXML_AOChn::ATT_name = "name";
TObjString SeqXML_AOChn::ATT_num = "num";
TObjString SeqXML_AOChn::ATT_physChn = "physChn";
TObjString SeqXML_AOChn::ATT_type = "type";

SeqXML_AOChn::SeqXML_AOChn(SeqXML* seq, TXMLNode* n)
{
  if(!strcmp(n->GetNodeName(), TAG_AOChn.String().Data()))
    {
      // we have an AO Chn
      SQDG("parsing AO Chn: ");

      _ind = AttrToInt_t(n, &ATT_ind);
      _AOBank = AttrToInt_t(n, &ATT_AOBank);
      _num = AttrToInt_t(n, &ATT_num);
      _physChn = AttrToInt_t(n, &ATT_physChn);
      _type = AttrToTString(n, &ATT_type)[0];
      _name = AttrToTString(n, &ATT_name);
      _maxV = AttrToDouble_t(n, &ATT_maxV);
      _minV = AttrToDouble_t(n, &ATT_minV);
      
      SQDG(
           "ind: " << _ind << 
           "bank: " << _AOBank << 
           "num: " << _num << 
           "physChn" << _physChn <<
           "type" << _type <<
           "name" << _name <<
           "maxV" << _maxV <<
           "minV" << _minV << endl);
    }
  else
    {
      cout << "Error parsing AOChn - this is not an AOChn" << endl;
    }
}

ClassImp(SeqXML_AOChn)

/* END class SeqXML_AOChn */

/* BEGIN class SeqXML_HVElec */
TObjString SeqXML_HVElec::TAG_HVElec = "HVElect";
TObjString SeqXML_HVElec::ATT_ElecNum= "ElectNum";
TObjString SeqXML_HVElec::ATT_HVBitNum = "HVBitNum";
TObjString SeqXML_HVElec::ATT_ind = "ind";

SeqXML_HVElec::SeqXML_HVElec(SeqXML* seq, TXMLNode* n)
{
  if(!strcmp(n->GetNodeName(), TAG_HVElec.String().Data()))
    {
      // we have a HVElec
      SQDG("parsing HVElec: ");

      _bitNum = AttrToInt_t(n, &ATT_HVBitNum);
      _elecNum = AttrToInt_t(n, &ATT_ElecNum);
      _ind = AttrToInt_t(n, &ATT_ind);
      
      SQDG(
           "ind: " << _ind << 
           "bitNum: " << _bitNum << 
           "electNum" << _elecNum << endl);
    }
  else
    {
      cout << "Error parsing HVElec - this is not an HVElec" << endl;
    }
}


ClassImp(SeqXML_HVElec)

/* END class SeqXML_HVChn */

/* BEGIN class SeqXML_AOConfig */
TObjString SeqXML_AOConfig::TAG_AOConfig = "AOConfig";
TObjString SeqXML_AOConfig::TAG_AOChnList = "AOChnList";
TObjString SeqXML_AOConfig::TAG_HVElectrodes = "HVElectrodes";

TMap* SeqXML_AOConfig::TagFunctionMap=NULL;
vector<parseFPtr> SeqXML_AOConfig::parseFunctions;

SeqXML_AOConfig::SeqXML_AOConfig():_AOChns(0),_HVElecs(0) {;}

SeqXML_AOConfig::SeqXML_AOConfig(SeqXML* seq, TXMLNode* n) {
  if(!TagFunctionMap) {
    TagFunctionMap = new TMap(10,0);
    TagFunctionMap->Add(&TAG_AOChnList, new SeqInt(0));
    TagFunctionMap->Add(&TAG_HVElectrodes, new SeqInt(1));
    TagFunctionMap->Add(&SeqXML_AOChn::TAG_AOChn, new SeqInt(2));
    TagFunctionMap->Add(&SeqXML_HVElec::TAG_HVElec, new SeqInt(3));
    
    parseFunctions.push_back(&SeqXML_AOConfig::Parse_AOChnList);
    parseFunctions.push_back(&SeqXML_AOConfig::Parse_HVElectrodes);
    parseFunctions.push_back(&SeqXML_AOConfig::Parse_AOChn);
    parseFunctions.push_back(&SeqXML_AOConfig::Parse_HVElec);
  }

  _AOChns = new vector<SeqXML_AOChn*>();
  _HVElecs = new vector<SeqXML_HVElec*>();
  _thisSeq = seq;

  AutomaticParseList(this, n->GetChildren(), TagFunctionMap, parseFunctions);
}

int SeqXML_AOConfig::Parse_AOChnList(void* dataObj, TXMLNode* n){
  SQDG("Parsing AO channel list" << endl);
  AutomaticParseList(dataObj, n->GetChildren(), TagFunctionMap, parseFunctions);
  
  return 0;
}

int SeqXML_AOConfig::Parse_HVElectrodes(void* dataObj, TXMLNode* n) {
  SQDG("Parsing HV Channel List" << endl);
  AutomaticParseList(dataObj, n->GetChildren(), TagFunctionMap, parseFunctions);
  
  return 0;
}

int SeqXML_AOConfig::Parse_AOChn(void* dataObj, TXMLNode* n){
  SeqXML_AOConfig* ac = (SeqXML_AOConfig*) dataObj;

  ac->_AOChns->push_back(new SeqXML_AOChn(ac->_thisSeq, n));
  return 0;
}

int SeqXML_AOConfig::Parse_HVElec(void* dataObj, TXMLNode* n){
  SeqXML_AOConfig* ac = (SeqXML_AOConfig*) dataObj;

  ac->_HVElecs->push_back(new SeqXML_HVElec(ac->_thisSeq, n));
  return 0;
}

SeqXML_AOConfig::~SeqXML_AOConfig()
{
  SQDG("Deleting AOConfig" << endl);
  
  if(_AOChns!=0)
    for(int i = 0 ; i<(int)_AOChns->size();i++)
      {
	if( (*_AOChns)[i] )
	  delete((*_AOChns)[i]);
      }

  if(_HVElecs!=0)
  for(int i = 0 ; i<(int)_HVElecs->size();i++)
    {
      if( (*_HVElecs)[i] )
	delete((*_HVElecs)[i]);
    }

  if(_AOChns) delete(_AOChns);
  if(_HVElecs) delete(_HVElecs);
}

ClassImp(SeqXML_AOConfig)

/* END class SeqXML_AOConfig */


/* BEGIN class SeqXML_State */
TObjString SeqXML_State::TAG_Time = "Time";
TObjString SeqXML_State::TAG_DO = "DO";
TObjString SeqXML_State::TAG_Comment = "comment";

TObjString SeqXML_State::TAG_Loop  = "Loop";
TObjString SeqXML_State::TAG_LoopCnt = "LCnt"; // subtag of Loop node
TObjString SeqXML_State::ATT_count = "count";
TObjString SeqXML_State::ATT_isReturn = "return";

TObjString SeqXML_State::TAG_AO = "AO";
TObjString SeqXML_State::TAG_Vi = "Vi";
TObjString SeqXML_State::TAG_Vf = "Vf";
TObjString SeqXML_State::ATT_StateID = "id";

TMap* SeqXML_State::TagFunctionMap=NULL;
vector<parseFPtr> SeqXML_State::parseFunctions;
// constructor
SeqXML_State::SeqXML_State(SeqXML* seq, TXMLNode* n) {
  if(!TagFunctionMap) {
    TagFunctionMap = new TMap(10,0);
    TagFunctionMap->Add(&TAG_Time, new SeqInt(0));
    TagFunctionMap->Add(&TAG_DO, new SeqInt(1));
    TagFunctionMap->Add(&TAG_Loop, new SeqInt(2));
    TagFunctionMap->Add(&TAG_AO, new SeqInt(3));
    TagFunctionMap->Add(&TAG_Comment, new SeqInt(4));
    
    parseFunctions.push_back(&SeqXML_State::Parse_Time);
    parseFunctions.push_back(&SeqXML_State::Parse_DO);
    parseFunctions.push_back(&SeqXML_State::Parse_Loop);
    parseFunctions.push_back(&SeqXML_State::Parse_AO);
    parseFunctions.push_back(&SeqXML_State::Parse_Comment);
  }
  
  // set som default values of importance
  _loopCnt = 1;
  _isLoopReturn = false;
  _comment = NULL;

  TList* atts = n->GetAttributes();
  TXMLAttr* id_att;
  if((id_att = (TXMLAttr*)(atts->FindObject(ATT_StateID.String()))))
    {
      // found an id;
      _id = atoi(id_att->GetValue());
    }
  else
    {
      cout << "State parse error: no id found" << endl;
    }

  _thisSeq = seq;
  _DO = new vector<Bool_t>(seq->getSeqXML_DriverConsts()->getNumDOLines());
  _AOi = new vector<Double_t>(seq->getAOConfig()->getNumAOChns());
  _AOf = new vector<Double_t>(seq->getAOConfig()->getNumAOChns());

  AutomaticParseList(this, n->GetChildren(), TagFunctionMap, parseFunctions);

  if(!_comment) _comment = new TString("");
}


// parse functions

int SeqXML_State::Parse_Time(void* dataObj, TXMLNode* n) {
  SeqXML_State* state = (SeqXML_State*) dataObj;
  const char* ts = n->GetText();
  state->_time = atof(ts);
  SQDG( "parsing a state time of: " << state->_time << endl);
  return 0;
}

int SeqXML_State::Parse_Comment(void* dataObj, TXMLNode* n) {
  SeqXML_State* state = (SeqXML_State*) dataObj;
  const char* comment = n->GetText();
  state->_comment = new TString(comment);
  SQDG( "state has a comment: " << state->_comment << endl);
  return 0;
}

int SeqXML_State::Parse_DO(void* dataObj, TXMLNode* n){

  SeqXML_State* state = (SeqXML_State*) dataObj;
  SQDG("parsing DO" << endl);
  TString dos(n->GetText());
  if(dos.Length() != state->_thisSeq->getSeqXML_DriverConsts()->getNumDOLines())
    {
      cout << "Error parsing DO Lines: num lines don't match driver consts" << endl;
    }
  else
    {
      for(int i = 0; i < dos.Length(); i++)
        {
          (*state->_DO)[i] = (dos[i] == '1');
        }
      
      #ifdef SEQDEBUG
      // debug only display DO
      state->PrintDO();
      cout << endl;
      // end debug
      #endif

    }
  return 0;
}

int SeqXML_State::Parse_Loop(void* dataObj, TXMLNode* n) {
  SeqXML_State* s = (SeqXML_State*) dataObj;
  SQDG ("parsing Loop" << endl);
  if(n->HasAttributes())
    {
      TList* atts = n->GetAttributes();
      TXMLAttr* att;
      
      if((att = (TXMLAttr*)(atts->FindObject(ATT_count.String()))))
        {
          SQDG( " found count" << endl);
          s->_loopCnt = atoi(att->GetValue());
        }
      if((att = (TXMLAttr*)(atts->FindObject(ATT_isReturn.String()))))
        {
          SQDG( " found return" << endl);
          s->_isLoopReturn = (1 == atoi(att->GetValue()));
        }
    }
  else SQDG("loop has no attributes");
  SQDG("Loop count: " << s->_loopCnt << "  isReturn: " << s->_isLoopReturn << endl);
  return 0;
}

int SeqXML_State::Parse_AO(void* dataObj, TXMLNode* n) {
  SQDG ("parsing AO" << endl);

  SeqXML_State* state = (SeqXML_State*) dataObj;
  TXMLNode *Vn;
  if((Vn = FindNode(n, TAG_Vf.String()))) 
    {
      SQDG("parsing Vf" << endl);
      ParseVList(Vn->GetText(), state->_AOf, state->_thisSeq->getAOConfig()->getNumAOChns());
      
      #ifdef SEQDEBUG
      // debug purposes
      for(int i = 0; i < (int) state->_AOf->size(); i++) {
        cout << "V_" << i << ": " << (*(state->_AOf))[i] << endl;
      }
      // end debug
      #endif

    }

  if((Vn = FindNode(n, TAG_Vi.String()))) 
    {
      SQDG("parsing Vi" << endl);
      ParseVList(Vn->GetText(), state->_AOi, state->_thisSeq->getAOConfig()->getNumAOChns());
      
      #ifdef SEQDEBUG
      // debug purposes
      for(int i = 0; i < (int) state->_AOi->size(); i++) {
        cout << "V_" << i << ": " << (*(state->_AOi))[i] << endl;
      }
      // end debug
      #endif
    }

 
  return 0;
}

void SeqXML_State::ParseVList(TString Vs, vector<Double_t> *V, Int_t maxLvls)
{
  int i = 0;
  int cur_ind = 0;
  int cs = 0;
  int len = Vs.Length();
  
  while(i < len && cur_ind < maxLvls)
    {
      if ((Vs[i] == ',' && i>=1))  
        {


          TString ss = Vs(cs, i-1-cs);
          SQDG( "i: " << i << " cur_ind: " << cur_ind << " cs: " << cs << " len: " <<i-1-cs << " ss: " << ss.Data() << endl);
          (*V)[cur_ind] = ss.Atof();
          cs = i+1;
          i = i+2;
          cur_ind ++;
        }
      else if ( (i == len-1) && (i > cs)) 
        {
          TString ss = Vs(cs, i-cs);
          // this should be for the last level
          (*V)[cur_ind] = ss.Atof();
          break;
        }
      else
        i++;
    }
}

// accessor
Bool_t SeqXML_State::getDO(Int_t ind) {
  if(ind < _thisSeq->getSeqXML_DriverConsts()->getNumDOLines() && ind >=0)
    {
      return (*_DO)[ind];
    }
  else
    {
      cout << "Tried to access a DO line not configured for this sequence." << endl;
      assert(false);
    }
  return false;
}

void SeqXML_State::PrintDO() {
     for(int i = 0; i< (int) _DO->size(); i++)
        {
          if((*_DO)[i])
            {
              cout << "1";
            }
          else
            {
              cout <<"0";
            }
        }
}

void SeqXML_State::Print(Option_t* opt) 
{ 
  cout << "State " << _id <<
    " loopCnt: " << _loopCnt << 
    " cntPerIteration: " << _cntPerIteration << 
    " loopHead: " << _loopHead << 
    " comment: " << _comment->Data() <<
    " returnState: " << _returnState << 
    " cntPerIteration: " << _cntPerIteration <<
    " fullCnt: " << _fullCnt <<
    " _loopHead: " << _loopHead <<
    " _firstCnt: " << _firstCnt <<
    " _returnState: " << _returnState << endl;
}

// destructor
SeqXML_State::~SeqXML_State()
{
  if( _DO )
    delete _DO;
  if( _AOi )
    delete _AOi;
  if( _AOf )
    delete _AOf;
  if( _comment )
    delete _comment;

  SQDG( "deleting state" << _id << endl);
}

ClassImp(SeqXML_State)

/* END class SeqXML_State */
 
/* START Class SeqXML_ChainLink */
TObjString SeqXML_ChainLink::TAG_State = "State";
TObjString SeqXML_ChainLink::ATT_StateID = "id";
TObjString SeqXML_ChainLink::TAG_NumStates = "NumStates";
TObjString SeqXML_ChainLink::TAG_EventTable = "eventTable";
TObjString SeqXML_ChainLink::TAG_Event = "event";
TObjString SeqXML_ChainLink::TAG_NumEvents = "numEvents";

TMap* SeqXML_ChainLink::TagFunctionMap=NULL;
vector<parseFPtr> SeqXML_ChainLink::parseFunctions;

// constructor

SeqXML_ChainLink::SeqXML_ChainLink(SeqXML* seq, TXMLNode* n)
{
  if(!TagFunctionMap) {
    TagFunctionMap = new TMap(10,0);
    
    TagFunctionMap->Add(&TAG_State, new SeqInt(0));
    TagFunctionMap->Add(&TAG_NumStates, new SeqInt(1));
    TagFunctionMap->Add(&TAG_EventTable, new SeqInt(2));
    TagFunctionMap->Add(&TAG_Event, new SeqInt(3));
    TagFunctionMap->Add(&TAG_NumEvents, new SeqInt(4));
    
    parseFunctions.push_back(&SeqXML_ChainLink::Parse_State);
    parseFunctions.push_back(&SeqXML_ChainLink::Parse_NumStates);
    parseFunctions.push_back(&SeqXML_ChainLink::Parse_EventTable);
    parseFunctions.push_back(&SeqXML_ChainLink::Parse_Event);
    parseFunctions.push_back(&SeqXML_ChainLink::Parse_NumEvents);
  }

  _thisSeq = seq;

  // intialize state array.
  //AO: increased _states to 5000 from 2000 because this caused crash on run28341
  
  _states = new TObjArray(10000,0);

  // initialize event array
  //  _events = new TObjArray(50,0);
  // JTKM: increased _events to 400 from 200 because this caused crash on run34033
  _events = new TObjArray(1000,0);

  AutomaticParseList(this, n->GetChildren(), TagFunctionMap, parseFunctions);

  // Should be done getting the state array together. Put together count search
  // data

  _numEvents = _events->GetEntriesFast();
  _numStates = _states->GetEntriesFast();
  //  cout<<"_numEvents = "<<_numEvents<<endl;
  //  cout<<"_numStates = "<<_numStates<<endl;
  if (_numStates >10000) cout << "Warning Sequencer2.cxx: number of _states "<<_numStates<<" overflows array bound of 10000."<<endl;

  if(_numEvents != _parsedNumEvents) 
    {
      cout << "Error: parsed num does not agree with number of events!" << endl ;
      assert(false);
    }
  if(_numStates != _parsedNumStates) 
    {
      cout << "Error: parsed num does not agree with number of states!" << endl ;
      assert(false);
    }
  
  InitCntSearch();

  for(int i = 0; i<_numEvents; i++)
    {
      SeqXML_Event* e = (SeqXML_Event*)((*_events)[i]);
      if(e)
        e->_onCnt = getCountFromState(e->_onState);
    }
}

int SeqXML_ChainLink::Parse_State(void* dataObj, TXMLNode*n)
{
  SeqXML_ChainLink* thisChain = (SeqXML_ChainLink*) dataObj;
  SQDG("parsing a state" << endl);
  SeqXML_State* s;

  if((s = new SeqXML_State(thisChain->_thisSeq, n))) {
    TList* atts = n->GetAttributes();
    TXMLAttr* id_att;
    if((id_att = (TXMLAttr*)(atts->FindObject(ATT_StateID.String()))))
        {
          // found an id;
          Int_t ind = atoi(id_att->GetValue());
          thisChain->_states->AddAt((TObject*)s, ind);
        }
      else
        {
          cout << " Don't know where to insert this state! " << endl;
        }
  }
  return 0;
}

int SeqXML_ChainLink::Parse_EventTable(void* dataObj, TXMLNode* n)
{
  SQDG( "Parsing an event table" << endl);
  
  AutomaticParseList(dataObj, n->GetChildren(), TagFunctionMap, parseFunctions);

  return 0;
}

int SeqXML_ChainLink::Parse_Event(void* dataObj, TXMLNode* n)
{
  SeqXML_ChainLink* thisCL = (SeqXML_ChainLink*) dataObj;

  SeqXML_Event* event = new SeqXML_Event(thisCL->_thisSeq, n);
  thisCL->_events->AddAt((TObject*) event, event->GetID());
  SQDG("Parsed and added event " << event->GetID() << endl);

  return 0;
}


int SeqXML_ChainLink::Parse_NumEvents(void* dataObj, TXMLNode* n) {
  SeqXML_ChainLink* cl = (SeqXML_ChainLink*) dataObj;
  SQDG("Parsing num events node: " << n->GetText());
  cl->_parsedNumEvents = atoi(n->GetText());
  SQDG("ParsedNumEvents: " << cl->_parsedNumEvents << endl);
  return 0;
} 

int SeqXML_ChainLink::Parse_NumStates(void* dataObj, TXMLNode* n) {
  SeqXML_ChainLink* cl = (SeqXML_ChainLink*) dataObj;
  // SQDG("Parsing num state node: " << n->GetText());
  cl->_parsedNumStates = atoi(n->GetText());
  SQDG("ParsedNumStates: " << cl->_parsedNumStates << endl);
  return 0;
} 

void SeqXML_ChainLink::printEventTable()
{
 
}

void SeqXML_ChainLink::InitCntSearch()
{
  Int_t next_ind = 1;
  Int_t totalCnt = 0;

  SQDG2("Setting up search data" << endl);
  
  CalcCntsPerIteration(0, &next_ind, &totalCnt, 0,0);
  
  for(int i =0; i< _states->GetEntriesFast() ;i++)
    {
      SeqXML_State* s = (SeqXML_State*)((*_states)[i]);
      if(i==0)
        {

          s->_firstCnt = 1;
        }
      else
        {
          SeqXML_State* sm = (SeqXML_State*)((*_states)[i-1]);
          s->_firstCnt = sm->_firstCnt + sm->_fullCnt;         
        }
    }
 
  _totalCnt = totalCnt;
  SQDG2("Total Count for this chain: " << totalCnt << endl);
}

void SeqXML_ChainLink::CalcCntsPerIteration(Int_t ind, Int_t *next_ind, Int_t *current_Cnt, Int_t loopHead, Int_t loopDepth)
{
  static Int_t numCalls;

  numCalls ++;

  SQDG2(
        "ind: " << ind << 
        " next_ind: " << (*next_ind) << 
        " current_Cnt: " << (*current_Cnt) << 
        " loopHead: " << loopHead << 
        " loopDepth: " << loopDepth << 
        " numCalls: " << numCalls <<
        endl);

  if(ind >= 0 && ind < _states->GetEntriesFast()) 
    {
      SeqXML_State* s = (SeqXML_State*)((*_states)[ind]);
      
      if(s->_loopCnt > 1 && s->_isLoopReturn)
        {
          // loop and return on same line.
          
          (*current_Cnt) = (*current_Cnt) + s->_loopCnt;
          (*next_ind) = ind + 1;
          
          s->_cntPerIteration = 1;
          s->_fullCnt = s->_loopCnt;
          s->_loopHead = loopHead;
          s->_returnState = ind;
          
        }
      else if(s->_loopCnt > 1)
        {
          // head of a loop only.
          Int_t iterationCnt = 1;
          CalcCntsPerIteration(ind + 1, next_ind, &iterationCnt, ind, loopDepth + 1);
          
          (*current_Cnt) = (*current_Cnt) + iterationCnt;
          
          s->_cntPerIteration = iterationCnt;
          s->_fullCnt = iterationCnt * s->_loopCnt;
          s->_loopHead = loopHead;
          s->_returnState = (*next_ind)-1;
        }
      else if(s->_isLoopReturn)
        {
          // is a loop return only.

          (*current_Cnt) = (*current_Cnt) + 1;
          (*next_ind) = ind+1;

          s->_cntPerIteration = 1;
          s->_fullCnt = 1;
          s->_loopHead = loopHead;          
          s->_returnState = loopHead;

        }
      else
        {
          // this is just any state
          s->_loopHead = loopHead;
          s->_cntPerIteration = 1;
          s->_fullCnt = 1;
          s->_returnState = -1;

          (*current_Cnt) = (*current_Cnt) +1;

          while((*next_ind) >=0)
            {
              Int_t localNext = (*next_ind);
              (*next_ind) = (*next_ind+1);
              CalcCntsPerIteration(localNext, next_ind, current_Cnt, loopHead, loopDepth); 
            }
        }
    }
  else
    {
      // This is a stopping condition.
      *next_ind = -1;
      numCalls = 0;
    }
}

Bool_t SeqXML_ChainLink::FindState(Int_t* cnt, Int_t ind, Int_t *next_ind)
{
  SQDG2("Cnt: " << (*cnt) << " ind: " << ind << " next_ind: " << (*next_ind) << endl);

  if(ind >=0 && ind < _states->GetEntriesFast())
    {
      SeqXML_State* s = (SeqXML_State *)((*_states)[ind]);
      
      Int_t newCnt = (*cnt) - s->_fullCnt; 
      SQDG2("newCnt: " << newCnt << endl);

      if(newCnt == 0)
        {
          SQDG2("Found state" << endl);
          // We know we've found the state. Just pick the right one and exit.
          if(s->_loopCnt >1)
            {
              SQDG("in loop, therefore the cnt must be on the return line?" << endl);
              // We're in a loop. Zero in a loop means that we must be on the loop's last state.
              (*next_ind) = s->_returnState;
              return(true);
            }
          else
            {
              SQDG2("not in a loop. The state is just this one." << endl);
              // We're not in a loop. This is the state.
              (*next_ind) = ind;
              return(true);
            }
        }
      else
        {
          SQDG2("Not obvious that we have found state. Check what's going on" << endl);
          // not sure if we  found it.
          Int_t cur_ind = ind;
          if (newCnt>0)
            {
              // maybe at the next state or past th loop.
              SQDG2("New Cnt >0, so check if it's the next state or past a loop." << endl);

              if(s->_loopCnt >1)
                {
                  // was a loop. Jump to the next state after the loop's return.
                  SQDG2("This is a loop, so we must be counting past the loop. Jump to the next state after the loop's return." << endl);
                  (*next_ind) = s->_returnState + 1;
                }
              else
                {
                  SQDG2("Was not a loop, just go to the next line.");
                  // was not a loop. search on the next line.
                  (*next_ind) = cur_ind +1;
                }
              SQDG2("Update cnt to newCnt: cnt: " << (*cnt) << " newCnt: " << newCnt << endl);
              (*cnt) = newCnt;
              return(false);
            }
          else
            {
              SQDG2("New cnt was negative, so we must be in this loop" << endl);
              // we're in this loop. need to adjust cnt to get to right number of iterations.
              newCnt = (*cnt) % s->_cntPerIteration;
              SQDG2("Revise newCntL " << newCnt << endl);
              if(newCnt >1)
                {
                  SQDG2("newCnt > 1, so must be somewhere in loop, not on first or last line" << endl);
                  // not on the first or last line
                  (*cnt) = newCnt-1;
                  (*next_ind) = cur_ind +1;
                  SQDG2("update cnt: " << (*cnt) << " and next_ind " << (*next_ind) << endl);
                  return(false);
                }
              else
                {
                  SQDG2("On the first or last line of the loop" << endl);
                  // on the first or last line
                  if(newCnt == 1)
                    {
                      SQDG2("Cnt is 1, must be the first line" << endl);
                      // first line
                      (*cnt) = 0;
                      (*next_ind) = ind;
                      return(true);
                    }
                  else
                    {
                      SQDG2("Cnt is 0, must be the last line" << endl);
                      //last line
                      (*cnt) = 0;
                      (*next_ind) = s->_returnState;
                      return(true);
                    }
                }
              
            }
        }
    }
  else
    {
      SQDG2("End condition, or index out of range" << endl);
      // stop condition. Did not find jack.
      (*next_ind) = -1;
      return true;
    }
}

Int_t SeqXML_ChainLink::getStateFromCount(Int_t cnt)
{
  // We're going to find a state.
  Int_t next_ind = 0;
  Int_t search_ind = 0;
  Int_t searchCnt = cnt;
  Bool_t found = false;
  
  while(!found) 
    {
      next_ind = search_ind;
      found = FindState(&searchCnt, next_ind, &search_ind);
    }

  return(search_ind);
}

Int_t SeqXML_ChainLink::getCountFromState(Int_t state)
{
  return (((SeqXML_State*)((*_states)[state]))->_firstCnt);
}

SeqXML_ChainLink::~SeqXML_ChainLink()
{
  // This is the desctructor

  // destroy the states and events array.
  
  // delete the states array. This should also delete the states.
  //TIter ss(_states);
  //SeqXML_State* s;
  /*while((s = (SeqXML_State*) ss.Next()))
    {
      delete(s);
    }*/
  _events->SetOwner(kTRUE);
  _events->Delete();
  delete _events;
  _states->SetOwner(kTRUE);
  _states->Delete();
  delete _states;
  
}

ClassImp(SeqXML_ChainLink)
/* END class SeqXML_ChainLink */

/* Seq classes */
Seq::Seq() {

 // Empty constructor for Seq class
}
ClassImp(Seq)

/* SeqXML class */
TObjString SeqXML::TAG_SequenceHead = "Sequencer2XML";
TObjString SeqXML::TAG_SequencerName = "SequencerName";
TObjString SeqXML::TAG_Chain = "Chain";

// actually a child tag of Chain
TObjString SeqXML::TAG_ChainLink = "ChainLink";

TMap* SeqXML::TagFunctionMap=NULL;
vector<parseFPtr> SeqXML::parseFunctions;

SeqXML::SeqXML():driverConsts(0),_AOConfig(0),_chainLinks(0)
{ ;}

SeqXML::SeqXML(TXMLNode* head) {
  if(!_chainLinks) _chainLinks = new TObjArray(); 
  if(!TagFunctionMap) {
    TagFunctionMap = new TMap(10,0);
    
    TagFunctionMap->Add(&TAG_SequencerName, new SeqInt(0));
    TagFunctionMap->Add(&TAG_Chain, new SeqInt(1));
    
    parseFunctions.push_back(&SeqXML::Parse_SequencerName);
    parseFunctions.push_back(&SeqXML::Parse_Chain);
  }

  SQDG("trying to construct SeqXML with node: " << head->GetNodeName() << endl);

  if(head->GetNodeName() == TAG_SequenceHead.String())
    {
      // When parsing the main sequence, the driver version must be parsed first.
      TXMLNode* ndc = FindNode(head, SeqXML_DriverConsts::TAG_DriverConstsNode.String());
      if(ndc){
        driverConsts = new SeqXML_DriverConsts(ndc);
        if(driverConsts) {
          // We have intialized our constants. Parse the rest of this sequence.
          
          TXMLNode* acn = FindNode(head, SeqXML_AOConfig::TAG_AOConfig.String());
          _AOConfig = new SeqXML_AOConfig(this, acn);

          AutomaticParseList(this, head->GetChildren(), TagFunctionMap, parseFunctions);
          
          SQDG( "Done parsing the SeqXML." << endl);
 
        }
      }
      else {
        // no version tag. No good.
        cout << "ERROR: Couldn't find a sequencer version tag";
        // assert(NULL);
      }

    }
  else
    {
      // Error - this isn't a sequencer node
      cout << "This is not a Sequencer2XML object";
      // assert(NULL);
    }
}

int SeqXML::Parse_SequencerName(void* dataObj, TXMLNode* n)
{

  SeqXML* dc = (SeqXML*) dataObj;
  dc->_sequencerName = TString(n->GetText());
  SQDG( "parsed the sequencer name: " << dc->_sequencerName << endl);
  return 0;
}

int SeqXML::Parse_Chain(void* dataObj, TXMLNode* n)
{
  SeqXML* thisSeq = (SeqXML*) dataObj;

  SQDG( "parsing the chain entries" << endl);
  // intialize the ChainLinks array

  thisSeq->_chainLinks = new TObjArray(10,0);
  Int_t clCnt = 0;
  
  // parse the ChainLink children of node n

  // Currently ignores any other data items in 'Chain' node.

  TXMLNode* cn = n->GetChildren();
  while(cn) {
    SQDG("parsing node: " << cn->GetNodeName() << endl);
    if(cn->GetNodeName() == TAG_ChainLink.String())
      {
        // found a chain link
        SQDG( " Found a chain link." << endl);
        SeqXML_ChainLink* cl = new SeqXML_ChainLink(thisSeq, cn);
        if(cl) {
          SQDG ("added a chain link" << endl);
          thisSeq->_chainLinks->AddAt((TObject*)cl,clCnt); 
          clCnt ++;
        }
      }
    cn = cn->GetNextNode();

  }

  return 0;
}

SeqXML::~SeqXML() {
  SQDG("Deleting the sequence." << endl);
  if(driverConsts) delete(driverConsts);
  
  // delete the chain links
  if(_chainLinks)
    {
      TIter cls(_chainLinks);
      SeqXML_ChainLink* cl;
      while((cl = (SeqXML_ChainLink*)cls.Next()))
      {
        delete(cl);
      }
      delete(_chainLinks);
    }

  if(_AOConfig) delete(_AOConfig);
}

ClassImp(SeqXML)
 
/** End class SeqXML **/

TXMLNode* FindNode(TXMLNode*node,const char*name)
{
  for (; node != NULL; node = node->GetNextNode())
    {

      if (strcmp(node->GetNodeName(),name) == 0)
        return node;
      
      if (node->HasChildren())
        {
          TXMLNode* found = FindNode(node->GetChildren(),name);
          if (found)
            return found;
        }
    }
  
  return NULL;
}

SeqXML* testParse(TString filename)
{
  TDOMParser *domParser = new TDOMParser();
  domParser->SetValidate(false);
  SeqXML* mySeq;

  Int_t parsecode = domParser->ParseFile(filename);
  
  	   if (parsecode < 0 ) {
	     cerr << domParser->GetParseCodeMessage(parsecode) << endl;
	     return NULL;
	   }
	   
	   TXMLNode * node = domParser->GetXMLDocument()->GetRootNode();
       

       cout << "trying to construct a sequence" << endl;

       mySeq = new SeqXML(node);

       cout << "This has a SeqDriverConsts: " << *mySeq->getSeq_DriverConsts() << endl;


       TIter myChains((TObjArray*)mySeq->getChainLinks(), true);
       SeqXML_ChainLink *cl;
       while((cl = (SeqXML_ChainLink *) myChains.Next()))
         {
           SeqXML_State *state;
           TIter myStates(cl->getStates());
           while((state = (SeqXML_State *) myStates.Next())) {
             state->Print("");
           }
           
           SeqXML_Event *event;
           TIter myEvents(cl->getEvents());
           while((event = (SeqXML_Event *) myEvents.Next())) {
             event->Print("");
           }
         }
       
       return mySeq;
}

#ifdef TEST_ANALYSIS

int main(int argc, char* argv[])
{
  Bool_t testCount = false;

   cout << "argc = " << argc << endl;
   char* fname = NULL;
   for(int i = 0; i < argc; i++) {
     
     cout << "argv[" << i << "] = " << argv[i] << endl;
     if(!strcmp(argv[i],"-f") && (i+1) < argc) fname = argv[i+1];
     
     if(!strcmp(argv[i],"-c") )
     {
       // run the test counting program.
       testCount = true;
     }
   }

  cout << "start program" << endl;
  if(fname) 
    {
      ifstream f(fname);
      if(!f)
        {
          cout << "file '" << fname << "' does not exist" << endl;
        }
      else
        {
          f.close();
          cout << "Parsing file: " << fname << endl;
          SeqXML* seq;
          seq = testParse(fname);

          if(testCount && (seq)) 
            {
              Bool_t done = false;
              Bool_t getChainLink = true;
              Int_t numCls = seq->getChainLinks()->GetEntriesFast();
              Int_t selectedCl = 0;
              
              while (!done)
                {
                  TString input;
                  // do the test count
                  if(getChainLink)                     
                    {
                      cout << "Select a chain link (of " << numCls << ") ('Q' to quit):" << endl;
                    }
                  else
                    {
                      cout << "Set a count for chain link " << selectedCl << " ('Q' to quit):";
                    }
                  cin >>input;
                  if(!strcmp(input, "Q")) 
                    {
                      // quit. 
                      done = true;
                    }
                  else
                    {
                      // not a Q. 
                      if(input.IsFloat())
                        {
                          // is a number
                          Int_t num = input.Atoi();
                          if(getChainLink)
                            {
                              if(num >= 0 && num < numCls)
                                {
                                  selectedCl = num;
                                  getChainLink = false;
                                }
                              else
                                {
                                  cout << "Chain link not in range! " << endl;
                                }
                            }
                          else
                            {
                              if(num >0)
                                {
                                  // num is range valid (positive integer)
                                  // try to find the count
                                  
                                  Int_t theState;
                                  
                                  theState = ((SeqXML_ChainLink*)seq->getChainLinks()->At(selectedCl))->getStateFromCount(num);
                                  if(theState <0 )
                                    {
                                      cout << "Count " << num << " not found in chain link " << selectedCl << endl;
                                      
                                    }
                                  else
                                    {
                                      cout << "Count " << num << " is state " << theState << " of chain link " << selectedCl << endl;
                                    }

                                  getChainLink = true;
                                }
                              else
                                {
                                  cout << "Number is not a valid count: " << num << endl;
                                }

                            }
                        }
                      else
                        {
                          cout << "Enter a number to count, or 'Q' to quit." << endl;
                        }
                    }
                }
            }
        }
    }
  else
    {
      testParse("SomeAOManipulations.TL.pbar.xml");
    }
}
#endif
