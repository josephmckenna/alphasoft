#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <signal.h>

#include "tmfe.h"
#include "tmvodb.h"

#include "midas.h"

#include "EsperComm.h"

class BVlvdb: public TMFeRpcHandlerInterface
{
private:
  TMFE* fFe=0;
  TMFeEquipment* fEq=0;
  TMVOdb* fS=0; // Settings
  TMVOdb* fV=0; // Variables
  TMVOdb* fR=0; // Readback

  Esper::EsperComm* fEsper=0;
  std::vector<std::string> fModules;
  

  std::vector<double> fDemandVoltage;
  std::vector<double> fSenseVoltage;
  std::vector<double> fSenseTemperature;
  std::vector<double> fDemandDAC;
  std::vector<double> fSenseDAC; 

public:

  time_t fFastUpdate = 0;
  int  fReadPeriodSec = 15;

  BVlvdb(TMFE* fe, TMFeEquipment* eq, Esper::EsperComm* esp):fFe(fe),fEq(eq),fEsper(esp)
  {
    fS = fEq->fOdbEqSettings;
    fV = fEq->fOdbEqVariables;
    fR = fEq->fOdbEq->Chdir("Readback", true);
  }

  ~BVlvdb()
  {
    // if(fV) delete fV;
    // if(fR) delete fR;
    // if(fS) delete fS;
    if(fEq) delete fEq;
    //if(fFe) delete fFe;
    if(fEsper) delete fEsper;
  }

  TMFeEquipment* GetEquipment() {return fEq;}

  void ReadModules()
  {
    fEsper->Open();
    KOtcpError e = fEsper->GetModules( &fModules );
    fEsper->Close();
    if( e.error )
      {
	std::cerr<<e.message<<std::endl;
	fEq->SetStatus(e.message.c_str(),"red");
	return;
      }
  }
  
  void ReadVariables(Esper::EsperNodeData* data)
  {
    fEsper->fVerbose=true;
    for( unsigned i=0; i<fModules.size(); ++i ) 
      {
	fEsper->Open();
	std::cout<<"BVlvdb::ReadVariables modules: "<<fModules[i]<<std::endl;
	KOtcpError e = fEsper->ReadVariables(fR, fModules[i], &(*data)[fModules[i]]);
	fEsper->Close();
	if( e.error )
	  {
	    std::cerr<<e.message<<std::endl;
	    fEq->SetStatus(e.message.c_str(),"red");
	    return;
	  }
      }
  }
  
  void ReadBack()
  { 
    ReadModules();
    Esper::EsperNodeData data;
    ReadVariables(&data);
    
    fDemandVoltage=data["board"].da["bias_voltage_setpoint"];
    fS->RDA("bias_voltage_setpoint",&fDemandVoltage,true,fDemandVoltage.size());

    fDemandDAC=data["board"].da["asd_dac_setpoint"];
    fS->RDA("asd_dac_setpoint",&fDemandDAC,true,fDemandDAC.size());
  }

  bool ReadVariables()
  {
    Esper::EsperNodeData data;
    fEsper->fVerbose=false;
    fEsper->Open();
    KOtcpError e = fEsper->ReadVariables(fR, "board", &(data)["board"]);
    fEsper->Close();
    if( e.error )
      {
	std::cerr<<e.message<<std::endl;
	fEq->SetStatus(e.message.c_str(),"red");
	return false;
      }

    fSenseVoltage=data["board"].da["bias_voltage"];
    fV->WDA("bias_voltage",fSenseVoltage);

    fSenseTemperature=data["board"].da["asd_temp"];
    fV->WDA("asd_temp",fSenseTemperature);

    double max_temp = *std::max_element(fSenseTemperature.begin(), fSenseTemperature.end());
    if( max_temp > 30. )
      {
	// std::string msg("Temperature running high: ");
	// msg += std::to_string(max_temp);
	// msg += " degC";
	// fEq->SetStatus( msg.c_str() , "#F1C40F");
	char msg[64];
	sprintf(msg,"Temperature running high: %1.2f degC",max_temp);
	fEq->SetStatus( msg, "#F1C40F");
	return false;
      }
    else if( max_temp < 0. )
      {
	fEq->SetStatus( "ASD off", "white");
	return false;
      }

    fSenseDAC=data["board"].da["asd_dac"];
    fV->WDA("asd_dac",fSenseDAC);

    return true;
  }

  bool WriteSettings()
  {
    fEsper->fVerbose=false;

    fS->RDA("bias_voltage_setpoint",&fDemandVoltage,false,fDemandVoltage.size());
    std::string json("[,");
    for(auto it = fDemandVoltage.begin(); it != fDemandVoltage.end(); ++it)
      {
	json += std::to_string(*it);
	json += ",";
      }
    json += "]";
    fEsper->Open();
    fEsper->Write("board","bias_voltage_setpoint",json.c_str());
    fEsper->Close();


    fS->RDA("asd_dac_setpoint",&fDemandDAC,false,fDemandDAC.size());  
    json = "[,";
    for(auto it = fDemandDAC.begin(); it != fDemandDAC.end(); ++it)
      {
	json += std::to_string(*it);
	json += ",";
      }
    json += "]";
    fEsper->Open();
    fEsper->Write("board","asd_dac_setpoint",json.c_str());  
    fEsper->Close();

    return true;
  }

  std::string HandleRpc(const char* cmd, const char* args)
  {
    fFe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
    if( std::string(cmd) == "read") 
      {
	ReadBack();
      }
    return "OK";
  }
};



int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   std::string end("nowhere");
   const char* name = argv[1];
   if( strcmp(name, "lvdb03")==0 )
     {
       end="top";
     }
   else if( strcmp(name, "lvdb06")==0 )
     {
       end="bot";
     }
   else
     {
       std::cerr<<"No esper server"<<std::endl;
       return 1;
     }
   
   std::cout<<"fe for "<<name<<" at "<<end<<std::endl;
     
   TMFE* mfe = TMFE::Instance();

   std::string fename("febv");
   fename+=name;
   
   TMFeError err = mfe->Connect(fename.c_str());
   if( err.error )
     {
       std::cerr<<"Cannot connect, bye."<<std::endl;
       return 1;
     }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 3;
   eqc->FrontendName = std::string(fename.c_str());
   eqc->LogHistory = 1;
   eqc->FrontendHost = "alphagdaq.cern.ch";
   eqc->FrontendFileName = std::string(__FILE__);

   std::string eqname("BVlv");
   eqname+=end;
   TMFeEquipment* eq = new TMFeEquipment(eqname.c_str());
   eq->Init(mfe->fOdbRoot, eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   Esper::EsperComm* esper = new Esper::EsperComm(name);

   BVlvdb* bv = new BVlvdb(mfe,eq,esper);

   //  bv->UpdateSettings();

   mfe->RegisterRpcHandler(bv);
   //   mfe->SetTransitionSequence(-1, -1, -1, -1);

   bv->ReadBack();
   mfe->Msg(MINFO, eqc->FrontendName.c_str(), "started");


   while (!mfe->fShutdown) 
     {
       bool statr = bv->ReadVariables();
       bool statw = bv->WriteSettings();

       if( statr && statw )
	 bv->GetEquipment()->SetStatus("OK","#00FF00");

       if (bv->fFastUpdate != 0) 
	 {
	   if (time(NULL) > bv->fFastUpdate)
	     bv->fFastUpdate = 0;
	 }
       
      if (bv->fFastUpdate) 
	{
	 //mfe->Msg(MINFO, "main", "fast update!");
	  mfe->PollMidas(1000);
	 if (mfe->fShutdown)
	    break;
         } 
      else 
	{
	  for (int i=0; i<bv->fReadPeriodSec; i++) 
	    {
	      mfe->PollMidas(1000);
	      if (mfe->fShutdown)
		break;
            }
	  if (mfe->fShutdown)
	    break;
	}
     }

   mfe->Disconnect();

   //   delete bv;
   delete esper;
   return 0;
}
