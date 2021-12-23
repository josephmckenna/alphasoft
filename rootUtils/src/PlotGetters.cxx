#include "PlotGetters.h"
#include "TSISChannels.h"

#include "TStyle.h"

extern Int_t gNbin;
//Plots

#ifdef BUILD_AG

// Significant duplication of code between this and Plot_SIS...
TCanvas* Plot_Chrono(Int_t runNumber, std::vector<TChronoChannel> channel, std::vector<double> tmin, std::vector<double> tmax)
{
   for (const TChronoChannel& c: channel)
      std::cout << c << std::endl;
   TCanvas* c = new TCanvas();
   AlphaColourWheel colour;
   TLegend* legend = new TLegend(0.1,0.7,0.48,0.9);

   std::vector<std::vector<TH1D*>> hh=Get_Chrono(runNumber, channel,tmin, tmax);
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
   
   for (size_t j=0; j<tmin.size(); j++)
      for (size_t i=0; i<channel.size(); i++)
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

#endif
#ifdef BUILD_AG

TCanvas* Plot_Chrono(Int_t runNumber, std::vector<TChronoChannel> channel, std::vector<std::string> description, std::vector<int> index)
{
  std::vector<TAGSpill> spills = Get_AG_Spills(runNumber, description, index);
  std::vector<double> tmin;
  std::vector<double> tmax;
  for (const TAGSpill& s: spills)
  {
    tmin.push_back( s.GetStartTime() );
    tmax.push_back( s.GetStopTime() );
  }
  return Plot_Chrono(runNumber, channel, tmin, tmax);
}
#endif
#ifdef BUILD_AG
TCanvas* Plot_Chrono(Int_t runNumber, const char* ChannelName, std::vector<std::string> description, std::vector<int> dumpIndex)
{
  std::vector<TChronoChannel> chan;
  chan.push_back( Get_Chrono_Channel(runNumber, ChannelName) );
  return Plot_Chrono(runNumber, chan, description, dumpIndex);
}
#endif

#ifdef BUILD_AG
void Plot_Delta_Chrono(Int_t runNumber, TChronoChannel channel, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TH1D* h=Get_Delta_Chrono( runNumber, channel, tmin, tmax);  
  h->GetXaxis()->SetTitle("Time [s]");
  h->GetYaxis()->SetTitle("Counts");
  h->Draw();
  return;  
} 
#endif
#ifdef BUILD_AG
void Plot_Delta_Chrono(Int_t runNumber, TChronoChannel channel, const char* description, Int_t dumpIndex)
{
  std::vector<TAGSpill> spills = Get_AG_Spills(runNumber, {description}, {dumpIndex});
  std::vector<double> tmin;
  std::vector<double> tmax;
  for (const TAGSpill& s: spills)
  {
    tmin.push_back( s.GetStartTime() );
    tmax.push_back( s.GetStopTime() );
  }
  return Plot_Delta_Chrono(runNumber, channel, tmin.front(), tmax.front());
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
void Plot_Delta_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t dumpIndex)
{
   std::vector<TAGSpill> spills = Get_AG_Spills(runNumber, {description}, {dumpIndex});
   return Plot_Delta_Chrono(runNumber, ChannelName, spills.front().GetStartTime(), spills.front().GetStopTime());
}
#endif
#ifdef BUILD_AG
void PlotChronoScintillators(Int_t runNumber, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);

  std::vector<std::string> channels {"SiPM_A","SiPM_D","SiPM_A_OR_D","SiPM_E",
      "SiPM_C","SiPM_F","SiPM_C_OR_F","SiPM_B"};
  TString cname = TString::Format("cSiPM_%1.3f-%1.3f",tmin,tmax);
  TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1800, 1500);
  c->Divide(2,4);
  int i=0;
  for(auto it = channels.begin(); it!=channels.end(); ++it)
    {
      TChronoChannel chan = Get_Chrono_Channel(runNumber,it->c_str());
      TH1D* h=Get_Chrono( runNumber, {chan}, {tmin}, {tmax}).front().front();
      h->GetXaxis()->SetTitle("Time [s]");
      h->GetYaxis()->SetTitle("Counts"); 
      c->cd(++i);
      h->Draw();
    }
  return;
}
#endif
#ifdef BUILD_AG
void PlotChronoScintillators(Int_t runNumber, const char* description, Int_t dumpIndex)
{
   std::vector<TAGSpill> spills = Get_AG_Spills(runNumber, {description}, {dumpIndex});
   return PlotChronoScintillators(runNumber, spills.front().GetStartTime(), spills.front().GetStopTime());
}
#endif
#ifdef BUILD_AG
TCanvas* Plot_TPC(Int_t runNumber,  Double_t tmin, Double_t tmax, bool ApplyCuts)
{
  if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
  TAGPlot* p=new TAGPlot(ApplyCuts); //Cuts off
  p->SetTimeRange(0.,tmax-tmin);
  p->AddEvents(runNumber,tmin,tmax);
  TString cname = TString::Format("cVTX_R%d",runNumber);
  std::cout<<"NVerts:"<<p->GetTotalVertices()<<std::endl;
  return p->Canvas(cname);
}
#endif
#ifdef BUILD_AG
TCanvas* Plot_TPC(Int_t runNumber,  const char* description, Int_t dumpIndex, bool ApplyCuts)
{
   std::vector<TAGSpill> spills = Get_AG_Spills(runNumber, {description}, {dumpIndex});
   double tmin = spills.front().GetStartTime();
   double tmax = spills.front().GetStopTime();
  std::cout<<"Dump at ["<<tmin<<","<<tmax<<"] s   duration: "<<tmax-tmin<<" s"<<std::endl;
  double ttmin = GetTrigTimeBefore(runNumber,tmin),
    ttmax = GetTrigTimeAfter(runNumber,tmax);
  std::cout<<"Trigger window ["<<ttmin<<","<<ttmax<<"] s   duration:"<<ttmax-ttmin<<" s"<<std::endl;
  return Plot_TPC(runNumber,tmin,tmax, ApplyCuts);
}
#endif
#ifdef BUILD_AG
TCanvas* Plot_TPC(Int_t* runNumber, Int_t Nruns, const char* description, Int_t dumpIndex)
{ 
   TAGPlot* p=new TAGPlot(0); //Cuts off  
   for( Int_t i=0; i<Nruns; ++i )
   {
      std::cout<<"Run"<<runNumber[i]<<std::endl;
      std::vector<TAGSpill> spills = Get_AG_Spills(runNumber[i], {description}, {dumpIndex});
      double tmin = spills.front().GetStartTime();
      double tmax = spills.front().GetStopTime();
  
      std::cout<<"Dump at ["<<tmin<<","<<tmax<<"] s   duration: "<<tmax-tmin<<" s"<<std::endl;
      double ttmin = GetTrigTimeBefore(runNumber[i],tmin),
      ttmax = GetTrigTimeAfter(runNumber[i],tmax);
      std::cout<<"Trigger window ["<<ttmin<<","<<ttmax<<"] s   duration:"<<ttmax-ttmin<<" s"<<std::endl;
      p->SetTimeRange(0.,tmax-tmin);
      p->AddEvents(runNumber[i],tmin,tmax);
    }
  TString cname = TString::Format("cVTX_%s_Rlist",description);
  return p->Canvas(cname);
}
#endif
#ifdef BUILD_AG
void Plot_Vertices_And_Tracks(Int_t runNumber, double tmin, double tmax, bool ApplyCuts)
{
  TAGPlot* p=new TAGPlot(ApplyCuts); //Cuts off  
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
			      Int_t dumpIndex, bool ApplyCuts)
{ 
  Int_t runList[]={runNumber};
  Int_t Nruns = 1;
  return Plot_Vertices_And_Tracks( runList, Nruns, description, 
				   dumpIndex, ApplyCuts);
}
#endif
#ifdef BUILD_AG
void Plot_Vertices_And_Tracks(Int_t* runNumber, Int_t Nruns, const char* description, 
			      Int_t dumpIndex, bool ApplyCuts)
{ 
  TAGPlot* p=new TAGPlot(ApplyCuts); //Cuts off  
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
	   std::vector<TAGSpill> spills = Get_AG_Spills(runNumber[i], {description}, {dumpIndex});
     tmin = spills.front().GetStartTime();
     tmax = spills.front().GetStopTime();
  
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

//*************************************************************
// Energy Analysis
//*************************************************************
TCanvas* gc;
TH1D* gh;
#ifdef BUILD_AG
TCanvas* Plot_AG_ColdDump(Int_t runNumber,Int_t dumpIndex, Int_t binNumber, const char* dumpFile, Double_t EnergyRangeFactor, const char* Chrono_Channel_Name)
{  
   std::vector<TAGSpill> spills = Get_AG_Spills(runNumber, {"Cold Dump"}, {dumpIndex});
   double start_time = spills.front().GetStartTime();
   double stop_time = spills.front().GetStopTime();
   if (stop_time<0.) stop_time = GetAGTotalRunTime(runNumber);

   double startOffset = 0.002; // dump starts two milliseconds after the start dump trigger
   double stopOffset = 0.; // dump finishes at trigger

   double dumpDuration = stop_time - start_time - startOffset - stopOffset;

   std::cout <<"Dump start: "<< start_time-startOffset << " Dump stop: " << stop_time-stopOffset << std::endl;
   std::cout<<"Dump Duration "<<"\t"<<dumpDuration<<" s"<<std::endl;

   Int_t oldBinNumber = gNbin;
   gNbin=1.e4;
   TH1D* dumpHisto = Get_Chrono( runNumber, {Get_Chrono_Channel(runNumber,Chrono_Channel_Name)}, {start_time}, {stop_time} ).front().front();
  gNbin=oldBinNumber;
 
  if(!dumpHisto){Error("PlotEnergyDump","NO CB counts plot"); return 0;}
  // and the voltage ramp function of time
  TSpline5* dumpRamp = InterpolateVoltageRamp(dumpFile);
  if(!dumpRamp){Error("PlotEnergyDump","NO voltage ramp function"); return 0;}
    TH1D* dumpHisto_toPlot =Get_Chrono(runNumber,{Get_Chrono_Channel(runNumber,Chrono_Channel_Name)},{start_time+startOffset},{stop_time}).front().front(); // this is a lower resolution histo to plot

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

TCanvas* Plot_AG_CT_ColdDump(Int_t runNumber,Int_t dumpIndex, Int_t binNumber, const char* dumpFile, Double_t EnergyRangeFactor)
{
   return Plot_AG_ColdDump( runNumber, dumpIndex, binNumber, dumpFile, EnergyRangeFactor,"PMT_CATCH_OR");
}

TCanvas* Plot_AG_RCT_ColdDump(Int_t runNumber,Int_t dumpIndex, Int_t binNumber, const char* dumpFile, Double_t EnergyRangeFactor)
{
   return Plot_AG_ColdDump(runNumber, dumpIndex, binNumber, dumpFile, EnergyRangeFactor,"SiPM_E");
}

#endif


#ifdef BUILD_A2
TCanvas* Plot_A2_CT_HotDump(Int_t runNumber, Int_t repitition, Int_t binNumber, const char* dumpFile, Double_t EnergyRangeFactor)
{  
  //Double_t start_time=MatchEventToTime(runNumber, "Cold Dump",true,1,0);
  //Double_t stop_time=MatchEventToTime(runNumber, "Cold Dump",false,1,0);

  std::vector<TA2Spill> spills = Get_A2_Spills(runNumber,{"Hot Dump"},{-1});
  std::cout << "Spills size = " << spills.size() << std::endl;
  std::vector<double> tmin;
  std::vector<double> tmax;
  std::cout << "tmin size = " << tmin.size() << std::endl;
  std::cout << "tmax size = " << tmax.size() << std::endl;
  for (TA2Spill& s: spills) {
    tmin.push_back(s.GetStartTime());
    tmax.push_back(s.GetStopTime());
  }
    std::cout << "tmin size = " << tmin.size() << std::endl;
  std::cout << "tmax size = " << tmax.size() << std::endl;

  Double_t start_time = tmin.at(repitition);
  Double_t stop_time = tmax.at(repitition);

  if (stop_time<0.) stop_time=GetA2TotalRunTime(runNumber);

  Double_t startOffset = 0.002; // dump starts two milliseconds after the start dump trigger
  Double_t stopOffset = 0.; // dump finishes at trigger
  
  Double_t dumpDuration = stop_time-start_time-startOffset-stopOffset;
  
  std::cout <<"Dump start: "<< start_time-startOffset << " Dump stop: " << stop_time-stopOffset << std::endl;
  std::cout<<"Dump Duration "<<"\t"<<dumpDuration<<" s"<<std::endl;

  Int_t oldBinNumber = gNbin;
  gNbin=1.e4;

  TSISChannels chans(runNumber);
  TSISChannel channel = chans.GetChannel("SIS_PMT_CATCH_OR");
  std::vector<TSISChannel> SISChannels = {channel};
  
  std::vector<TH1D*> dumpHisto = Get_SIS( runNumber, SISChannels, {start_time}, {stop_time}).front();
  gNbin=oldBinNumber;
 
  if(!dumpHisto.at(0)){Error("PlotEnergyDump","NO CB counts plot"); return 0;}
  // and the voltage ramp function of time
  TSpline5* dumpRamp = InterpolateVoltageRamp(dumpFile);
  if(!dumpRamp){Error("PlotEnergyDump","NO voltage ramp function"); return 0;}
     std::vector<TH1D*> dumpHisto_toPlot = Get_SIS(runNumber, SISChannels, {start_time+startOffset}, {stop_time}).front(); // this is a lower resolution histo to plot

  TString htitle = "Run ";
  htitle+=runNumber;
  htitle+=" : #bar{p} energy  Hot Dump;Energy [eV];counts";

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
  for(Int_t b=1; b<=dumpHisto.at(0)->GetNbinsX(); ++b)
    {
      dt = (dumpHisto.at(0)->GetBinCenter(b)/*-start_time*/-startOffset);//(dumpDuration);
      energy = dumpRamp->Eval(dt);
      //~ //if(energy<res) continue;
      counts = dumpHisto.at(0)->GetBinContent(b);
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
    //Lukas turned off
      //TStyle->SetOptStat(1011111);
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
  dumpHisto_toPlot.at(0)->Draw("HIST");
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



TCanvas* MultiPlotRunsAndDumps(std::vector<Int_t> runNumbers, std::string SISChannel, std::vector<std::string> description, std::vector<std::vector<int>> dumpNumbers, std::string drawOption)
{
  //Set up a vector of final histograms to save.
  std::vector<TH1D*> allHistos;

  //Loop through each run inputed.
  for(int i=0; i<runNumbers.size();  i++) 
  {
    Int_t run = runNumbers.at(i);

    //These 3 lines find the SIS channel of the input. 
    TSISChannels channelFinder(run);
    TSISChannel channel = channelFinder.GetChannel(SISChannel.c_str());
    std::vector<TSISChannel> channels = {channel};

    //Get the spills from the data (should only be the range selected in the dumpNumbers) 
    std::vector<TA2Spill> spills = Get_A2_Spills(run, description, {dumpNumbers.at(i)});
    //Get the SIS from the spill, then pills the first (and only histo) and adds it to allHistos
    allHistos.push_back( Get_SIS(run, channels, {spills.at(0)}).front().front() );
  }

  //Set up a nice title, and create a vector of strings for the legend(s).
  std::vector<std::string> legendStrings;
  std::string title = description.at(0);
  title+="s for the following runs and spills (RunNum/Spill): ";
  for(int i=0; i<runNumbers.size(); i++) 
  {
    //Turn a vector into a string - this is the best way in C++
    std::stringstream result;
    std::copy(dumpNumbers.at(i).begin(), dumpNumbers.at(i).end(), std::ostream_iterator<int>(result, " "));
    
    //Add to the long title string.
    title+="(";
    title+=std::to_string(runNumbers.at(i));
    title+="/";
    title+=result.str();
    title+=")";
    title+=", ";

    //Add each string to legend names vector.
    legendStrings.push_back("Hist " + std::to_string(i) + " = (" + std::to_string(runNumbers.at(i)) + " / [" + result.str() + "])");
  }

  //Create the final canvas, 2D histogram stack, and 2D legend.
  TCanvas *finalCanvas = new TCanvas("finalCanvas",title.c_str());
  THStack *histoStack2D = new THStack("histoStack",title.c_str());
  TLegend *legend2D = new TLegend();
  gStyle->SetPalette(kRainBow); //Select colour style, this has been the clearest I can find.

  //Loop through all histos and stack into a standard 2D stack.
  for(int i=0; i<allHistos.size(); i++) 
  {
    histoStack2D->Add(allHistos.at(i)); //Add each histo to the stack. 
    legend2D->AddEntry(allHistos.at(i), legendStrings.at(i).c_str() ); //Add an entry to the legend.
  }

  //If 2D draw options are selected we will enter this and draw. If not the 2D stack is used to convert to a 3D stack.
  if(drawOption == "2dstack" || drawOption == "2dnostack")
  {
    //We have to draw it first to be able to get the axis. This draw will be overwritten below. 
    histoStack2D->Draw(); //Dummy draw
    histoStack2D->GetXaxis()->SetTitle("Time (s)");
    histoStack2D->GetYaxis()->SetTitle("Counts");
    if(drawOption == "2dstack") 
      histoStack2D->Draw("pfc hist"); //Real draw
    else if (drawOption == "2dnostack")
      histoStack2D->Draw("pfc hist nostack"); //Real draw
    legend2D->Draw(); //Draw legend
  }



  //If 3D options are selected we will skip the 2D draw, go straight to the 3D conversion then the 3D draw.
  if(drawOption == "3dheat" || drawOption == "3dstack")
  {
    //Create new 3D stacks and legend.
    THStack *histoStack3D = new THStack("histoStack",title.c_str());
    TLegend *legend3D = new TLegend();
    //This function adds a z axis to the plots to allow to plot them in 3D.
    Generate3DTHStack(allHistos, histoStack3D, legend3D, legendStrings);
    histoStack3D->Draw(); //Dummy draw, again just to be able to grab axis and set titles.
    histoStack3D->GetXaxis()->SetTitle("Time (s)");
    histoStack3D->GetYaxis()->SetTitle("Run and dump - see legend");
    histoStack3D->GetHistogram()->GetZaxis()->SetTitle("Counts");
    if(drawOption == "3dheat") 
      histoStack3D->Draw("lego2"); //Real draw
    else if (drawOption == "3dstack")
      histoStack3D->Draw("pfc lego1"); //Real draw
    legend3D->Draw(); //Draw the legend
    }
  return finalCanvas;
}
#endif

void Generate3DTHStack(std::vector<TH1D*> allHistos, THStack* emptyStack, TLegend* emptyLegend, std::vector<std::string> legendStrings) {
  //Function takes the vector of relevent histos we had and generates a 3D histogram stack from it for visualisation. 
  int numOfHists = allHistos.size();
  gStyle->SetPalette(kRainBow); //This should match the one above but can be different if you like.
  
  for(int i=0; i<numOfHists; i++) 
  {
    //Loops over all relevant TH1D histograms and grabs the x data.
    TH1D* currentHist = allHistos.at(i); 
    int numXbins = currentHist->GetNbinsX();
    double minX = currentHist->GetXaxis()->GetXmin();
    double maxX = currentHist->GetXaxis()->GetXmax();
    //Creates a TH2D with the old TH1D x data and a arbitrary y axis such that each histogram is back to back.
    TH2D *newHist = new TH2D( Form("h2_%d",i), Form("h2_%d",i), numXbins, minX, maxX, numOfHists, 0, numOfHists);
    for (int j=1; j<=numXbins; j++) 
    {
      newHist->SetBinContent(j,i+1,currentHist->GetBinContent(j)); //Populates each histogram with its original content in x and just its number in y.
    }
    emptyStack->Add(newHist); //Add histogram to our empty stack.
    emptyLegend->AddEntry(newHist, legendStrings.at(i).c_str() ); //Adds legend entry to our old legend.
  }
}


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
  TSubString extension = Description(Description.Sizeof()-5,Description.Sizeof());
  //If not a 3 char extension... add one
  if (extension[0] != '.')
     Description+=".png";
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
TCanvas* Plot_A2_ColdDump(Int_t runNumber,int repetition, Int_t binNumber, const char* dumpFile, Double_t EnergyRangeFactor, const char* SIS_Channel_Name)
{  

   std::vector<TA2Spill> spills = Get_A2_Spills(runNumber,{"Cold Dump"},{-1});
   std::cout << "Spills size = " << spills.size() << std::endl;
   std::vector<double> tmin;
   std::vector<double> tmax;

   std::cout << "tmin size = " << tmin.size() << std::endl;
   std::cout << "tmax size = " << tmax.size() << std::endl;

   for (TA2Spill& s: spills) {
     tmin.push_back(s.GetStartTime());
     tmax.push_back(s.GetStopTime());
   }

   std::cout << "tmin size = " << tmin.size() << std::endl;
   std::cout << "tmax size = " << tmax.size() << std::endl;
   Double_t start_time = tmin.at(repetition);
   Double_t stop_time = tmax.at(repetition);

   if (stop_time<0.) stop_time=GetA2TotalRunTime(runNumber);

   Double_t startOffset = 0.002; // dump starts two milliseconds after the start dump trigger
   Double_t stopOffset = 0.; // dump finishes at trigger
 
   Double_t dumpDuration = stop_time-start_time-startOffset-stopOffset;
 
   std::cout <<"Dump start: "<< start_time-startOffset << " Dump stop: " << stop_time-stopOffset << std::endl;
   std::cout<<"Dump Duration "<<"\t"<<dumpDuration<<" s"<<std::endl;

   Int_t oldBinNumber = gNbin;
   gNbin=1.e4;

   TSISChannels chans(runNumber);

   TSISChannel channel = chans.GetChannel(SIS_Channel_Name);
   std::cout<<"Plotting SIS Channel "<<channel<<std::endl;
   std::vector<TSISChannel> SISChannels = {channel};
 
   std::vector<std::vector<TH1D*>> dumpHisto = Get_SIS( runNumber, SISChannels, {start_time}, {stop_time});

   gNbin=oldBinNumber;

   if(!dumpHisto.at(0).at(0)){Error("PlotEnergyDump","NO CB counts plot"); return 0;}
    // and the voltage ramp function of time
   TSpline5* dumpRamp = InterpolateVoltageRamp(dumpFile);
   if(!dumpRamp){Error("PlotEnergyDump","NO voltage ramp function"); return 0;}
     std::vector<std::vector<TH1D*>> dumpHisto_toPlot = Get_SIS(runNumber, SISChannels, {start_time+startOffset}, {stop_time}); // this is a lower resolution histo to plot
   TString htitle = "Run ";

   htitle+=runNumber;
   htitle+=" : #bar{p} energy  Hot Dump;Energy [eV];counts";

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
   for(Int_t b=1; b<=dumpHisto.at(0).at(0)->GetNbinsX(); ++b)
   {
      dt = (dumpHisto.at(0).at(0)->GetBinCenter(b)/*-start_time*/-startOffset);//(dumpDuration);
      energy = dumpRamp->Eval(dt);
      //~ //if(energy<res) continue;
      counts = dumpHisto.at(0).at(0)->GetBinContent(b);
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
   //if (gLegendDetail>=1)
   //{
   //   TStyle->SetOptStat(1011111);
  //}
   //else
   //{
   //   gStyle->SetOptStat("ni");
  // }
   delete gc;

   TCanvas* cEnergy = new TCanvas("AntiprotonTemperature","AntiprotonTemperature",1800,1000);
   cEnergy->Divide(2,2);
   cEnergy->cd(1);
   gPad->SetLogy(1);
   dumpHisto_toPlot.at(0).at(0)->Draw("HIST");

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

#ifdef BUILD_A2
TCanvas* Plot_A2_CT_ColdDump(Int_t runNumber, int repetition, Int_t binNumber, 
                          const char* dumpFile,
                          Double_t EnergyRangeFactor)
                          {
                            return Plot_A2_ColdDump(runNumber, repetition, binNumber, dumpFile, EnergyRangeFactor, "SIS_PMT_CATCH_OR");
                          }
#endif

#ifdef BUILD_A2
TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
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
   std::vector<TSISChannel> chans = GetSISChannels(runNumber, SIS_Channel_Names);
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

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<TA2Spill> spills)
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
   std::vector<TSISChannel> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_Summed_SIS(runNumber, chans, spills);
}

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TA2Spill> s=Get_A2_Spills(runNumber,description,dumpIndex);
   return Plot_Summed_SIS(runNumber, SIS_Channel, s);
}

TCanvas* Plot_Summed_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TSISChannel> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_Summed_SIS( runNumber, chans, description, dumpIndex);
}


TCanvas* Plot_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
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
   std::vector<TSISChannel> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_SIS(runNumber, chans, tmin, tmax);
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<TA2Spill> spills)
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
   std::vector<TSISChannel> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_SIS(runNumber, chans, spills);
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TA2Spill> s=Get_A2_Spills(runNumber,description,dumpIndex);
   return Plot_SIS(runNumber, SIS_Channel, s);
}

TCanvas* Plot_SIS(Int_t runNumber, std::vector<std::string> SIS_Channel_Names, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TSISChannel> chans = GetSISChannels(runNumber, SIS_Channel_Names);
   return Plot_SIS( runNumber, chans, description, dumpIndex);
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

void Plot_SVD(Int_t runNumber, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TA2Spill> s=Get_A2_Spills(runNumber,description,dumpIndex);
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
