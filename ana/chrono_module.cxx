// 
// chronobox 
// 
// A. Capra
//

#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"

#include <iostream>

class ChronoFlags
{
public:
   bool fPrint = false;
};

class Chrono: public TARunObject
{
private:

public:
  ChronoFlags* fFlags;
  bool fTrace = true;
   
   Chrono(TARunInfo* runinfo, ChronoFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("Chrono::ctor!\n");
   }

   ~Chrono()
   {
      if (fTrace)
         printf("Chrono::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
	printf("Chrono::BeginRun, run %d\n", runinfo->fRunNo);
	//printf("Chrono::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Chrono::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Chrono::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      std::cout<<"Chrono::Analyze   Event # "<<me->serial_number<<std::endl;

      if( me->event_id != 10 ) // sequencer event id
      	return flow;
      me->FindAllBanks();
      std::cout<<"===================================="<<std::endl;
      std::cout<<me->HeaderToString()<<std::endl;
      std::cout<<me->BankListToString()<<std::endl;
      const TMBank* b = me->FindBank("CBS1");

      if( b ) std::cout<<"Chrono::Analyze   BANK NAME: "<<b->name<<std::endl;
      else return flow;
      uint32_t *pdata32;
      pdata32= (uint32_t*)me->GetBankData(b);
      //const char* bkptr = me->GetBankData(b);
      int bklen = b->data_size;
      std::cout<<"bank size: "<<bklen<<std::endl;
      if( bklen > 0 )
	//printf("%s\n",bkptr);
	//for (Int_t dave=0; dave<64; dave++)
	//std::cout<<pdata32[dave]<<std::endl;
	std::cout<<pdata32[58]<<std::endl;
      std::cout<<"________________________________________________"<<std::endl;
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("Chrono::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class ChronoFactory: public TAFactory
{
public:
   ChronoFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("ChronoFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("ChronoFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("ChronoFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new Chrono(runinfo, &fFlags);
   }
};

static TARegister tar(new ChronoFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

