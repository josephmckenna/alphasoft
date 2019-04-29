#include "TAGPlot.h"
#define SCALECUT 0.6

ClassImp(TAGPlot);

//Default Constructor
TAGPlot::TAGPlot(Bool_t ApplyCuts, Int_t MVAMode)
{
  Nbin=100; 
  DrawStyle=0;
  gLegendDetail=1; 

  TMin=-1;
  TMax=-1;

  top={-1,-1};
  bottom={-1,-1};
  TPC_TRIG={-1,-1};
  Beam_Injection={-1,-1};
  /*trig=-1;// = -1;
  trig_nobusy=-1; //Record of SIS channels
  atom_or=-1;
  Beam_Injection=-1;
  Beam_Ejection=-1;
  
  CATStart=-1;
  CATStop=-1;
  RCTStart=-1;
  RCTStop=-1;
  ATMStart=-1;
  ATMStop=-1;
  */
  
  BarMultiplicityCut=3;

  fTotalTime = -1.;
  fTotalVert = -1.;

  fVerbose=false;

  SetMVAMode(MVAMode);
  gApplyCuts=ApplyCuts;
}

TAGPlot* TAGPlot::LoadTAGPlot(TString file)
{
  TFile* f0=new TFile(file);
  TAGPlot* a;//=new TAGPlot();
  f0->GetObject("TAGPlot",a);
  f0->Close();
  delete f0;
  return a;
}

void TAGPlot::Draw(Option_t *option)
{
  TCanvas* a=Canvas("TAGPlot");
  a->Draw(option);
  return;
}

void TAGPlot::Print()
{
  std::cout<<"TAGPlot Summary"<<std::endl;
  FillHisto();
  std::cout <<""<<std::endl<<"Run(s): ";
  for (UInt_t i=0; i<Runs.size(); i++)
  {
     if (i>1) std::cout <<", ";
     std::cout <<Runs[i];
  }
  std::cout <<std::endl;
  
  //Loop over TObj array and print it?
  for (int i=0; i<HISTOS.GetEntries(); i++)
  {
      TH1D* a=dynamic_cast<TH1D*>(HISTOS.At(i));
      if (a)
      {
         std::cout <<a->GetTitle()<<"\t"<<a->Integral()<<std::endl;
      }
  }
  
  /*if (ht_MVA)
    std::cout <<"Pass MVA:      "<<ht_MVA->Integral() <<std::endl;
  if (ht)
    std::cout<<"Passed Cuts:    "<<ht->Integral() <<std::endl;
  if (ht_IO32_sistime)
    std::cout<<"VF48 Events:    "<<ht_IO32_sistime->Integral()<<std::endl;
  if (ht_IO32)
    std::cout<<"SIS readout:    "<<ht_IO32->Integral()<<std::endl;
  if (ht_IO32_notbusy)
    std::cout<<"IO32 NotBusy:   "<<ht_IO32_notbusy->Integral() <<std::endl;
  */
}

void TAGPlot::ExportCSV(TString filename, Bool_t PassedCutOnly)
{
  
  if (!filename.EndsWith(".csv"))
  {
    TString tmp("TAGPlot");
    tmp+=filename;
    tmp+="_vtx.csv";
    filename=tmp;
  }
  std::ofstream SaveCSV (filename);

  SaveCSV<<"RunNumber,Event Number,RunTime (Official time), TPC Time,VertexStatus,nHelices,NBars,x,y,z,t,Passed Cut,Passed MVA"<<std::endl;
  for (UInt_t i=0; i<VertexEvents.size(); i++)
  {
    //Skip events that fail cuts if only saving passed cuts
    if (PassedCutOnly && !VertexEvents[i].CutsResult ) continue; 
    SaveCSV<<VertexEvents[i].runNumber<<",";
    SaveCSV<<VertexEvents[i].EventNo<<",";
    SaveCSV<<VertexEvents[i].RunTime<<",";
    SaveCSV<<VertexEvents[i].EventTime<<",";
    SaveCSV<<VertexEvents[i].VertexStatus<<",";
    SaveCSV<<VertexEvents[i].nHelices<<",";
    SaveCSV<<VertexEvents[i].NBars<<",";
    SaveCSV<<VertexEvents[i].x<<",";
    SaveCSV<<VertexEvents[i].y<<",";
    SaveCSV<<VertexEvents[i].z<<",";
    SaveCSV<<VertexEvents[i].t<<",";
    SaveCSV<<(VertexEvents[i].CutsResult&1)<<",";
    SaveCSV<<(VertexEvents[i].CutsResult&2)<<","<<std::endl;
  }
  SaveCSV.close();
  
}

//Default Destructor
TAGPlot::~TAGPlot()
{
  ClearHisto();
  Ejections.clear();
  Injections.clear();
  DumpStarts.clear();
  DumpStops.clear();
  ChronoChannels.clear();
  Runs.clear();
  VertexEvents.clear();
  ChronoPlotEvents.clear();
}

void TAGPlot::ClearHisto() //Destroy all histograms
{
   //HISTOS.SetOwner(kTRUE);
   //HISTOS.Delete();
}


void TAGPlot::AddToTAGPlot(TAGPlot *ialphaplot)
{
  ClearHisto();
  ChronoPlotEvents.insert(ChronoPlotEvents.end(), ialphaplot->ChronoPlotEvents.begin(), ialphaplot->ChronoPlotEvents.end());
  VertexEvents.insert(VertexEvents.end(), ialphaplot->VertexEvents.begin(), ialphaplot->VertexEvents.end());
  Ejections.insert(Ejections.end(), ialphaplot->Ejections.begin(), ialphaplot->Ejections.end());
  Injections.insert(Injections.end(), ialphaplot->Injections.begin(), ialphaplot->Injections.end());
  Runs.insert(Runs.end(), ialphaplot->Runs.begin(), ialphaplot->Runs.end());
  //Draw();
}

void TAGPlot::AddToTAGPlot(TString file)
{
  TAGPlot* tmp=LoadTAGPlot(file);
  AddToTAGPlot(tmp);
  delete tmp;
  return;
}


void TAGPlot::AddStoreEvent(TStoreEvent *event, Double_t OfficialTimeStamp, Double_t StartOffset)
{
  VertexEvent Event;
  TVector3 vtx = event->GetVertex();
  Event.EventNo= event->GetEventNumber();

  Event.EventTime= event->GetTimeOfEvent();
  Event.RunTime= OfficialTimeStamp;
  Event.t= OfficialTimeStamp - StartOffset;
  Event.VertexStatus=event->GetVertexStatus();
  Event.x=vtx.X();
  Event.y=vtx.Y();
  Event.z=vtx.Z();
  Event.nHelices=event->GetUsedHelices()->GetEntries();
  Event.NBars=event->GetBarMultiplicity();
  Event.nTracks=event->GetNumberOfTracks();
  //Event.vtx=vtx;
  Int_t CutsResult = 3;

  //ht_IO32_sistime->Fill(run_time);
  if (MVAMode > 0)
  {
    //CutsResult=TRootUtils::ApplyMVA(sil_event);
  }
  else
  {
    if (gApplyCuts)
      CutsResult=ApplyCuts(event);
  }
  Event.CutsResult=CutsResult;
  VertexEvents.push_back(Event);

  if( Event.VertexStatus >= 1 && Event.CutsResult > 0 )
    {
      ++fTotalVert;
      if( fVerbose ) event->Print();
    }

  if( fPlotTracks )
    {
      ProcessHelices(event->GetHelixArray());
      ProcessUsedHelices(event->GetUsedHelices());
    }

  return;
}

void TAGPlot::ProcessHelices(const TObjArray* tracks)
{
  int Nhelices = tracks->GetEntries();
  for(int i=0; i<Nhelices; ++i)
    {
      HelixEvent helix;
      TStoreHelix* aHelix = (TStoreHelix*) tracks->At(i);
      helix.parD = aHelix->GetD();
      //helix.Curvature = aHelix->GetC();
      helix.Curvature = aHelix->GetRc();
      helix.pT = aHelix->GetMomentumV().Perp();
      helix.pZ = aHelix->GetMomentumV().Z();
      helix.pTot = aHelix->GetMomentumV().Mag();
      helix.nPoints = aHelix->GetNumberOfPoints(); 
      HelixEvents.push_back(helix);
      const TObjArray* points = aHelix->GetSpacePoints();
      for(int ip = 0; ip<points->GetEntries(); ++ip  )
	{
	  TSpacePoint* ap = (TSpacePoint*) points->At(ip);
	  SpacePointEvent sp;
	  sp.x = ap->GetX();
	  sp.y = ap->GetY();
	  sp.z = ap->GetZ();
	  sp.r = ap->GetR();
	  sp.p = ap->GetPhi()*TMath::RadToDeg();
	  SpacePointHelixEvents.push_back(sp);
	}
    }
}

void TAGPlot::ProcessUsedHelices(const TObjArray* tracks)
{
  int Nhelices = tracks->GetEntries();
  for(int i=0; i<Nhelices; ++i)
    {
      HelixEvent helix;
      TFitHelix* aHelix = (TFitHelix*) tracks->At(i);
      helix.parD = aHelix->GetD();
      //helix.Curvature = aHelix->GetC();
      helix.Curvature = aHelix->GetRc();
      helix.pT = aHelix->GetMomentumV().Perp();
      helix.pZ = aHelix->GetMomentumV().Z();
      helix.pTot = aHelix->GetMomentumV().Mag();
      helix.nPoints = aHelix->GetNumberOfPoints(); 
      UsedHelixEvents.push_back(helix);
      const std::vector<TSpacePoint*>* points = aHelix->GetPointsArray();
      for(uint ip = 0; ip<points->size(); ++ip  )
	{
	  TSpacePoint* ap = (TSpacePoint*) points->at(ip);
	  SpacePointEvent sp;
	  sp.x = ap->GetX();
	  sp.y = ap->GetY();
	  sp.z = ap->GetZ();
	  sp.r = ap->GetR();
	  sp.p = ap->GetPhi()*TMath::RadToDeg();
	  SpacePointUsedHelixEvents.push_back(sp);
	}
    }
}
 

void TAGPlot::AddChronoEvent(TChrono_Event *event, double official_time, Double_t StartOffset)
{
  ChronoPlotEvent Event;
  Event.runNumber     =0;//event->GetRunNumber();
  Event.Counts        =event->GetCounts();
  Event.Chrono_Channel.Board  =event->GetBoardIndex()-1;
  Event.Chrono_Channel.Channel=event->GetChannel();
  Event.RunTime       =event->GetRunTime();
  Event.OfficialTime  =official_time;
  Event.t             =official_time-StartOffset;
  ChronoPlotEvents.push_back(Event);
}

//Maybe dont have this function... lets see...
Int_t TAGPlot::AddEvents(Int_t runNumber, char *description, Int_t repetition, Double_t Toffset, Bool_t zeroTime)
{
  Double_t start_time = MatchEventToTime(runNumber, "startDump", description, repetition);
  Double_t stop_time = MatchEventToTime(runNumber, "stopDump", description, repetition);
  return AddEvents(runNumber, start_time, stop_time, Toffset, zeroTime);
}

Int_t TAGPlot::AddEvents(Int_t runNumber, Double_t tmin, Double_t tmax, Double_t Toffset, Bool_t zeroTime)
{
  if (TMax < 0. && TMin < 0.)
  {
    std::cout << "Setting time range" << std::endl;
    if (zeroTime)
      SetTimeRange(0. - Toffset, tmax - tmin - Toffset);
    else
      SetTimeRange(tmin - Toffset, tmax - Toffset);
    PrintTimeRange();

  } // If plot range not set... use first instance of range

  fTotalTime += (tmax-tmin);
  //cout <<"Sis channels set."<<endl;
  //Add Silicon Events:
  //cout <<"Adding Silicon Events"<<endl;

  if (std::find(Runs.begin(), Runs.end(), runNumber) == Runs.end() || Runs.size() == 0)
  {
    //std::cout <<"Adding runNo"<<std::endl;
    Runs.push_back(runNumber);
  }
  TStoreEvent *store_event = new TStoreEvent();
  Double_t official_time;
  TTree *t0 = Get_StoreEvent_Tree(runNumber, official_time);
  t0->SetBranchAddress("StoredEvent", &store_event);
  //SPEED THIS UP BY PREPARING FIRST ENTRY!
  Int_t processed_events = 0;
  for (Int_t i = 0; i < t0->GetEntries(); ++i)
  {
    store_event->Reset();
    t0->GetEntry(i);
    //store_event->Print();
    if (!store_event)
    {
      std::cout<<"NULL TStore event: Probably more OfficialTimeStamps than events"<<std::endl;
      break;
    }
    if (official_time <= tmin)
    {
      continue;
    }
    if (official_time > tmax)
    {
      break;
    }
    if (zeroTime)
      AddStoreEvent(store_event, official_time, Toffset + tmin);
    else
      AddStoreEvent(store_event, official_time,Toffset);
    ++processed_events;
    if( (processed_events%1000) == 0 ) std::cout<<"TAGPlot::AddEvents StoreEvents: "<<processed_events<<std::endl;
  }
  if (store_event) delete store_event;
  delete t0;
  //Add SIS Events:
  std::cout <<"Adding Chrono Events"<<std::endl;
  SetChronoChannels(runNumber);
  //SetSISChannels(runNumber);
 
  int processed_ts=0;
  for (UInt_t j=0; j<ChronoChannels.size(); j++)
  {
    //std::cout <<"Adding Channel: "<<ChronoChannels[j]<<std::endl;
    double official_time;
    TTree *t = Get_Chrono_Tree(runNumber, ChronoChannels[j].Board, ChronoChannels[j].Channel,official_time);
    TChrono_Event* e=new TChrono_Event();

    t->SetBranchAddress("ChronoEvent", &e);
    for (Int_t i = 0; i < t->GetEntriesFast(); ++i)
    {
      t->GetEntry(i);
      if (official_time <= tmin)
        continue;
      if (official_time > tmax)
        break;
      if (zeroTime)
        AddChronoEvent(e, official_time, Toffset + tmin);
      else
        AddChronoEvent(e, official_time, Toffset);

      ++processed_ts;
      if( (processed_ts%1000) == 0 ) std::cout<<"TAGPlot::AddEvents Chrono Events: "<<processed_ts<<std::endl;
    }
    delete e;
    delete t;
  }
  return processed_events;
}

void TAGPlot::SetChronoChannels(Int_t runNumber)
{
   top      = Get_Chrono_Channel( runNumber, "SiPM_B");
   bottom   = Get_Chrono_Channel( runNumber, "SiPM_E");
   sipmad   = Get_Chrono_Channel( runNumber, "SiPM_A_AND_D");
   sipmcf   = Get_Chrono_Channel( runNumber, "SiPM_C_AND_F");
   TPC_TRIG = Get_Chrono_Channel( runNumber, "TPC_TRIG");
   Beam_Injection = Get_Chrono_Channel( runNumber, "AD_TRIG");
/*
  TSISChannels *sisch = new TSISChannels(runNumber);
  trig =           sisch->GetChannel("IO32_TRIG");
  trig_nobusy =    sisch->GetChannel("IO32_TRIG_NOBUSY");
  atom_or =        sisch->GetChannel("SIS_PMT_ATOM_OR");
  Beam_Injection = sisch->GetChannel("SIS_AD");
  Beam_Ejection =  sisch->GetChannel("SIS_AD_2");
  CATStart =       sisch->GetChannel("SIS_PBAR_DUMP_START");
  CATStop =        sisch->GetChannel("SIS_PBAR_DUMP_STOP");
  RCTStart =       sisch->GetChannel("SIS_RECATCH_DUMP_START");
  RCTStop =        sisch->GetChannel("SIS_RECATCH_DUMP_STOP");
  ATMStart =       sisch->GetChannel("SIS_ATOM_DUMP_START");
  ATMStop =        sisch->GetChannel("SIS_ATOM_DUMP_STOP");
*/
  //Add all valid SIS channels to a list for later:
  if (top.Channel>0)             ChronoChannels.push_back(top);
  if (bottom.Channel>0)          ChronoChannels.push_back(bottom);
  if (sipmad.Channel>0)             ChronoChannels.push_back(sipmad);
  if (sipmcf.Channel>0)          ChronoChannels.push_back(sipmcf);
  if (TPC_TRIG.Channel>0)        ChronoChannels.push_back(TPC_TRIG);
  if (Beam_Injection.Channel>0)  ChronoChannels.push_back(Beam_Injection);
  /*if (CATStart>0)       SISChannels.push_back(CATStart);
  if (CATStop>0)        SISChannels.push_back(CATStop);
  if (RCTStart>0)       SISChannels.push_back(RCTStart);
  if (RCTStop>0)        SISChannels.push_back(RCTStop);
  if (ATMStart>0)       SISChannels.push_back(ATMStart);
  if (ATMStop>0)        SISChannels.push_back(ATMStop);*/
   

   std::cout <<"Top:"<<top<<std::endl;
   std::cout <<"Bottom:"<<bottom<<std::endl;
   std::cout <<"TPC_TRIG:"<<TPC_TRIG<<std::endl;
   return;
}

void TAGPlot::AutoTimeRange()
{
  std::cout <<"Automatically setting time range for histrograms: ";
  TMax=GetSilEventMaxT();
  TMin=GetSilEventMinT();
  std::cout <<TMin<<"s - "<<TMax<<"s"<<std::endl;
}

void TAGPlot::SetTimeRange(Double_t tmin_, Double_t tmax_)
{
  TMin = tmin_;
  TMax = tmax_;
  SetUpHistograms();
  return;
}

Double_t TAGPlot::GetSilEventMaxT()
{
  Double_t max=-1;
  for (UInt_t i=0; i<VertexEvents.size(); i++)
  {
    if (max<VertexEvents[i].t) max=VertexEvents[i].t;
  }
  return max;
}

Double_t TAGPlot::GetChronoPlotEventMaxT()
{
  Double_t max=-1;
  for (UInt_t i=0; i<ChronoPlotEvents.size(); i++)
  {
    if (max<ChronoPlotEvents[i].t) max=ChronoPlotEvents[i].t;
  }
  return max;
}

Double_t TAGPlot::GetSilEventMinT()
{
  Double_t min=9999999.;
  for (UInt_t i=0; i<VertexEvents.size(); i++)
  {
    if (min>VertexEvents[i].t) min=VertexEvents[i].t;
  }
  return min;
}

Double_t TAGPlot::GetChronoPlotEventMinT()
{
  Double_t min=9999999.;
  for (UInt_t i=0; i<ChronoPlotEvents.size(); i++)
  {
    if (min>ChronoPlotEvents[i].t) min=ChronoPlotEvents[i].t;
  }
  return min;
}

void TAGPlot::SetUpHistograms()
{
   Bool_t ScaleAsMiliSeconds=kFALSE;
   if (fabs(TMax-TMin)<SCALECUT)
      ScaleAsMiliSeconds=kTRUE;
   Double_t XMAX,YMAX,RMAX,ZMAX;
   XMAX=YMAX=RMAX=100.;
   ZMAX=1300.;
   if (HISTOS.GetEntries()>0)
   {
      HISTOS.Delete();
      HISTO_POSITION.clear();
   }
   
   TH1D* top=new TH1D("top_pm", "t;t [s];events", Nbin, TMin, TMax);
   top->SetLineColor(kGreen);
   top->SetMarkerColor(kGreen);
   top->SetMinimum(0);
   HISTOS.Add(top);
   HISTO_POSITION["top_pm"]=HISTOS.GetEntries()-1;

  
   TH1D* bot=new TH1D("bot_pm", "t;t [s];events", Nbin, TMin, TMax);
   bot->SetLineColor(kAzure - 8);
   bot->SetMarkerColor(kAzure - 8);
   bot->SetMinimum(0);
   HISTOS.Add(bot);
   HISTO_POSITION["bot_pm"]=HISTOS.GetEntries()-1;

   TH1D* aandd=new TH1D("aandd_pm", "t;t [s];events", Nbin, TMin, TMax);
   aandd->SetLineColor(kGreen);
   aandd->SetMarkerColor(kGreen);
   aandd->SetMinimum(0);
   HISTOS.Add(aandd);
   HISTO_POSITION["aandd_pm"]=HISTOS.GetEntries()-1;
  
   TH1D* candf=new TH1D("candf_pm", "t;t [s];events", Nbin, TMin, TMax);
   candf->SetLineColor(kAzure - 8);
   candf->SetMarkerColor(kAzure - 8);
   candf->SetMinimum(0);
   HISTOS.Add(candf);
   HISTO_POSITION["candf_pm"]=HISTOS.GetEntries()-1;

   
   TH1D* TPC=new TH1D("TPC_TRIG", "t;t [s];events", Nbin, TMin, TMax);
   TPC->SetMarkerColor(kRed);
   TPC->SetLineColor(kRed);
   TPC->SetMinimum(0);
   HISTOS.Add(TPC);
   HISTO_POSITION["TPC_TRIG"]=HISTOS.GetEntries()-1;

   
   
   HISTOS.Add(new TH1D("zvtx", "Z Vertex;z [mm];events", Nbin, -ZMAX, ZMAX));
   HISTO_POSITION["zvtx"]=HISTOS.GetEntries()-1;

   TH1D* hr = new TH1D("rvtx", "R Vertex;r [mm];events", Nbin, 0., RMAX);
   hr->SetMinimum(0);
   HISTOS.Add(hr);
   HISTO_POSITION["rvtx"]=HISTOS.GetEntries()-1;

   TH1D* hphi = new TH1D("phivtx", "phi Vertex;phi [rad];events", Nbin, -TMath::Pi(), TMath::Pi());
   hphi->SetMinimum(0);
   HISTOS.Add(hphi);
   HISTO_POSITION["phivtx"]=HISTOS.GetEntries()-1;

   TH2D* hxy = new TH2D("xyvtx", "X-Y Vertex;x [mm];y [mm]", Nbin, -XMAX, XMAX, Nbin, -YMAX, YMAX);
   HISTOS.Add(hxy);
   HISTO_POSITION["xyvtx"]=HISTOS.GetEntries()-1;

   TH2D* hzr = new TH2D("zrvtx", "Z-R Vertex;z [mm];r [mm]", Nbin, -ZMAX, ZMAX, Nbin, 0., RMAX);
   HISTOS.Add(hzr);
   HISTO_POSITION["zrvtx"]=HISTOS.GetEntries()-1;

   TH2D* hzphi = new TH2D("zphivtx", "Z-Phi Vertex;z [mm];phi [rad]", Nbin, -ZMAX, ZMAX, Nbin, -TMath::Pi(), TMath::Pi());
   HISTOS.Add(hzphi);
   HISTO_POSITION["zphivtx"]=HISTOS.GetEntries()-1;

   if (ScaleAsMiliSeconds)
   {
      TH1D* ht = new TH1D("tvtx", "t Vertex;t [ms];events", Nbin, TMin*1000., TMax*1000.);
      ht->SetLineColor(kMagenta);
      ht->SetMarkerColor(kMagenta);
      ht->SetMinimum(0);
      HISTOS.Add(ht);
      HISTO_POSITION["tvtx"]=HISTOS.GetEntries()-1;

      TString BarTitle="t bars>";
      BarTitle+=BarMultiplicityCut;
      BarTitle+=";t [ms];events";
      TH1D* htbar = new TH1D("tbar", BarTitle.Data(), Nbin, TMin*1000., TMax*1000.);
      htbar->SetMinimum(0);
      HISTOS.Add(htbar);
      HISTO_POSITION["tbar"]=HISTOS.GetEntries()-1;

      TH2D* hzt = new TH2D("ztvtx", "Z-T Vertex;z [cm];t [ms]", Nbin, -ZMAX, ZMAX, Nbin, TMin*1000., TMax*1000.);
      HISTOS.Add(hzt);
      HISTO_POSITION["ztvtx"]=HISTOS.GetEntries()-1;

      TH2D* hphit = new TH2D("phitvtx", "Phi-T Vertex;phi [rad];t [s]", Nbin,-TMath::Pi(), TMath::Pi() ,  Nbin,TMin*1000., TMax*1000);
      HISTOS.Add(hphit);
      HISTO_POSITION["phitvtx"]=HISTOS.GetEntries()-1;

      //if (MVAMode)
      //   ht_MVA = new TH1D("htMVA", "Vertex, Passcut and MVA;t [ms];Counts", Nbin, TMin*1000., TMax*1000.);
   }
   else
   {
      TH1D* ht = new TH1D("tvtx", "t Vertex;t [s];events", Nbin, TMin, TMax); 
      ht->SetLineColor(kMagenta);
      ht->SetMarkerColor(kMagenta);
      ht->SetMinimum(0);
      HISTOS.Add(ht);
      HISTO_POSITION["tvtx"]=HISTOS.GetEntries()-1;

      TString BarTitle="t bars>";
      BarTitle+=BarMultiplicityCut;
      BarTitle+=";t [ms];events";
      TH1D* htbar= new TH1D("tbar", BarTitle.Data(), Nbin, TMin, TMax);
      htbar->SetMinimum(0);
      HISTOS.Add(htbar);
      HISTO_POSITION["tbar"]=HISTOS.GetEntries()-1;

      TH2D* hzt = new TH2D("ztvtx", "Z-T Vertex;z [mm];t [s]", Nbin, -ZMAX, ZMAX, Nbin, TMin, TMax);
      HISTOS.Add(hzt);
      HISTO_POSITION["ztvtx"]=HISTOS.GetEntries()-1;

      TH2D* hphit = new TH2D("phitvtx", "Phi-T Vertex;phi [rad];t [s]", Nbin,-TMath::Pi(), TMath::Pi() ,  Nbin,TMin, TMax);
      HISTOS.Add(hphit);
      HISTO_POSITION["phitvtx"]=HISTOS.GetEntries()-1;

      //if (MVAMode)
      //   ht_MVA = new TH1D("htMVA", "Vertex, Passcut and MVA;t [s];Counts", Nbin, TMin, TMax);
  }
  return;
}

void TAGPlot::SetupTrackHistos()
{
  // reco helices
  TH1D* hNhel = new TH1D("hNhel","Reconstructed Helices",10,0.,10.);
  HISTOS.Add(hNhel);
  HISTO_POSITION[hNhel->GetName()]=HISTOS.GetEntries()-1;

  TH1D* hhD = new TH1D("hhD","Hel D;[mm]",200,-100.,100.);
  HISTOS.Add(hhD);
  HISTO_POSITION[hhD->GetName()]=HISTOS.GetEntries()-1;
  // TH1D* hhc = new TH1D("hhc","Hel c;[mm^{-1}]",200,-1.e-2,1.e-2);
  // HISTOS.Add(hhc);
  // HISTO_POSITION[hhc->GetName()]=HISTOS.GetEntries()-1;
  TH1D* hhRc = new TH1D("hhRc","Hel Rc;[mm]",200,-2000.,2000.);
  HISTOS.Add(hhRc);
  HISTO_POSITION[hhRc->GetName()]=HISTOS.GetEntries()-1;

  TH1D* hpt = new TH1D("hpt","Helix Transverse Momentum;p_{T} [MeV/c]",200,0.,1000.);
  HISTOS.Add(hpt);
  HISTO_POSITION[hpt->GetName()]=HISTOS.GetEntries()-1;
  TH1D* hpz = new TH1D("hpz","Helix Longitudinal Momentum;p_{Z} [MeV/c]",500,-1000.,1000.);
  HISTOS.Add(hpz);
  HISTO_POSITION[hpz->GetName()]=HISTOS.GetEntries()-1;
  TH1D* hpp = new TH1D("hpp","Helix Total Momentum;p_{tot} [MeV/c]",200,0.,1000.);
  HISTOS.Add(hpp);
  HISTO_POSITION[hpp->GetName()]=HISTOS.GetEntries()-1;
  TH2D* hptz = new TH2D("hptz","Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		  100,0.,1000.,200,-1000.,1000.);
  HISTOS.Add(hptz);
  HISTO_POSITION[hptz->GetName()]=HISTOS.GetEntries()-1;

  // reco helices spacepoints
  TH2D* hhspxy = new TH2D("hhspxy","Spacepoints in Helices;x [mm];y [mm]",
		    100,-190.,190.,100,-190.,190.);
  hhspxy->SetStats(kFALSE);
  HISTOS.Add(hhspxy);
  HISTO_POSITION[hhspxy->GetName()]=HISTOS.GetEntries()-1;
  TH2D* hhspzr = new TH2D("hhspzr","Spacepoints in Helices;z [mm];r [mm]",
		    600,-1200.,1200.,60,109.,174.);
  hhspzr->SetStats(kFALSE);
  HISTOS.Add(hhspzr);
  HISTO_POSITION[hhspzr->GetName()]=HISTOS.GetEntries()-1;
  TH2D* hhspzp = new TH2D("hhspzp","Spacepoints in Helices;z [mm];#phi [deg]",
		    600,-1200.,1200.,180,0.,360.);
  hhspzp->SetStats(kFALSE);
  HISTOS.Add(hhspzp);
  HISTO_POSITION[hhspzp->GetName()]=HISTOS.GetEntries()-1;

  // TH2D* hhsprp = new TH2D("hhsprp","Spacepoints in Helices;#phi [deg];r [mm]",
  // 		    180,0.,TMath::TwoPi(),200,108.,175.);
  // hhsprp->SetStats(kFALSE);
  // HISTOS.Add(hhsprp);
  // HISTO_POSITION[hhsprp->GetName()]=HISTOS.GetEntries()-1;

  // used helices
  TH1D* hNusedhel = new TH1D("hNusedhel","Used Helices",10,0.,10.);
  HISTOS.Add(hNusedhel);
  HISTO_POSITION[hNusedhel->GetName()]=HISTOS.GetEntries()-1;

  TH1D* huhD = new TH1D("huhD","Used Hel D;[mm]",200,-100.,100.);
  HISTOS.Add(huhD);
  HISTO_POSITION[huhD->GetName()]=HISTOS.GetEntries()-1;
  // TH1D* huhc = new TH1D("huhc","Used Hel c;[mm^{-1}]",200,-1.e-2,1.e-2);
  // HISTOS.Add(huhc);
  // HISTO_POSITION[huhc->GetName()]=HISTOS.GetEntries()-1;
  TH1D* huhRc = new TH1D("huhRc","Used Hel Rc;[mm]",200,-2000.,2000.);
  HISTOS.Add(huhRc);
  HISTO_POSITION[huhRc->GetName()]=HISTOS.GetEntries()-1;
  
  TH1D* huhpt = new TH1D("huhpt","Used Helix Transverse Momentum;p_{T} [MeV/c]",200,0.,1000.);
  HISTOS.Add(huhpt);
  HISTO_POSITION[huhpt->GetName()]=HISTOS.GetEntries()-1;
  TH1D* huhpz = new TH1D("huhpz","Used Helix Longitudinal Momentum;p_{Z} [MeV/c]",500,-1000.,1000.);
  HISTOS.Add(huhpz);
  HISTO_POSITION[huhpz->GetName()]=HISTOS.GetEntries()-1;
  TH1D* huhpp = new TH1D("huhpp","Used Helix Total Momentum;p_{tot} [MeV/c]",200,0.,1000.);
  HISTOS.Add(huhpp);
  HISTO_POSITION[huhpp->GetName()]=HISTOS.GetEntries()-1;
  TH2D* huhptz = new TH2D("huhptz","Used Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		    100,0.,1000.,200,-1000.,1000.);
  HISTOS.Add(huhptz);
  HISTO_POSITION[huhptz->GetName()]=HISTOS.GetEntries()-1;

  // used helices spacepoints
  TH2D* huhspxy = new TH2D("huhspxy","Spacepoints in Used Helices;x [mm];y [mm]",
		     100,-190.,190.,100,-190.,190.);
  huhspxy->SetStats(kFALSE);
  HISTOS.Add(huhspxy);
  HISTO_POSITION[huhspxy->GetName()]=HISTOS.GetEntries()-1;
  TH2D* huhspzr = new TH2D("huhspzr","Spacepoints in Used Helices;z [mm];r [mm]",
		     600,-1200.,1200.,60,109.,174.);
  huhspzr->SetStats(kFALSE);
  HISTOS.Add(huhspzr);
  HISTO_POSITION[huhspzr->GetName()]=HISTOS.GetEntries()-1;
  TH2D* huhspzp = new TH2D("huhspzp","Spacepoints in Used Helices;z [mm];#phi [deg]",
		     600,-1200.,1200.,180,0.,360.);
  huhspzp->SetStats(kFALSE);
  HISTOS.Add(huhspzp);
  HISTO_POSITION[huhspzp->GetName()]=HISTOS.GetEntries()-1;
  // TH2D* huhsprp = new TH2D("huhsprp","Spacepoints in Used Helices;#phi [deg];r [mm]",
  // 		     180,0.,TMath::TwoPi(),200,108.,175.);
  // huhsprp->SetStats(kFALSE);
  // HISTOS.Add(huhsprp);
  // HISTO_POSITION[huhsprp->GetName()]=HISTOS.GetEntries()-1;
}

void SetUpBarHistos()
{
   //Get some cool looking bar plots going!
}

void TAGPlot::FillHisto()
{
   if (TMin<0 && TMax<0.) AutoTimeRange();
   ClearHisto();
   SetUpHistograms();
  
   //Fill SIS histograms
  
  
   for (UInt_t i=0; i<ChronoPlotEvents.size(); i++)
   {
      /*if (trig < 0.)
      {
         std::cout << "Warning: TAGPlot->SetSISChannels(runNumber) should have been called before AddChronoPlotEvent" << std::endl;
         SetSISChannels(ChronoPlotEvents[i].runNumber);
      }*/
      Double_t time = ChronoPlotEvents[i].t;
      if (fabs(TMax-TMin)<SCALECUT) time=time*1000.;
      ChronoChannel Channel = ChronoPlotEvents[i].Chrono_Channel;
      Int_t CountsInChannel = ChronoPlotEvents[i].Counts;
 
      if (Channel == top)
         if (HISTO_POSITION.count("top_pm"))
            ((TH1D*)HISTOS.At(HISTO_POSITION.at("top_pm")))->Fill(time,CountsInChannel);

      if (Channel == bottom)
         if (HISTO_POSITION.count("bot_pm"))
            ((TH1D*)HISTOS.At(HISTO_POSITION.at("bot_pm")))->Fill(time,CountsInChannel);

      if (Channel == sipmad)
         if (HISTO_POSITION.count("aandd_pm"))
            ((TH1D*)HISTOS.At(HISTO_POSITION.at("aandd_pm")))->Fill(time,CountsInChannel);

      if (Channel == sipmcf)
         if (HISTO_POSITION.count("candf_pm"))
            ((TH1D*)HISTOS.At(HISTO_POSITION.at("candf_pm")))->Fill(time,CountsInChannel);

      if (Channel == TPC_TRIG)
         if (HISTO_POSITION.count("TPC_TRIG"))
            ((TH1D*)HISTOS.At(HISTO_POSITION.at("TPC_TRIG")))->Fill(time,CountsInChannel);
      
      /*
      if (Channel == trig)
         ht_IO32->Fill(time, CountsInChannel);
      else if (Channel == trig_nobusy)
         ht_IO32_notbusy->Fill(time, CountsInChannel);
      else if (Channel == atom_or)
         ht_ATOM_OR->Fill(time, CountsInChannel);
      else if (Channel == Beam_Injection)
         Injections.push_back(time);
      else if (Channel == Beam_Ejection)
         Ejections.push_back(time);
      else if (Channel == CATStart || Channel == RCTStart || Channel == ATMStart)
         DumpStarts.push_back(time);
      else if (Channel == CATStop || Channel == RCTStop || Channel == ATMStop)
         DumpStops.push_back(time);
      else std::cout <<"Unconfigured SIS channel in TAlhaPlot"<<std::endl;*/
  }
  
  //Fill Vertex Histograms
  
  
   TVector3 vtx;
   for (UInt_t i=0; i<VertexEvents.size(); i++)
   {
      Double_t time = VertexEvents[i].t;
      if (time<TMin) continue;
      if (time>TMax) continue; //Cannot assume events are in order... cannot use break
      if (fabs(TMax-TMin)<SCALECUT) time=time*1000.;
      vtx=TVector3(VertexEvents[i].x,VertexEvents[i].y,VertexEvents[i].z);

     if (HISTO_POSITION.count("tTPC"))
     {
        ((TH1D*)HISTOS.At(HISTO_POSITION.at("tTPC")))->Fill(time);
     }
      
     if (HISTO_POSITION.count("tbar")  && VertexEvents[i].NBars> BarMultiplicityCut)
     {
        ((TH1D*)HISTOS.At(HISTO_POSITION.at("tbar")))->Fill(time);
     }
      
     Int_t CutsResult=VertexEvents[i].CutsResult;
     if (MVAMode>0)
     {
        if (CutsResult & 1)//Passed cut result!
        {
           if (HISTO_POSITION.count("tvtx"))
              ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Fill(time);
        }
        if (CutsResult & 2)
        {
           if (HISTO_POSITION.count("tvtx"))
              ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Fill(time);
        }
        else
           continue; //Don't draw vertex if it tails MVA cut
     }
     else
     {
        if (gApplyCuts)
        {
           if (CutsResult & 1)//Passed cut result!
           {
              if (HISTO_POSITION.count("tvtx"))
                 ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Fill(time);
           }
           else
              continue;
        }
        else
        {
           if ( VertexEvents[i].VertexStatus > 0) 
           {
              if (HISTO_POSITION.count("tvtx"))
                 ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Fill(time);
           }
           else 
              continue;
        }
        if (VertexEvents[i].VertexStatus <= 0) continue; //Don't draw invaid vertices
      }
      if (HISTO_POSITION.count("phivtx"))
         ((TH1D*)HISTOS.At(HISTO_POSITION.at("phivtx")))->Fill(vtx.Phi());
      if (HISTO_POSITION.count("zphivtx"))
         ((TH2D*)HISTOS.At(HISTO_POSITION.at("zphivtx")))->Fill(vtx.Z(), vtx.Phi());
      if (HISTO_POSITION.count("phitvtx"))
         ((TH2D*)HISTOS.At(HISTO_POSITION.at("phitvtx")))->Fill(vtx.Phi(),time);
      if (HISTO_POSITION.count("xyvtx"))
         ((TH2D*)HISTOS.At(HISTO_POSITION.at("xyvtx")))->Fill(vtx.X(), vtx.Y());
      if (HISTO_POSITION.count("zvtx"))
         ((TH1D*)HISTOS.At(HISTO_POSITION.at("zvtx")))->Fill(vtx.Z());
      if (HISTO_POSITION.count("rvtx"))
         ((TH1D*)HISTOS.At(HISTO_POSITION.at("rvtx")))->Fill(vtx.Perp());
      if (HISTO_POSITION.count("zrvtx"))
         ((TH2D*)HISTOS.At(HISTO_POSITION.at("zrvtx")))->Fill(vtx.Z(), vtx.Perp());
      if (HISTO_POSITION.count("ztvtx"))
         ((TH2D*)HISTOS.At(HISTO_POSITION.at("ztvtx")))->Fill(vtx.Z(), time);
   }
   if (HISTO_POSITION.count("rvtx"))
   {
      TH1D* hr=(TH1D*)HISTOS.At(HISTO_POSITION.at("rvtx"));
      TH1D *hrdens = (TH1D *)hr->Clone("radial density");
      hrdens->Sumw2();
      TF1 *fr = new TF1("fr", "x", -100, 100);
      hrdens->Divide(fr);
      hrdens->Scale(hr->GetBinContent(hr->GetMaximumBin()) / hrdens->GetBinContent(hrdens->GetMaximumBin()));
      hrdens->SetMarkerColor(kRed);
      hrdens->SetLineColor(kRed);
      delete fr;
      HISTOS.Add(hrdens);
      HISTO_POSITION["rdens"]=HISTOS.GetEntries()-1;
   }

}

void TAGPlot::FillTrackHisto()
{ 
  if( !fPlotTracks )
    {
      std::cerr<<"TAGPlot::FillTrackHisto() histograms for tracks not created!"<<std::endl;
      return;
    }
  // else
  //   std::cout<<"TAGPlot::FillTrackHisto()"<<std::endl;
  std::cout<<"TAGPlot::FillTrackHisto() Number of histos: "<<HISTOS.GetEntries()<<std::endl;

  for(auto it = HelixEvents.begin(); it != HelixEvents.end(); ++it)
    {
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("hhD")))->Fill(it->parD);
      //      ((TH1D*)HISTOS.At(HISTO_POSITION.at("hhc")))->Fill(it->Curvature);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("hhRc")))->Fill(it->Curvature);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("hpt")))->Fill(it->pT);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("hpz")))->Fill(it->pZ);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("hpp")))->Fill(it->pTot);
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("hptz")))->Fill(it->pT,it->pZ);
    }

  for(auto it = SpacePointHelixEvents.begin(); it != SpacePointHelixEvents.end(); ++it)
    {
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhspxy")))->Fill(it->x,it->y);
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhspzr")))->Fill(it->z,it->r);
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhspzp")))->Fill(it->z,it->p);
      //      ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhsprp")))->Fill(it->r,it->p);
    }

  for(auto it = UsedHelixEvents.begin(); it != UsedHelixEvents.end(); ++it)
    {
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhD")))->Fill(it->parD);
      //((TH1D*)HISTOS.At(HISTO_POSITION.at("huhc")))->Fill(it->Curvature);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhRc")))->Fill(it->Curvature);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhpt")))->Fill(it->pT);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhpz")))->Fill(it->pZ);
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhpp")))->Fill(it->pTot);
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhptz")))->Fill(it->pT,it->pZ);
    }

  for (auto it = VertexEvents.begin(); it != VertexEvents.end(); ++it)
   {  
     ((TH1D*)HISTOS.At(HISTO_POSITION.at("hNhel")))->Fill( double(it->nTracks) );
     ((TH1D*)HISTOS.At(HISTO_POSITION.at("hNusedhel")))->Fill(double(it->nHelices));
   }

  for(auto it = SpacePointUsedHelixEvents.begin(); it != SpacePointUsedHelixEvents.end(); ++it)
    {
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhspxy")))->Fill(it->x,it->y);
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhspzr")))->Fill(it->z,it->r);
      ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhspzp")))->Fill(it->z,it->p);
      //      ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhsprp")))->Fill(it->r,it->p);
    }
}




TCanvas *TAGPlot::Canvas(TString Name)
{
   FillHisto();
   std::cout<<"Number of Vtx Histos: "<<HISTOS.GetEntries()<<std::endl;
   TCanvas *cVTX = new TCanvas(Name, Name, 1800, 1000);
   //Scale factor to scale down to ms:
   Double_t tFactor=1.;
   if (fabs(TMax-TMin)<SCALECUT) tFactor=1000.;
   cVTX->Divide(4, 2);

   if (gLegendDetail >= 1)
   {
      gStyle->SetOptStat(11111);
   }
   else
   {
      gStyle->SetOptStat("ni");	//just like the knights of the same name
   }

   cVTX->cd(1);

   if (HISTO_POSITION.count("zvtx"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("zvtx")))->Draw("HIST E1");

   cVTX->cd(2); // Z-counts (with electrodes?)4
   TVirtualPad *cVTX_1 = cVTX->cd(2);
   gPad->Divide(1, 2);
   cVTX_1->cd(1);
   //cVTX->cd(2)->SetFillStyle(4000 );
      //cVTX->cd(1)->SetFillStyle(4000 );
   // R-counts
   if (HISTO_POSITION.count("rvtx"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("rvtx")))->Draw("HIST E1");

   if (HISTO_POSITION.count("rdens"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("rdens")))->Draw("HIST E1 SAME");

   //((TH1D *)hh[VERTEX_HISTO_RDENS])->Draw("HIST E1 SAME");
   TPaveText *rdens_label = new TPaveText(0.6, 0.8, 0.90, 0.85, "NDC NB");
   rdens_label->AddText("radial density [arbs]");
   rdens_label->SetTextColor(kRed);
   rdens_label->SetFillStyle(0);
   rdens_label->SetLineStyle(0);
   rdens_label->Draw();
   cVTX_1->cd(2);

   if (HISTO_POSITION.count("phivtx"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("phivtx")))->Draw("HIST E1");

   cVTX->cd(3); // T-counts
   //cVTX->cd(3)->SetFillStyle(4000 );
   //((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->Draw("HIST"); // io32-notbusy = readouts
   //((TH1D *)hh[VERTEX_HISTO_IO32])->Draw("HIST SAME");    // io32
   //((TH1D *)hh[VERTEX_HISTO_ATOM_OR])->Draw("HIST SAME");    // ATOM OR PMTs


   if (HISTO_POSITION.count("TPC_TRIG"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("TPC_TRIG")))->Draw("HIST");

   if (HISTO_POSITION.count("tvtx"))     //verticies
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Draw("HIST SAME");

   if (HISTO_POSITION.count("tbar"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("tbar")))->Draw("HIST SAME");

   if (HISTO_POSITION.count("tTPC"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("tTPC")))->Draw("HIST SAME");

   if (HISTO_POSITION.count("top_pm"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("top_pm")))->Draw("HIST SAME");

   if (HISTO_POSITION.count("bot_pm"))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at("bot_pm")))->Draw("HIST SAME");

   //((TH1D *)hh[VERTEX_HISTO_VF48])->Draw("HIST SAME");    //io32 sistime
   if (MVAMode)
      if (HISTO_POSITION.count("tmva"))
         ((TH1D*)HISTOS.At(HISTO_POSITION.at("tmva")))->Draw("HIST SAME");


   //auto legend = new TLegend(0.1,0.7,0.48,0.9);(0.75, 0.8, 1.0, 0.95
   //auto legend = new TLegend(1., 0.7, 0.45, 1.);//, "NDC NB");
   auto legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
   char line[201];
   TH1D* TPC=((TH1D*)HISTOS.At(HISTO_POSITION.at("TPC_TRIG")));
   snprintf(line, 200, "TPC_TRIG: %5.0lf", TPC->Integral());
   //   snprintf(line, 200, "TPC_TRIG: %5.0lf", TPC->Integral("width"));
   legend->AddEntry(TPC, line, "f");
   TH1D* top=((TH1D*)HISTOS.At(HISTO_POSITION.at("top_pm")));
   snprintf(line, 200, "top_pm: %5.0lf", top->Integral());
   //snprintf(line, 200, "top_pm: %5.0lf", top->Integral("width"));
   legend->AddEntry(top, line, "f");
   TH1D* bot=((TH1D*)HISTOS.At(HISTO_POSITION.at("bot_pm")));
   snprintf(line, 200, "bottom pm: %5.0lf", bot->Integral());
   //snprintf(line, 200, "bottom pm: %5.0lf", bot->Integral("width"));
   legend->AddEntry(bot, line, "f");
   
   TH1D* tbar=((TH1D*)HISTOS.At(HISTO_POSITION.at("tbar")));
   snprintf(line, 200, "> %d bars: %5.0lf", BarMultiplicityCut, tbar->Integral());
   std::cout<<line<<std::endl;
   //snprintf(line, 200, "bottom pm: %5.0lf", bot->Integral("width"));
   legend->AddEntry(tbar, line, "f");
   
   //snprintf(line, 200, "TPC Events: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_T])->Integral());
   //legend->AddEntry(hh[VERTEX_HISTO_VF48], line, "f");
   if (MVAMode)
   {
      if (HISTO_POSITION.count("tvtx"))     //verticies
         ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Draw("HIST SAME");

      snprintf(line, 200, "Pass Cuts: %5.0lf", ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Integral());
      legend->AddEntry((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")), line, "f");
      snprintf(line, 200, "Pass MVA (rfcut %0.1f): %5.0lf", grfcut, ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Integral());
      legend->AddEntry((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")), line, "f");
   }
   else
   {
      if (gApplyCuts)
         snprintf(line, 200, "Pass Cuts: %5.0lf", ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Integral());
      else
         snprintf(line, 200, "Vertices: %5.0lf", ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Integral());
      //      snprintf(line, 200, "Vertices: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_T])->Integral());
      legend->AddEntry((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")), line, "f");
      legend->SetFillColor(kWhite);
      legend->SetFillStyle(1001);
    //std::cout <<"Drawing lines"<<std::endl;
   /*
    for (UInt_t i = 0; i < Injections.size(); i++)
    {
      TLine *l = new TLine(Injections[i]*tFactor, 0., Injections[i]*tFactor, ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetMaximum());
      l->SetLineColor(6);
      l->Draw();
      if (i == 0)
        legend->AddEntry(l, "AD fill", "l");
    }
    for (UInt_t i = 0; i < Ejections.size(); i++)
    {
      TLine *l = new TLine(Ejections[i]*tFactor, 0., Ejections[i]*tFactor, ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetMaximum());
      l->SetLineColor(7);
      l->Draw();
      if (i == 0)
        legend->AddEntry(l, "Beam to ALPHA", "l");
    }
    for (UInt_t i = 0; i < DumpStarts.size(); i++)
    {
      if (DumpStarts.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *l = new TLine(DumpStarts[i]*tFactor, 0., DumpStarts[i]*tFactor, ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetMaximum());
      //l->SetLineColor(7);
      l->SetLineColorAlpha(kGreen, 0.35);
      //l->SetFillColorAlpha(kGreen,0.35);
      l->Draw();
      if (i == 0)
        legend->AddEntry(l, "Dump Start", "l");
    }
    for (UInt_t i = 0; i < DumpStops.size(); i++)
    {
      if (DumpStops.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *l = new TLine(DumpStops[i]*tFactor, 0., DumpStops[i]*tFactor, ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetMaximum());
      //l->SetLineColor(7);
      l->SetLineColorAlpha(kRed, 0.35);
      l->Draw();
      if (i == 0)
        legend->AddEntry(l, "Dump Stop", "l");
    }*/
  }

  // legend->AddEntry("f1","Function abs(#frac{sin(x)}{x})","l");
  // legend->AddEntry("gr","Graph with error bars","lep");
  legend->Draw();
  cVTX->cd(4);
  // X-Y-counts
  //cVTX->cd(4)->SetFillStyle(4000 );
  if (HISTO_POSITION.count("xyvtx"))     //verticies
     ((TH1D*)HISTOS.At(HISTO_POSITION.at("xyvtx")))->Draw("colz");

  cVTX->cd(5);
  // Z-R-counts
  //cVTX->cd(5)->SetFillStyle(4000 );
  if (HISTO_POSITION.count("phitvtx"))     //verticies
     ((TH1D*)HISTOS.At(HISTO_POSITION.at("phitvtx")))->Draw("colz");

  cVTX->cd(6);
  // Z-T-counts
  //cVTX->cd(6)->SetFillStyle(4000 );
  if (HISTO_POSITION.count("ztvtx"))
     ((TH1D*)HISTOS.At(HISTO_POSITION.at("ztvtx")))->Draw("colz");

  cVTX->cd(7);
  // phi counts
  //cVTX->cd(7)->SetFillStyle(4000 );
  /*((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->SetStats(0);
  ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetCumulative()->Draw("HIST");
  ((TH1D *)hh[VERTEX_HISTO_IO32])->GetCumulative()->Draw("HIST SAME");*/
  if (HISTO_POSITION.count("tvtx"))     //verticies
     ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->GetCumulative()->Draw("HIST SAME");
  
  //((TH1D *)hh[VERTEX_HISTO_VF48])->GetCumulative()->Draw("HIST SAME");

  if (MVAMode && HISTO_POSITION.count("tvtx"))
  {
    ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->GetCumulative()->Draw("HIST SAME");
    TH1 *h = ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->GetCumulative();
    {
      //Draw line at halfway point
      Double_t Max = h->GetBinContent(h->GetMaximumBin());
      Double_t Tmax = h->GetXaxis()->GetXmax();
      for (Int_t i = 0; i < h->GetMaximumBin(); i++)
      {
        if (h->GetBinContent(i) > Max / 2.)
        {
          TLine *half = new TLine(TMax*tFactor * (Double_t)i / (Double_t)Nbin, 0., Tmax * (Double_t)i / (Double_t)Nbin, Max / 2.);
          half->SetLineColor(kViolet);
          half->Draw();
          break;
        }
      }
    }
  }
  else
  {
     //Draw line at halfway point
     if (HISTO_POSITION.count("tvtx"))     //verticies
     {
        TH1 *h = ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->GetCumulative();
        Double_t Max = h->GetBinContent(h->GetMaximumBin());
        Double_t Tmax = h->GetXaxis()->GetXmax();
        for (Int_t i = 0; i < h->GetMaximumBin(); i++)
        {
           if (h->GetBinContent(i) > Max / 2.)
           {
              TLine *half = new TLine(TMax*tFactor * (Double_t)i / (Double_t)Nbin, 0., Tmax * (Double_t)i / (Double_t)Nbin, Max / 2.);
              half->SetLineColor(kBlue);
              half->Draw();
              break;
           }
        }
     }
  }
  //IO32_NOTBUSY Halfway point
  /*
  TH1 *h2 = ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetCumulative();
  //Draw line at halfway point
  Double_t Max = h2->GetBinContent(h2->GetMaximumBin());
  Double_t Tmax = h2->GetXaxis()->GetXmax();
  for (Int_t i = 0; i < h2->GetMaximumBin(); i++)
  {
    if (h2->GetBinContent(i) > Max / 2.)
    {
      TLine *half = new TLine(TMax *tFactor* (Double_t)i / (Double_t)Nbin, 0., Tmax * (Double_t)i / (Double_t)Nbin, Max / 2.);
      half->SetLineColor(kRed);
      half->Draw();
      break;
    }
  }
*/
  cVTX->cd(8);
  // Z-PHI-counts
  //cVTX->cd(8)->SetFillStyle(4000 );
  if (HISTO_POSITION.count("zphivtx"))     //verticies
     ((TH1D*)HISTOS.At(HISTO_POSITION.at("zphivtx")))->Draw("colz");
  if (gApplyCuts)
  {
    cVTX->cd(1);
    TPaveText *applycuts_label = new TPaveText(0., 0.95, 0.20, 1.0, "NDC NB");
    if (MVAMode>0)
      applycuts_label->AddText("RF cut applied");
    else
      applycuts_label->AddText("Cuts applied");
    applycuts_label->SetTextColor(kRed);
    applycuts_label->SetFillColor(kWhite);
    applycuts_label->Draw();
  };
  //TLatex* runs_label = new  TLatex(-32.5,-.0625, 32.5, 5.6, "NDC NB");
  //
  cVTX->cd(0);
  TString run_txt = "Run(s): ";
  std::sort(Runs.begin(), Runs.end());
  for (UInt_t i = 0; i < Runs.size(); i++)
  {
    //std::cout <<"Run: "<<Runs[i] <<std::endl;
    if (i > 0)
      run_txt += ",";
    run_txt += Runs[i];
  }
  run_txt += " ";
  run_txt += Nbin;
  run_txt += " bins";
  /*if (gZcutMin > -998.)
    {
      run_txt += " gZcutMin=";
      run_txt += gZcutMin;
    }
    if (gZcutMax < 998.)
    {
      run_txt += " gZcutMax=";
      run_txt += gZcutMax;
    }*/
  //runs_label->AddText(run_txt);
  TLatex *runs_label = new TLatex(0., 0., run_txt);
  runs_label->SetTextSize(0.016);
  //runs_label->SetTextColor(kRed);
  //runs_label->SetFillColor(kWhite);
  //runs_label->SetFillStyle(1001);
  runs_label->Draw();

  std::cout<<run_txt<<std::endl;
  return cVTX;
}

TCanvas* TAGPlot::DrawTrackHisto(TString Name)
{
  if( !fPlotTracks )
    {
      std::cerr<<"TAGPlot::DrawTrackHisto() histograms for tracks not created!"<<std::endl;
      return 0;
    }
  // else
  //   std::cout<<"TAGPlot::DrawTrackHisto()"<<std::endl;
  SetupTrackHistos();
  FillTrackHisto();

  TCanvas* ct = new TCanvas(Name,Name,2000,1800);
  ct->Divide(5,4);
  
  ct->cd(1);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hNhel")))->Draw("hist");

  ct->cd(2);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hhD")))->Draw("hist");
  ct->cd(3);
  //  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hhc")))->Draw("hist");
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hhRc")))->Draw("hist");
  ct->cd(4);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hpt")))->Draw("hist");
  ct->cd(5);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hpz")))->Draw("hist");
  ct->cd(6);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hpp")))->Draw("hist");
  ct->cd(7);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("hptz")))->Draw("colz");

  ct->cd(8);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhspxy")))->Draw("colz");
  ct->cd(9);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhspzr")))->Draw("colz");
  ct->cd(10);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhspzp")))->Draw("colz");
  //  ((TH2D*)HISTOS.At(HISTO_POSITION.at("hhsprp")))->Draw("colz");
  
  ct->cd(11);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("hNusedhel")))->Draw("hist");

  ct->cd(12);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhD")))->Draw("hist");
  ct->cd(13);
  //((TH1D*)HISTOS.At(HISTO_POSITION.at("huhc")))->Draw("hist");
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhRc")))->Draw("hist");
  ct->cd(14);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhpt")))->Draw("hist");
  ct->cd(15);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhpz")))->Draw("hist");
  ct->cd(16);
  ((TH1D*)HISTOS.At(HISTO_POSITION.at("huhpp")))->Draw("hist");
  ct->cd(17);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhptz")))->Draw("colz");

  ct->cd(18);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhspxy")))->Draw("colz");
  ct->cd(19);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhspzr")))->Draw("colz");
  ct->cd(20);
  ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhspzp")))->Draw("colz");
  //  ((TH2D*)HISTOS.At(HISTO_POSITION.at("huhsprp")))->Draw("colz");
  
  return ct;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
