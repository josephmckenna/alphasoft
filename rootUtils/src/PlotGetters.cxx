#include "PlotGetters.h"

extern Int_t gNbin;
//Plots

#ifdef BUILD_AG
void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TH1D* h=Get_Chrono( runNumber, Chronoboard, ChronoChannel, tmin, tmax);
  h->GetXaxis()->SetTitle("Time [s]");
  h->GetYaxis()->SetTitle("Counts");
  h->Draw();
  return;  
} 
#endif
#ifdef BUILD_AG
void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  return Plot_Chrono(runNumber, Chronoboard, ChronoChannel, tmin, tmax);
}
#endif
#ifdef BUILD_AG
void Plot_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TH1D* h=Get_Chrono( runNumber, ChannelName, tmin, tmax);  
  h->GetXaxis()->SetTitle("Time [s]");
  h->GetYaxis()->SetTitle("Counts");
  TString cname = TString::Format("c%s_%1.3f-%1.3f",ChannelName,tmin,tmax);
  TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1800, 1000);
  c->cd();
  h->Draw();
  return;  
} 
#endif
#ifdef BUILD_AG
void Plot_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  return Plot_Chrono(runNumber, ChannelName, tmin, tmax);
}
#endif
#ifdef BUILD_AG
void Plot_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TH1D* h=Get_Delta_Chrono( runNumber, Chronoboard, ChronoChannel, tmin, tmax);  
  h->GetXaxis()->SetTitle("Time [s]");
  h->GetYaxis()->SetTitle("Counts");
  h->Draw();
  return;  
} 
#endif
#ifdef BUILD_AG
void Plot_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  return Plot_Delta_Chrono(runNumber, Chronoboard, ChronoChannel, tmin, tmax);
}
#endif
#ifdef BUILD_AG
void Plot_Delta_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TH1D* h=Get_Delta_Chrono( runNumber, ChannelName, tmin, tmax);
  h->Draw();
  return;  
} 
#endif
#ifdef BUILD_AG
void Plot_Delta_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  return Plot_Delta_Chrono(runNumber, ChannelName, tmin, tmax);
}
#endif
#ifdef BUILD_AG
void PlotChronoScintillators(Int_t runNumber, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);

  std::vector<std::string> channels {"SiPM_A","SiPM_D","SiPM_A_AND_D","SiPM_E",
      "SiPM_C","SiPM_F","SiPM_C_AND_F","SiPM_B"};
  TString cname = TString::Format("cSiPM_%1.3f-%1.3f",tmin,tmax);
  TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1800, 1500);
  c->Divide(2,4);
  int i=0;
  for(auto it = channels.begin(); it!=channels.end(); ++it)
    {
      TH1D* h=Get_Chrono( runNumber, it->c_str(), tmin, tmax);  
      h->GetXaxis()->SetTitle("Time [s]");
      h->GetYaxis()->SetTitle("Counts"); 
      c->cd(++i);
      h->Draw();
    }
  return;
}
#endif
#ifdef BUILD_AG
void PlotChronoScintillators(Int_t runNumber, const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  return PlotChronoScintillators(runNumber, tmin, tmax);
}
#endif
#ifdef BUILD_AG
void Plot_TPC(Int_t runNumber,  Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TAGPlot* p=new TAGPlot(0); //Cuts off
  p->SetTimeRange(0.,tmax-tmin);
  p->AddEvents(runNumber,tmin,tmax);
  TString cname = TString::Format("cVTX_R%d",runNumber);
  std::cout<<"NVerts:"<<p->GetTotalVertices()<<std::endl;
  p->Canvas(cname);
}
#endif
#ifdef BUILD_AG
void Plot_TPC(Int_t runNumber,  const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  std::cout<<"Dump at ["<<tmin<<","<<tmax<<"] s   duration: "<<tmax-tmin<<" s"<<std::endl;
  double ttmin = GetTrigTimeBefore(runNumber,tmin),
    ttmax = GetTrigTimeAfter(runNumber,tmax);
  std::cout<<"Trigger window ["<<ttmin<<","<<ttmax<<"] s   duration:"<<ttmax-ttmin<<" s"<<std::endl;
  return Plot_TPC(runNumber,tmin,tmax);
}
#endif
#ifdef BUILD_AG
void Plot_TPC(Int_t* runNumber, Int_t Nruns, const char* description, Int_t repetition, Int_t offset)
{ 
  TAGPlot* p=new TAGPlot(0); //Cuts off  
  for( Int_t i=0; i<Nruns; ++i )
    {
      std::cout<<"Run"<<runNumber[i]<<std::endl;
      Double_t tmin=MatchEventToTime(runNumber[i], description,true,repetition, offset);
      Double_t tmax=MatchEventToTime(runNumber[i], description,false,repetition, offset);
      std::cout<<"Dump at ["<<tmin<<","<<tmax<<"] s   duration: "<<tmax-tmin<<" s"<<std::endl;
      double ttmin = GetTrigTimeBefore(runNumber[i],tmin),
      ttmax = GetTrigTimeAfter(runNumber[i],tmax);
      std::cout<<"Trigger window ["<<ttmin<<","<<ttmax<<"] s   duration:"<<ttmax-ttmin<<" s"<<std::endl;
      p->SetTimeRange(0.,tmax-tmin);
      p->AddEvents(runNumber[i],tmin,tmax);
    }
  TString cname = TString::Format("cVTX_%s_Rlist",description);
  p->Canvas(cname);
  return;
}
#endif
#ifdef BUILD_AG
void Plot_Vertices_And_Tracks(Int_t runNumber, double tmin, double tmax)
{
  TAGPlot* p=new TAGPlot(0); //Cuts off  
  p->SetPlotTracks();
  int total_number_events = p->AddEvents(runNumber,tmin,tmax);

  int total_number_vertices = p->GetTotalVertices();
  double total_runtime = p->GetTotalTime();

  TString cname = TString::Format("cVTX_%1.1f-%1.1f_R%d",tmin,tmax,runNumber);
  //  std::cout<<cname<<std::endl;
  p->Canvas(cname);

  cname = TString::Format("cHEL_%1.1f-%1.1f_R%d",tmin,tmax,runNumber);
  //  std::cout<<cname<<std::endl;
  p->DrawTrackHisto(cname.Data());

  std::cout<<"Total Number of Events: "<<total_number_events<<std::endl;
  std::cout<<"Total Number of Vertices: "<<total_number_vertices<<std::endl;
  std::cout<<"Total Runtime: "<<total_runtime<<std::endl;

  return;
} 
#endif
#ifdef BUILD_AG
void Plot_Vertices_And_Tracks(Int_t runNumber, const char* description, 
			      Int_t repetition, Int_t offset)
{ 
  Int_t runList[]={runNumber};
  Int_t Nruns = 1;
  return Plot_Vertices_And_Tracks( runList, Nruns, description, 
				   repetition, offset);
}
#endif
#ifdef BUILD_AG
void Plot_Vertices_And_Tracks(Int_t* runNumber, Int_t Nruns, const char* description, 
			      Int_t repetition, Int_t offset)
{ 
  TAGPlot* p=new TAGPlot(0); //Cuts off  
  p->SetPlotTracks();
  //  p->SetVerbose(true);
  int total_number_events=0;
  bool whole=false;
  double duration=0.;
  for( Int_t i=0; i<Nruns; ++i )
    {
      std::cout<<"Run"<<runNumber[i]<<std::endl;
      TString dump(description);
      Double_t tmin,tmax;
      if( dump.BeginsWith("all", TString::kIgnoreCase))
	{
	  tmin=0.;
	  tmax=GetAGTotalRunTime(runNumber[i]);
	  whole=true;
	  duration = duration>tmax?duration:tmax;
	}
      else
	{
	  tmin=MatchEventToTime(runNumber[i], description,true,repetition, offset);
	  tmax=MatchEventToTime(runNumber[i], description,false,repetition, offset);
	}
      std::cout<<"Dump at ["<<tmin<<","<<tmax<<"] s   duration: "<<tmax-tmin<<" s"<<std::endl;
      double ttmin = GetTrigTimeBefore(runNumber[i],tmin),
      ttmax = GetTrigTimeAfter(runNumber[i],tmax);
      std::cout<<"Trigger window ["<<ttmin<<","<<ttmax<<"] s   duration:"<<ttmax-ttmin<<" s"<<std::endl;
      total_number_events+=p->AddEvents(runNumber[i],tmin,tmax);
    }
  if( whole )
    p->SetTimeRange(0.,duration);

  int total_number_vertices = p->GetTotalVertices();
  double total_runtime = p->GetTotalTime();

  TString cnamev,cnamet;
  if(Nruns == 1 )
    {
      cnamev = TString::Format("cVTX_%s_R%d",description,runNumber[0]);
      cnamet = TString::Format("cHEL_%s_R%d",description,runNumber[0]);
    }
  else
    {
      cnamev = TString::Format("cVTX_%s_Rlist",description);
      cnamet = TString::Format("cHEL_%s_Rlist",description);
    }

  //  std::cout<<cnamev<<std::endl;
  p->Canvas(cnamev);
  //  std::cout<<cnamet<<std::endl;
  p->DrawTrackHisto(cnamet.Data());

  std::cout<<"Total Number of Events: "<<total_number_events<<std::endl;
  std::cout<<"Total Number of Vertices: "<<total_number_vertices<<std::endl;
  std::cout<<"Total Runtime: "<<total_runtime<<std::endl;

  return;
}
#endif
#ifdef BUILD_AG
void Plot_ClockDrift_TPC(Int_t runNumber, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TCanvas* c=new TCanvas("ClockDrift","ClockDrift",1200,800);
  c->Divide(1,3);
  c->cd(1);
  Get_TPC_EventTime_vs_OfficialTime(runNumber, tmin, tmax)->Draw();
  c->cd(2);
  Get_TPC_EventTime_vs_OfficialTime_Drift(runNumber,tmin,tmax)->Draw();
  c->cd(3);
  Get_TPC_EventTime_vs_OfficialTime_Matching(runNumber,tmin,tmax)->Draw();
  c->Draw();
}
#endif
#ifdef BUILD_AG
void Plot_ClockDrift_Chrono(Int_t runNumber, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TCanvas* c=new TCanvas("ChronoClockDrift","ChronoClockDrift",1200,800);
  c->Divide(CHRONO_N_BOARDS,3);
  for (int i=0; i<CHRONO_N_BOARDS; i++)
    {
      c->cd(1 + i);
      Get_Chrono_EventTime_vs_OfficialTime(runNumber, i, tmin, tmax)->Draw();
      c->cd(3 + i);
      Get_Chrono_EventTime_vs_OfficialTime_Drift(runNumber, i, tmin, tmax)->Draw();
      c->cd(5 + i);
      Get_Chrono_EventTime_vs_OfficialTime_Matching(runNumber, i, tmin, tmax)->Draw();
    }
  c->Draw();
}
#endif
#ifdef BUILD_AG
void Plot_Chrono_Sync(Int_t runNumber, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TCanvas* c=new TCanvas("ChronoClockSync","ChronoClockSync",1200,800);
  c->Divide(CHRONO_N_BOARDS,2);
  for (int i=0; i<CHRONO_N_BOARDS; i++)
    {
      c->cd(1 + (i*2));
      Plot_Chrono(runNumber,i,Get_Chrono_Channel(runNumber,i,"CHRONO_SYNC",false), tmin,tmax);
      c->cd(2 + (i*2));
      Plot_Delta_Chrono(runNumber,i,Get_Chrono_Channel(runNumber,i,"CHRONO_SYNC",false), tmin,tmax);
    }
  c->Draw();
}
#endif
//*************************************************************
// Energy Analysis
//*************************************************************
TCanvas* gc;
TH1D* gh;
#ifdef BUILD_AG
TCanvas* Plot_AG_CT_ColdDump(Int_t runNumber,Int_t binNumber, const char* dumpFile, Double_t EnergyRangeFactor)
{  
  Double_t start_time=MatchEventToTime(runNumber, "Cold Dump",true,1,0);
  Double_t stop_time=MatchEventToTime(runNumber, "Cold Dump",false,1,0);
  if (stop_time<0.) stop_time=GetAGTotalRunTime(runNumber);

  Double_t startOffset = 0.002; // dump starts two milliseconds after the start dump trigger
  Double_t stopOffset = 0.; // dump finishes at trigger
  
  Double_t dumpDuration = stop_time-start_time-startOffset-stopOffset;
  
  std::cout <<"Dump start: "<< start_time-startOffset << " Dump stop: " << stop_time-stopOffset << std::endl;
  std::cout<<"Dump Duration "<<"\t"<<dumpDuration<<" s"<<std::endl;

  Int_t oldBinNumber = gNbin;
  gNbin=1.e4;
  TH1D* dumpHisto=Get_Chrono( runNumber, "CATCH_OR", start_time, stop_time );
  gNbin=oldBinNumber;
 
  if(!dumpHisto){Error("PlotEnergyDump","NO CB counts plot"); return 0;}
  // and the voltage ramp function of time
  TSpline5* dumpRamp = InterpolateVoltageRamp(dumpFile);
  if(!dumpRamp){Error("PlotEnergyDump","NO voltage ramp function"); return 0;}
    TH1D* dumpHisto_toPlot =Get_Chrono(runNumber,"CATCH_OR",start_time+startOffset,stop_time); // this is a lower resolution histo to plot

  TString htitle = "Run ";
  htitle+=runNumber;
  htitle+=" : #bar{p} energy  Cold Dump;Energy [eV];counts";

  // energy (temperature) histogram
  Double_t RampTmin=dumpRamp->GetXmin();
  Double_t RampTmax=dumpRamp->GetXmax();
  if ( RampTmax<0.) {Error("PlotEnergyDump","Ramp invalid? Can't work out how long it was"); return 0; }
  Double_t Emin=dumpRamp->Eval(RampTmax), Emax=dumpRamp->Eval(RampTmin);
  TH1D* hEnergy = new TH1D("pbar_temperature", htitle.Data(), binNumber, Emin, Emax);
  hEnergy->SetMarkerColor(kRed);
  hEnergy->SetMarkerStyle(7);
  hEnergy->SetLineColor(kBlack);

  // calculate the energy resolution
  Double_t res = (Emax-Emin)/ (Double_t) gNbin;
  char resolution[80];
  sprintf(resolution,"Energy Resolution %.1lf meV ",res*1.e3);
  printf(resolution,"Energy Resolution %.1lf meV\n",res*1.e3);
  // map time to energy
  Double_t dt,energy,energy_max=0.;
  Int_t counts=0.;
  for(Int_t b=1; b<=dumpHisto->GetNbinsX(); ++b)
    {
      dt = (dumpHisto->GetBinCenter(b)/*-start_time*/-startOffset);//(dumpDuration);
      energy = dumpRamp->Eval(dt);
      //~ //if(energy<res) continue;
      counts = dumpHisto->GetBinContent(b);
      if(energy>energy_max && counts>10) energy_max=energy; // interesting bins only
#if DEBUG > 1
      std::cout<<b<<"\t"<<dt<<"\t"<<energy<<"\t"<<dumpHisto->GetBinContent(b)<<std::endl;
#endif
      hEnergy->Fill(energy, counts);

    }

  //hEnergy->GetXaxis()->SetRangeUser(Emin,energy_max);

  // calculate the total number of counts in the intresting range
  Int_t binmin = hEnergy->FindBin(Emin),
    binmax = hEnergy->FindBin(energy_max);
  Double_t total_counts = hEnergy->Integral(binmin,binmax);
  char integral[80];
  sprintf(integral,"Integral %1.01f",total_counts);

  // information
  TPaveText* tt = new TPaveText(0.1,0.5,0.9,0.9);
  tt->SetFillColor(0);
  tt->AddText(resolution);
  tt->AddText(integral);

  // plotting and wait for fitting
  //  if (gLegendDetail>=1)
  //    {
      gStyle->SetOptStat(1011111);
      //    }
      //  else
      //    {
      //      gStyle->SetOptStat("ni");
      //    }
  delete gc;
  TCanvas* cEnergy = new TCanvas("AntiprotonTemperature","AntiprotonTemperature",1800,1000);
  cEnergy->Divide(2,2);
  cEnergy->cd(1);
  gPad->SetLogy(1);
  dumpHisto_toPlot->Draw("HIST");
  cEnergy->cd(2);
  
  hEnergy->GetXaxis()->SetRangeUser(0,EnergyRangeFactor*(hEnergy->GetMean()));
  hEnergy->Draw("E1 HIST");
  gPad->SetLogy(1);
  cEnergy->cd(3);
  dumpRamp->Draw();

  cEnergy->cd(4);
  tt->Draw();

  //necessary for fitting
  gc=cEnergy;
  gh=hEnergy;
  return cEnergy;
}
#endif
#ifdef BUILD_AG
TCanvas* Plot_AG_RCT_ColdDump(Int_t runNumber,Int_t binNumber, const char* dumpFile, Double_t EnergyRangeFactor)
{  
  Double_t start_time=MatchEventToTime(runNumber, "Cold Dump",true,1,0);
  Double_t stop_time=MatchEventToTime(runNumber, "Cold Dump",false,1,0);
  if (stop_time<0.) stop_time=GetAGTotalRunTime(runNumber);

  Double_t startOffset = 0.002; // dump starts two milliseconds after the start dump trigger
  Double_t stopOffset = 0.; // dump finishes at trigger
  
  Double_t dumpDuration = stop_time-start_time-startOffset-stopOffset;
  
  std::cout <<"Dump start: "<< start_time-startOffset << " Dump stop: " << stop_time-stopOffset << std::endl;
  std::cout<<"Dump Duration "<<"\t"<<dumpDuration<<" s"<<std::endl;

  Int_t oldBinNumber = gNbin;
  gNbin=1.e4;
  TH1D* dumpHisto=Get_Chrono( runNumber, "SiPM_B", start_time, stop_time );
  gNbin=oldBinNumber;
 
  if(!dumpHisto){Error("PlotEnergyDump","NO CB counts plot"); return 0;}
  // and the voltage ramp function of time
  TSpline5* dumpRamp = InterpolateVoltageRamp(dumpFile);
  if(!dumpRamp){Error("PlotEnergyDump","NO voltage ramp function"); return 0;}
    TH1D* dumpHisto_toPlot =Get_Chrono(runNumber,"SiPM_B",start_time+startOffset,stop_time); // this is a lower resolution histo to plot

  TString htitle = "Run ";
  htitle+=runNumber;
  htitle+=" : #bar{p} energy  Cold Dump;Energy [eV];counts";

  // energy (temperature) histogram
  Double_t RampTmin=dumpRamp->GetXmin();
  Double_t RampTmax=dumpRamp->GetXmax();
  if ( RampTmax<0.) {Error("PlotEnergyDump","Ramp invalid? Can't work out how long it was"); return 0; }
  Double_t Emin=dumpRamp->Eval(RampTmax), Emax=dumpRamp->Eval(RampTmin);
  TH1D* hEnergy = new TH1D("pbar_temperature", htitle.Data(), binNumber, Emin, Emax);
  hEnergy->SetMarkerColor(kRed);
  hEnergy->SetMarkerStyle(7);
  hEnergy->SetLineColor(kBlack);

  // calculate the energy resolution
  Double_t res = (Emax-Emin)/ (Double_t) gNbin;
  char resolution[80];
  sprintf(resolution,"Energy Resolution %.1lf meV ",res*1.e3);
  printf(resolution,"Energy Resolution %.1lf meV\n",res*1.e3);
  // map time to energy
  Double_t dt,energy,energy_max=0.;
  Int_t counts=0.;
  for(Int_t b=1; b<=dumpHisto->GetNbinsX(); ++b)
    {
      dt = (dumpHisto->GetBinCenter(b)/*-start_time*/-startOffset);//(dumpDuration);
      energy = dumpRamp->Eval(dt);
      //~ //if(energy<res) continue;
      counts = dumpHisto->GetBinContent(b);
      if(energy>energy_max && counts>10) energy_max=energy; // interesting bins only
#if DEBUG > 1
      std::cout<<b<<"\t"<<dt<<"\t"<<energy<<"\t"<<dumpHisto->GetBinContent(b)<<std::endl;
#endif
      hEnergy->Fill(energy, counts);

    }

  //hEnergy->GetXaxis()->SetRangeUser(Emin,energy_max);

  // calculate the total number of counts in the intresting range
  Int_t binmin = hEnergy->FindBin(Emin),
    binmax = hEnergy->FindBin(energy_max);
  Double_t total_counts = hEnergy->Integral(binmin,binmax);
  char integral[80];
  sprintf(integral,"Integral %1.01f",total_counts);

  // information
  TPaveText* tt = new TPaveText(0.1,0.5,0.9,0.9);
  tt->SetFillColor(0);
  tt->AddText(resolution);
  tt->AddText(integral);

  // plotting and wait for fitting
  //  if (gLegendDetail>=1)
  //    {
      gStyle->SetOptStat(1011111);
      //    }
      //  else
      //    {
      //      gStyle->SetOptStat("ni");
      //    }
  delete gc;
  TCanvas* cEnergy = new TCanvas("AntiprotonTemperature","AntiprotonTemperature",1800,1000);
  cEnergy->Divide(2,2);
  cEnergy->cd(1);
  gPad->SetLogy(1);
  dumpHisto_toPlot->Draw("HIST");
  cEnergy->cd(2);
  
  hEnergy->GetXaxis()->SetRangeUser(0,EnergyRangeFactor*(hEnergy->GetMean()));
  hEnergy->Draw("E1 HIST");
  gPad->SetLogy(1);
  cEnergy->cd(3);
  dumpRamp->Draw();

  cEnergy->cd(4);
  tt->Draw();

  //necessary for fitting
  gc=cEnergy;
  gh=hEnergy;
  return cEnergy;
}
#endif

Double_t Boltzmann_constant = 8.61733e-5; //eV/K

Double_t FitEnergyDump( Double_t Emin, Double_t Emax,TH1D* h)
{

  if (!h) h=gh;
  if(!h) {std::cout<<"Nothing to fit... exiting"<<std::endl; return 0.;}
  h->Fit("expo","QM0","",Emin,Emax);
  TF1* fEnergy = h->GetFunction("expo");
  fEnergy->SetLineColor(kBlue);

  Double_t temperature = -1./fEnergy->GetParameter(1)/Boltzmann_constant,
    error = -temperature*fEnergy->GetParError(1)/fEnergy->GetParameter(1);
  if(!gc) { std::cout<<"Nothing to draw into... "<<std::endl; return temperature;}

  gc->cd(2);
  fEnergy->Draw("same");

  char result[100];
  sprintf(result," ( %.2lf #pm %.2lf ) K ",temperature,error);

  TPaveText* pt = new TPaveText(0.1,0.1,0.9,0.4);
  pt->SetFillColor(0);
  pt->SetFillStyle(1001);
  pt->SetTextColor(kRed);
  pt->AddText("#bar{p} temperature");
  pt->AddText(result);
  gc->cd(4);
  pt->Draw("same");

  return temperature;
}




void SaveCanvas()
{
  //Function to save the global canvas (gc)
  TString FileName;
  std::cout << "Please input the chosen filename (no file extension needed)" << std::endl;
  std::cin >> FileName;
  TString Output = MakeAutoPlotsFolder("");
  Output+=FileName;
  SaveCanvas(Output);
}

void SaveCanvas(Int_t runNumber, const char* Description)
{
  TString Output = MakeAutoPlotsFolder("");
  Output+="R";
  Output+=runNumber;
  Output+=Description;
  SaveCanvas(Output);
}

void SaveCanvas(TString Description)
{
  Description+=".pdf";
  gc->SaveAs(Description);
  std::cout << "File saved here:" << std::endl << Description << std::endl;
}

void SaveCanvas( TCanvas* iSaveCanvas, TString iDescription){
	TString Output = MakeAutoPlotsFolder("");
	Output+=iDescription;
	Output+=".png";
	iSaveCanvas->SaveAs(Output);
	
}


#ifdef BUILD_A2
TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
{
   TCanvas* c = new TCanvas();
   AlphaColourWheel colour;
   TLegend* legend = new TLegend(0.1,0.7,0.48,0.9);

   std::vector<TH1D*> hh=Get_Summed_SIS(runNumber, SIS_Channel,tmin, tmax);
   double max_height = 0;
   double min_height = 1E99;
   for (TH1D* h: hh)
   {
      double min, max;
      h->GetMinimumAndMaximum(min, max);
      if (max > max_height)
      {
         max_height = max;
      }
      if (min < min_height)
      {
         min_height = min;
      }
   }
   if (min_height < 10 && min_height >= 0)
      min_height = 0;
   for (size_t i=0; i<hh.size(); i++)
   {
      legend->AddEntry(hh[i]);
      hh[i]->SetLineColor(colour.GetNewColour());
      if (i ==0 )
      {
         hh[i]->GetYaxis()->SetRangeUser(min_height,max_height);
         hh[i]->Draw("HIST");
         
      }
      else
      {
         hh[i]->GetYaxis()->SetRangeUser(min_height,max_height);
         hh[i]->Draw("HIST SAME");

      }
   }
   //std::cout<<"min:"<< min_height <<"\tmax:"<<max_height <<std::endl;
   legend->Draw();
   c->Update();
   c->Draw();
   return c;
}

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<double> tmin, std::vector<double> tmax)
{
   std::vector<Int_t> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_Summed_SIS(runNumber, chans, tmin, tmax);
}

TCanvas* Plot_SIS_on_pulse(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<std::pair<double,int>> SIS_Counts,double tstart, double tstop)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (auto& a: SIS_Counts)
   {
     tmin.push_back(a.first + tstart);
     tmax.push_back(a.first + tstop);
   }
   return Plot_Summed_SIS(runNumber, SIS_Channel_Names, tmin, tmax);
}

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (TA2Spill& s: spills)
   {
      tmin.push_back(s.GetStartTime());
      tmax.push_back(s.GetStopTime());
   }
   return Plot_Summed_SIS(runNumber,SIS_Channel,tmin, tmax);
}

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<TA2Spill> spills)
{
   std::vector<Int_t> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_Summed_SIS(runNumber, chans, spills);
}

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<TA2Spill> s=Get_A2_Spills(runNumber,description,repetition);
   return Plot_Summed_SIS(runNumber, SIS_Channel, s);
}

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<Int_t> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_Summed_SIS( runNumber, chans, description, repetition);
}


TCanvas* Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
{
   TCanvas* c = new TCanvas();
   AlphaColourWheel colour;
   TLegend* legend = new TLegend(0.1,0.7,0.48,0.9);

   std::vector<std::vector<TH1D*>> hh=Get_SIS(runNumber, SIS_Channel,tmin, tmax);
   double max_height = 0;
   double min_height = 1E99;
   for (auto times: hh)
      for (TH1D* h: times)
      {
         double min, max;
         h->GetMinimumAndMaximum(min, max);
         if (max > max_height)
         {
            max_height = max;
         }
         if (min < min_height)
         {
            min_height = min;
         }
      }
   if (min_height < 10 && min_height >= 0)
      min_height = 0;
   
   for (int j=0; j<tmin.size(); j++)
      for (int i=0; i<SIS_Channel.size(); i++)
      {
         legend->AddEntry(hh[i][j]);
         hh[i][j]->SetLineColor(colour.GetNewColour());
         if (i ==0 && j == 0)
         {
            hh[i][j]->GetYaxis()->SetRangeUser(min_height,max_height);
            hh[i][j]->Draw("HIST");
         }
         else
         {
            hh[i][j]->GetYaxis()->SetRangeUser(min_height,max_height);
            hh[i][j]->Draw("HIST SAME");
         }
      }
   //std::cout<<"min:"<< min_height <<"\tmax:"<<max_height <<std::endl;
   legend->Draw();
   c->Update();
   c->Draw();
   return c;
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<double> tmin, std::vector<double> tmax)
{
   std::vector<Int_t> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_SIS(runNumber, chans, tmin, tmax);
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (TA2Spill& s: spills)
   {
      tmin.push_back(s.GetStartTime());
      tmax.push_back(s.GetStopTime());
   }
   return Plot_SIS(runNumber,SIS_Channel,tmin, tmax);
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<TA2Spill> spills)
{
   std::vector<Int_t> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_SIS(runNumber, chans, spills);
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<TA2Spill> s=Get_A2_Spills(runNumber,description,repetition);
   return Plot_SIS(runNumber, SIS_Channel, s);
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<Int_t> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_SIS( runNumber, chans, description, repetition);
}

#endif
#ifdef BUILD_A2
void Plot_SVD(int runNumber, std::vector<double> tmin, std::vector<double> tmax)
{
   TA2Plot* Plot=new TA2Plot();
   Plot->AddTimeGates(runNumber,tmin,tmax);
   //Slow part, read all data in 1 pass over each tree so is efficient
   Plot->LoadData();
   TCanvas* c=Plot->DrawCanvas("cVTX");
   c->Draw();
}

void Plot_SVD(Int_t runNumber, std::vector<TA2Spill> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         tmin.push_back(spill.ScalerData->StartTime);
         tmax.push_back(spill.ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return Plot_SVD(runNumber,tmin,tmax);
}

void Plot_SVD(Int_t runNumber, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<TA2Spill> s=Get_A2_Spills(runNumber,description,repetition);
   return Plot_SVD(runNumber,s);
}
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
