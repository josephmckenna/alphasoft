#include <iostream>
#include <vector>
#include <map>
#include <cassert>

#include <TH1D.h>
#include <TH2D.h>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

class tdcmodule: public TARunObject
{
private:
  bool fTrace = true;

  std::vector<TH1D> fhCoarseTime_0;
  std::vector<TH1D> fhFineTime_0;
  std::vector<TH1D> fhFinalTime_0;

  std::vector<TH1D> fhCoarseTime_1;
  std::vector<TH1D> fhFineTime_1;
  std::vector<TH1D> fhFinalTime_1;

  TH1D* hNhits;
  TH1D* hNuniqHits;

  TH1D* hOcc_fpga2;
  TH1D* hOcc_fpga3;

  TH1D* hOcc;
  TH2D* hOcc_0;
  TH2D* hOcc_1;

  const unsigned ffpga[2]={2,3};
  const unsigned fNch=64;

  // https://daq.triumf.ca/elog-alphag/alphag/1961
  const double epoch_freq = 97656.25; // 200MHz/(2<<11);
  const double coarse_freq = 200.0e6; // 200MHz  
  
  // linear calibration:
  // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
  const double trb3LinearLowEnd = 17.0; 
  const double trb3LinearHighEnd = 473.0; 

public:

  tdcmodule(TARunInfo* runinfo): TARunObject(runinfo)
  {
    printf("tdcmodule::ctor!\n");
  }

  ~tdcmodule() 
  {
    printf("tdcmodule::dtor!\n");
  }
  
  void BeginRun(TARunInfo* runinfo)
  {
    runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
    TDirectory* dir = gDirectory->mkdir("tdc");
    dir->cd(); // select correct ROOT directory

    hNhits = new TH1D("hNhits","Number of Hits;# of Hits",100,0.,100.);
    hNuniqHits = new TH1D("hNuniqHits","Number of Unique Hits;# of Hits",100,0.,100.);
    
    hOcc = new TH1D("hOcc","Channels Occupancy",128,0.,128.);
    hOcc_fpga2 = new TH1D("hOcc_fpga2","Channels Occupancy FPGA 2",128,0.,128.);
    hOcc_fpga3 = new TH1D("hOcc_fpga3","Channels Occupancy FPGA 3",128,0.,128.);
  
    const int NbinsCoarseTime = 1000;
    const double MaxCoarseTime = 1.e4;     
    const int NbinsFineTime = 512;
    const double MaxFineTime = 512.;
    const int NbinsFinalTime = 10000;
    const double MaxFinalTime = 1.e7;

    hOcc_0 = new TH2D("hOcc_0","Channels Occupancy re0 and Final Time;Channel;Time [ps]",
		      128,0.,128.,1000,0.,MaxFinalTime);
    hOcc_1 = new TH2D("hOcc_1","Channels Occupancy re1 and Final Time;Channel;Time [ps]",
		      128,0.,128.,1000,0.,MaxFinalTime);
     
    for(unsigned f=0; f<2; ++f)
      {
	for(unsigned ic = 0; ic<fNch; ++ic)
	  {
	    TString hname=TString::Format("hCoarseTime_%d_%02d_0",ffpga[f],ic);
	    TString htitle=TString::Format("Coarse Time FPGA: %d Ch: %02d re0;Hit time [ns]",ffpga[f],ic);
	    fhCoarseTime_0.emplace_back(hname.Data(),htitle.Data(),NbinsCoarseTime,0.,MaxCoarseTime);

	    hname=TString::Format("hFineTime_%d_%02d_0",ffpga[f],ic);
	    htitle=TString::Format("Fine Time FPGA: %d Ch: %02d re0",ffpga[f],ic);
	    fhFineTime_0.emplace_back(hname.Data(),htitle.Data(),NbinsFineTime,0.,MaxFineTime); 

	    hname=TString::Format("hFinalTime_%d_%02d_0",ffpga[f],ic);
	    htitle=TString::Format("Final Time FPGA: %d Ch: %02d re0;Hit time [ps]",ffpga[f],ic);
	    fhFinalTime_0.emplace_back(hname.Data(),htitle.Data(),NbinsFinalTime,0.,MaxFinalTime);

	    hname=TString::Format("hCoarseTime_%d_%02d_1",ffpga[f],ic);
	    htitle=TString::Format("Coarse Time FPGA: %d Ch: %02d re1;Hit time [ns]",ffpga[f],ic);
	    fhCoarseTime_1.emplace_back(hname.Data(),htitle.Data(),NbinsCoarseTime,0.,MaxCoarseTime);

	    hname=TString::Format("hFineTime_%d_%02d_1",ffpga[f],ic);
	    htitle=TString::Format("Fine Time FPGA: %d Ch: %02d re1",ffpga[f],ic);
	    fhFineTime_1.emplace_back(hname.Data(),htitle.Data(),NbinsFineTime,0.,MaxFineTime); 

	    hname=TString::Format("hFinalTime_%d_%02d_1",ffpga[f],ic);
	    htitle=TString::Format("Final Time FPGA: %d Ch: %02d re1;Hit time [ps]",ffpga[f],ic);
	    fhFinalTime_1.emplace_back(hname.Data(),htitle.Data(),NbinsFinalTime,0.,MaxFinalTime);
	  }
      }
  }

  void EndRun(TARunInfo* runinfo)
  {
    printf("tdcmodule::EndRun, run %d\n", runinfo->fRunNo);
    runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
    gDirectory->cd("tdc");
    for( auto it = fhCoarseTime_0.begin(); it != fhCoarseTime_0.end(); ++it) it->Write();
    for( auto it = fhFineTime_0.begin(); it != fhFineTime_0.end(); ++it) it->Write();
    for( auto it = fhFinalTime_0.begin(); it != fhFinalTime_0.end(); ++it) it->Write();
    for( auto it = fhCoarseTime_1.begin(); it != fhCoarseTime_1.end(); ++it) it->Write();
    for( auto it = fhFineTime_1.begin(); it != fhFineTime_1.end(); ++it) it->Write();
    for( auto it = fhFinalTime_1.begin(); it != fhFinalTime_1.end(); ++it) it->Write();
  }

  void PauseRun(TARunInfo* runinfo)
  { }

  void ResumeRun(TARunInfo* runinfo)
  { }

  TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
    AgEventFlow *ef = flow->Find<AgEventFlow>();

    if (!ef || !ef->fEvent)
      return flow;

    AgEvent* age = ef->fEvent;

    TdcEvent* et = age->tdc;

    if( et )
      {
	if( et->complete )
	  {
	    // if( !et->error )
	    //   {
	    // 	std::cout<<"tdcmodule::AnalyzeFlowEvent  good TDC event"<<std::endl;
	    // 	FillHistos( et );
	    //   }
	    // else
	    //   std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event error"<<std::endl;
	    std::cout<<"tdcmodule::AnalyzeFlowEvent  good TDC event"<<std::endl;
	    FillHistos( et );
	  }
	else
	  std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event incomplete"<<std::endl;
      }
    else
      std::cout<<"tdcmodule::AnalyzeFlowEvent  No TDC event"<<std::endl;
      
    return flow;
  }

  void FillHistos(TdcEvent* evt)
  {
    std::vector<TdcHit*> hits = evt->hits; 
    std::map<int,double> CountsInChannel;
    int zero=0;
    for(auto it=hits.begin(); it!=hits.end(); ++it)
      {
	if( (*it)->fpga == 2 )
	  {
	    hOcc_fpga2->Fill( (*it)->chan );
	  }
	else if( (*it)->fpga == 3 )
	  {
	    hOcc_fpga3->Fill( (*it)->chan );
	  }
	int ch = Channel( (*it)->fpga, (*it)->chan );
	if( ch < 0 ) 
	  {
	    // std::cout<<ch<<std::endl;
	    ++zero;
	    continue;
	  }
	double coarse_time = GetCoarseTime((*it)->epoch,(*it)->coarse_time);
	double fine_time = double((*it)->fine_time);
	double final_time = GetFinalTime((*it)->coarse_time,fine_time);
	if( fTrace )
	std::cout<<"tdcmodule::FillHistos ch: "<<ch
		 <<" fpga: "<<int((*it)->fpga)<<" chan: "<<int((*it)->chan)
		 <<"  coarse time: "<<coarse_time
		 <<" ns  fine time: "<<fine_time<<" dc  final time: "<<final_time<<" ps"<<std::endl;
	if( (*it)->rising_edge )
	  {
	    ++CountsInChannel[ch];
	    fhCoarseTime_1.at(ch).Fill(coarse_time);
	    fhFineTime_1.at(ch).Fill(fine_time);
	    fhFinalTime_1.at(ch).Fill(final_time);

	    hOcc->Fill(ch);
	    hOcc_1->Fill(ch,final_time);
	  }
	else
	  {
	    fhCoarseTime_0.at(ch).Fill(coarse_time);
	    fhFineTime_0.at(ch).Fill(fine_time);
	    fhFinalTime_0.at(ch).Fill(final_time);
	    hOcc_0->Fill(ch,final_time);
	  }
      }
    hNuniqHits->Fill( double(CountsInChannel.size()) );
    double tot_hits=0.;
    for( auto h=CountsInChannel.begin(); h!=CountsInChannel.end(); ++h)
      {
	tot_hits+=h->second;
	// std::cout<<"tdcmodule::FillHistos  ch:"<<h->first
	// 	 <<"\t cnt: "<<h->second<<" ["<<tot_hits<<"]"<<std::endl;
      }
    hNhits->Fill( tot_hits );
    // std::cout<<"zero = "<<zero<<std::endl;
    // assert(zero<9);
  }

  int Channel(uint8_t& fpga, uint8_t& chan)
  {
    if( chan == 0 )
      return -1;
    if( fpga == 2 )
      return int(chan) - 1;
    else if( fpga == 3 )
      return int(chan) - 1 + int(fNch);
    else
      {
	//	std::cout<<int(fpga)<<std::endl;
	return -2;
      }
  }

  double GetCoarseTime( uint32_t epoch, uint16_t coarse )
  {
    return double(epoch)/epoch_freq + double(coarse)/coarse_freq;
  }

  double GetFinalTime( uint16_t coarse, double fine )
  {
    double A = double(coarse) * 5000.,
      B = fine - trb3LinearLowEnd,
      C = trb3LinearHighEnd - trb3LinearLowEnd;
    return A - (B/C) * 5000.;
  }

  void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
  {
    printf("tdcmodule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
  }
  
  
};

class TdcModuleFactory: public TAFactory
{
public:
  void Init(const std::vector<std::string> &args)
  {
    printf("TdcModuleFactory::Init!\n");
      
    for (unsigned i=0; i<args.size(); i++) { }
  }

  void Finish()
  {
    printf("TdcModuleFactory::Finish!\n");
  }

  TARunObject* NewRunObject(TARunInfo* runinfo)
  {
    printf("TdcModule::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
    return new tdcmodule(runinfo);
  }
};

static TARegister tar(new TdcModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
