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


  trig=-1;// = -1;
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

  SaveCSV<<"RunNumber,Event Number,RunTime,VertexStatus,x,y,z,t,Passed Cut,Passed MVA"<<std::endl;
  for (UInt_t i=0; i<VertexEvents.size(); i++)
  {
    //Skip events that fail cuts if only saving passed cuts
    if (PassedCutOnly && !VertexEvents[i].CutsResult ) continue; 
    SaveCSV<<VertexEvents[i].runNumber<<",";
    SaveCSV<<VertexEvents[i].EventNo<<",";
    SaveCSV<<VertexEvents[i].RunTime<<",";
    SaveCSV<<VertexEvents[i].VertexStatus<<",";
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
  SISChannels.clear();
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


void TAGPlot::AddStoreEvent(TStoreEvent *event, Double_t StartOffset)
{
  VertexEvent Event;
  TVector3 vtx = event->GetVertex();
  Event.EventNo= event->GetEventNumber();

  Event.RunTime= event->GetTimeOfEvent();
  Event.t= event->GetTimeOfEvent() - StartOffset;
  Event.VertexStatus=event->GetVertexStatus();
  Event.x=vtx.X();
  Event.y=vtx.Y();
  Event.z=vtx.Z();
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
  return;
}

void TAGPlot::AddChronoEvent(TChrono_Event *event, Double_t StartOffset)
{
  ChronoPlotEvent Event;
  Event.runNumber     =0;//event->GetRunNumber();
  Event.Counts        =event->GetCounts();
  Event.Chrono_Board  =event->GetBoardIndex();
  Event.Chrono_Channel=event->GetChannel();
  Event.RunTime       =event->GetRunTime();
  Event.t             =event->GetRunTime()-StartOffset;
  ChronoPlotEvents.push_back(Event);
}

//Maybe dont have this function... lets see...
void TAGPlot::AddEvents(Int_t runNumber, char *description, Int_t repetition, Double_t Toffset, Bool_t zeroTime)
{
  Double_t start_time = MatchEventToTime(runNumber, "startDump", description, repetition);
  Double_t stop_time = MatchEventToTime(runNumber, "stopDump", description, repetition);
  AddEvents(runNumber, start_time, stop_time, Toffset, zeroTime);
}

void TAGPlot::AddEvents(Int_t runNumber, Double_t tmin, Double_t tmax, Double_t Toffset, Bool_t zeroTime)
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
  if (trig < 0.)
    SetChronoChannels(runNumber);
  //cout <<"Sis channels set."<<endl;
  //Add Silicon Events:
  //cout <<"Adding Silicon Events"<<endl;

  if (std::find(Runs.begin(), Runs.end(), runNumber) == Runs.end() || Runs.size() == 0)
  {
    //std::cout <<"Adding runNo"<<std::endl;
    Runs.push_back(runNumber);
  }
  TStoreEvent *store_event = new TStoreEvent();
  Double_t run_time;
  TTree *t0 = Get_StoreEvent_Tree(runNumber);
  t0->SetBranchAddress("StoredEvent", &store_event);
  //SPEED THIS UP BY PREPARING FIRST ENTRY!
  for (Int_t i = 0; i < t0->GetEntries(); ++i)
  {
    t0->GetEntry(i);
    //store_event->Print();
    if (!store_event)
    {
      std::cout<<"NULL TStore event: Probably more OfficialTimeStamps than events"<<std::endl;
      break;
    }
    run_time = store_event->GetTimeOfEvent();
    if (run_time <= tmin)
    {
      store_event->Reset();
      continue;
    }
    if (run_time > tmax)
    {
      store_event->Reset();
      break;
    }
    if (zeroTime)
      AddStoreEvent(store_event, Toffset + tmin);
    else
      AddStoreEvent(store_event, Toffset);
  }
  if (store_event) delete store_event;
  delete t0;
  //Add SIS Events:
  //std::cout <<"Adding SIS Events"<<std::endl;
  //SetSISChannels(runNumber);

/*
  for (UInt_t j=0; j<SISChannels.size(); j++)
  {
    TTree *sis_tree = TRootUtils::Get_Sis_Tree(runNumber, SISChannels[j]);
    TChronoPlotEvent *sis_event = new TChronoPlotEvent();
    sis_tree->SetBranchAddress("ChronoPlotEvent", &sis_event);
    for (Int_t i = 0; i < sis_tree->GetEntriesFast(); ++i)
    {
      sis_tree->GetEntry(i);
      run_time = sis_event->GetRunTime();
      if (run_time <= tmin)
        continue;
      if (run_time > tmax)
        break;
      if (zeroTime)
        AddChronoPlotEvent(sis_event, Toffset + tmin);
      else
        AddChronoPlotEvent(sis_event, Toffset);
    }
    delete sis_event;
    delete sis_tree;
  }
*/
}

void TAGPlot::SetChronoChannels(Int_t runNumber)
{
   //Silence compiler warning until this code is implemented
   runNumber=runNumber;
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


  //Add all valid SIS channels to a list for later:
  if (trig>0)           SISChannels.push_back(trig);
  if (trig_nobusy>0)    SISChannels.push_back(trig_nobusy);
  if (atom_or>0)        SISChannels.push_back(atom_or);
  if (Beam_Injection>0) SISChannels.push_back(Beam_Injection);
  if (Beam_Ejection>0)  SISChannels.push_back(Beam_Ejection);
  if (CATStart>0)       SISChannels.push_back(CATStart);
  if (CATStop>0)        SISChannels.push_back(CATStop);
  if (RCTStart>0)       SISChannels.push_back(RCTStart);
  if (RCTStop>0)        SISChannels.push_back(RCTStop);
  if (ATMStart>0)       SISChannels.push_back(ATMStart);
  if (ATMStop>0)        SISChannels.push_back(ATMStop);
  //std::cout <<"Trig:"<<trig<<std::endl;
  //std::cout <<"TrigNoBusy:"<<trig_nobusy<<std::endl;
  //std::cout <<"Beam Injection:"<<Beam_Injection<<std::endl;
  //std::cout <<"Beam Ejection:"<<Beam_Ejection<<std::endl;
  delete sisch;*/
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
   XMAX=YMAX=RMAX=50.;
   ZMAX=1300.;
   if (HISTOS.GetEntries()>0)
   {
      HISTOS.Delete();
      HISTO_POSITION.clear();
   }
   HISTOS.Add(new TH1D("zvtx", "Z Vertex;z [cm];events", Nbin, -ZMAX, ZMAX));
   HISTO_POSITION["zvtx"]=HISTOS.GetEntries()-1;

   TH1D* hr = new TH1D("rvtx", "R Vertex;r [cm];events", Nbin, 0., RMAX);
   hr->SetMinimum(0);
   HISTOS.Add(hr);
   HISTO_POSITION["rvtx"]=HISTOS.GetEntries()-1;

   TH1D* hphi = new TH1D("phivtx", "phi Vertex;phi [rad];events", Nbin, -TMath::Pi(), TMath::Pi());
   hphi->SetMinimum(0);
   HISTOS.Add(hphi);
   HISTO_POSITION["phivtx"]=HISTOS.GetEntries()-1;

   TH2D* hxy = new TH2D("xyvtx", "X-Y Vertex;x [cm];y [cm]", Nbin, -XMAX, XMAX, Nbin, -YMAX, YMAX);
   HISTOS.Add(hxy);
   HISTO_POSITION["xyvtx"]=HISTOS.GetEntries()-1;

   TH2D* hzr = new TH2D("zrvtx", "Z-R Vertex;z [cm];r [cm]", Nbin, -ZMAX, ZMAX, Nbin, 0., RMAX);
   HISTOS.Add(hzr);
   HISTO_POSITION["zrvtx"]=HISTOS.GetEntries()-1;

   TH2D* hzphi = new TH2D("zphivtx", "Z-Phi Vertex;z [cm];phi [rad]", Nbin, -ZMAX, ZMAX, Nbin, -TMath::Pi(), TMath::Pi());
   HISTOS.Add(hzphi);
   HISTO_POSITION["zphivtx"]=HISTOS.GetEntries()-1;

   if (ScaleAsMiliSeconds)
   {
      TH1D* ht = new TH1D("tvtx", "t Vertex;t [ms];events", Nbin, TMin*1000., TMax*1000.);
      HISTOS.Add(ht);
      HISTO_POSITION["tvtx"]=HISTOS.GetEntries()-1;

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
      HISTOS.Add(ht);
      HISTO_POSITION["tvtx"]=HISTOS.GetEntries()-1;

      TH2D* hzt = new TH2D("ztvtx", "Z-T Vertex;z [cm];t [s]", Nbin, -ZMAX, ZMAX, Nbin, TMin, TMax);
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

void TAGPlot::FillHisto()
{
   if (TMin<0 && TMax<0.) AutoTimeRange();
   ClearHisto();
   SetUpHistograms();
  
   //Fill SIS histograms
  /*
   for (UInt_t i=0; i<ChronoPlotEvents.size(); i++)
  {
    
    if (trig < 0.)
    {
      std::cout << "Warning: TAGPlot->SetSISChannels(runNumber) should have been called before AddChronoPlotEvent" << std::endl;
      SetSISChannels(ChronoPlotEvents[i].runNumber);
    }
    Double_t time = ChronoPlotEvents[i].t;
    if (fabs(TMax-TMin)<SCALECUT) time=time*1000.;
    Int_t Channel = ChronoPlotEvents[i].SIS_Channel;
    Int_t CountsInChannel = ChronoPlotEvents[i].Counts;
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
    else std::cout <<"Unconfigured SIS channel in TAlhaPlot"<<std::endl;
  }
  */
  
  //Fill Vertex Histograms
  
  
   TVector3 vtx;
   for (UInt_t i=0; i<VertexEvents.size(); i++)
   {
      Double_t time = VertexEvents[i].t;
      if (time<TMin) continue;
      if (time>TMax) continue; //Cannot assume events are in order... cannot use break
      if (fabs(TMax-TMin)<SCALECUT) time=time*1000.;
      vtx=TVector3(VertexEvents[i].x,VertexEvents[i].y,VertexEvents[i].z);

     if (HISTO_POSITION.count("tvtx"))
     {
        ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Fill(time);
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
        if (VertexEvents[i].VertexStatus != 1) continue; //Don't draw invaid vertices
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
}

TObjArray TAGPlot::GetHisto()
{
   FillHisto();
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
      HISTO_POSITION["rdens"]=HISTOS.GetEntries();
   }
/*
  ht_IO32->SetLineColor(kGreen);
  ht_IO32->SetMarkerColor(kGreen);
  ht_IO32->SetMinimum(0);
  ht_IO32_notbusy->SetMarkerColor(kRed);
  ht_IO32_notbusy->SetLineColor(kRed);
  ht_IO32_notbusy->SetMinimum(0);
  ht_IO32_sistime->SetLineColor(kAzure - 8);
  ht_IO32_sistime->SetMarkerColor(kAzure - 8);
  ht_IO32_sistime->SetMinimum(0);
  ht_ATOM_OR->SetLineColor(kMagenta - 9);
  ht_ATOM_OR->SetMarkerColor(kMagenta - 9);
  ht_ATOM_OR->SetMinimum(0);
*/
   TObjArray histos(VERTEX_HISTO_TMVA);

   if (HISTO_POSITION.count("tvtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("tvtx")),VERTEX_HISTO_T);
   if (HISTO_POSITION.count("phivtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("phivtx")),VERTEX_HISTO_PHI);
   if (HISTO_POSITION.count("zphivtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("zphivtx")),VERTEX_HISTO_ZPHI);
   if (HISTO_POSITION.count("phitvtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("phitvtx")),VERTEX_HISTO_TPHI);
   if (HISTO_POSITION.count("xyvtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("xyvtx")),VERTEX_HISTO_XY);
   if (HISTO_POSITION.count("zvtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("zvtx")),VERTEX_HISTO_Z);
   if (HISTO_POSITION.count("rvtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("rvtx")),VERTEX_HISTO_R);
   if (HISTO_POSITION.count("zrvtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("zrvtx")),VERTEX_HISTO_ZR);
   if (HISTO_POSITION.count("ztvtx"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("ztvtx")),VERTEX_HISTO_ZT);
   if (HISTO_POSITION.count("rdens"))
      histos.AddAt(HISTOS.At(HISTO_POSITION.at("rdens")),VERTEX_HISTO_RDENS);
/*  histos.AddAt(ht_IO32,VERTEX_HISTO_IO32);
  histos.AddAt(ht_IO32_notbusy,VERTEX_HISTO_IO32_NOTBUSY);
  histos.AddAt(ht_ATOM_OR,VERTEX_HISTO_ATOM_OR);
  histos.AddAt(hzphi,VERTEX_HISTO_ZPHI);
  histos.AddAt(hrdens,VERTEX_HISTO_RDENS);
  histos.AddAt(ht_IO32_sistime,VERTEX_HISTO_VF48);
  if (MVAMode)
  {
    ht_MVA->SetMarkerColor(kViolet);
    ht_MVA->SetLineColor(kViolet);
    ht_MVA->SetMinimum(0);
    histos.AddAt(ht_MVA,VERTEX_HISTO_TMVA);
  }*/
   return histos;
}

TCanvas *TAGPlot::Canvas(TString Name)
{
  TObjArray hh = GetHisto();
  TCanvas *cVTX = new TCanvas(Name, Name, 1600, 800);
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
  //cVTX->cd(1)->SetFillStyle(4000 );
  // R-counts
  ((TH1D *)hh[VERTEX_HISTO_R])->Draw("HIST E1");
  //((TH1D *)hh[VERTEX_HISTO_RDENS])->Draw("HIST E1 SAME");
  TPaveText *rdens_label = new TPaveText(0.6, 0.8, 0.90, 0.85, "NDC NB");
  rdens_label->AddText("radial density [arbs]");
  rdens_label->SetTextColor(kRed);
  rdens_label->SetFillStyle(0);
  rdens_label->SetLineStyle(0);
  rdens_label->Draw();

  cVTX->cd(2); // Z-counts (with electrodes?)4
  TVirtualPad *cVTX_1 = cVTX->cd(2);
  gPad->Divide(1, 2);
  cVTX_1->cd(1);
  //cVTX->cd(2)->SetFillStyle(4000 );
  ((TH1D *)hh[VERTEX_HISTO_Z])->Draw("HIST E1");
  cVTX_1->cd(2);

  ((TH1D *)hh[VERTEX_HISTO_PHI])->Draw("HIST E1");

  cVTX->cd(3); // T-counts
  //cVTX->cd(3)->SetFillStyle(4000 );
  //((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->Draw("HIST"); // io32-notbusy = readouts
  //((TH1D *)hh[VERTEX_HISTO_IO32])->Draw("HIST SAME");    // io32
  //((TH1D *)hh[VERTEX_HISTO_ATOM_OR])->Draw("HIST SAME");    // ATOM OR PMTs
  ((TH1D *)hh[VERTEX_HISTO_T])->Draw("HIST SAME");       //verticies
  //((TH1D *)hh[VERTEX_HISTO_VF48])->Draw("HIST SAME");    //io32 sistime
  if (MVAMode)
    ((TH1D *)hh[VERTEX_HISTO_TMVA])->Draw("HIST SAME"); //MVA results

  //auto legend = new TLegend(0.1,0.7,0.48,0.9);(0.75, 0.8, 1.0, 0.95
  //auto legend = new TLegend(1., 0.7, 0.45, 1.);//, "NDC NB");
  auto legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
  /*
  char line[201];
  snprintf(line, 200, "IO32 NotBusy: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->Integral());
  legend->AddEntry(hh[VERTEX_HISTO_IO32_NOTBUSY], line, "f");
  snprintf(line, 200, "ATOM_OR: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_ATOM_OR])->Integral());
  legend->AddEntry(hh[VERTEX_HISTO_ATOM_OR], line, "f");
  snprintf(line, 200, "IO32 sis time: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_IO32])->Integral());
  legend->AddEntry(hh[VERTEX_HISTO_IO32], line, "f");
  snprintf(line, 200, "IO32 vf48 time: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_VF48])->Integral());
  legend->AddEntry(hh[VERTEX_HISTO_VF48], line, "f");
  if (MVAMode)
  {
    snprintf(line, 200, "Pass Cuts: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_T])->Integral());
    legend->AddEntry(hh[VERTEX_HISTO_T], line, "f");
    snprintf(line, 200, "Pass MVA (rfcut %0.1f): %5.0lf", grfcut, ((TH1D *)hh[VERTEX_HISTO_TMVA])->Integral());
    legend->AddEntry(hh[VERTEX_HISTO_TMVA], line, "f");
  }
  else
  {
    if (gApplyCuts)
      snprintf(line, 200, "Pass Cuts: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_T])->Integral());
    else
      snprintf(line, 200, "vertices: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_T])->Integral());
    legend->AddEntry(hh[VERTEX_HISTO_T], line, "f");
    legend->SetFillColor(kWhite);
    legend->SetFillStyle(1001);
    //std::cout <<"Drawing lines"<<std::endl;

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
    }
  }
*/
  // legend->AddEntry("f1","Function abs(#frac{sin(x)}{x})","l");
  // legend->AddEntry("gr","Graph with error bars","lep");
  legend->Draw();
  cVTX->cd(4);
  // X-Y-counts
  //cVTX->cd(4)->SetFillStyle(4000 );
  ((TH2D *)hh[VERTEX_HISTO_XY])->Draw("colz");

  cVTX->cd(5);
  // Z-R-counts
  //cVTX->cd(5)->SetFillStyle(4000 );
  ((TH2D *)hh[VERTEX_HISTO_TPHI])->Draw("colz");

  cVTX->cd(6);
  // Z-T-counts
  //cVTX->cd(6)->SetFillStyle(4000 );
  ((TH2D *)hh[VERTEX_HISTO_ZT])->Draw("colz");

  cVTX->cd(7);
  // phi counts
  //cVTX->cd(7)->SetFillStyle(4000 );
  /*((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->SetStats(0);
  ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetCumulative()->Draw("HIST");
  ((TH1D *)hh[VERTEX_HISTO_IO32])->GetCumulative()->Draw("HIST SAME");*/
  ((TH1D *)hh[VERTEX_HISTO_T])->GetCumulative()->Draw("HIST SAME");
  //((TH1D *)hh[VERTEX_HISTO_VF48])->GetCumulative()->Draw("HIST SAME");

  if (MVAMode)
  {
    ((TH1D *)hh[VERTEX_HISTO_TMVA])->GetCumulative()->Draw("HIST SAME");
    TH1 *h = ((TH1D *)hh[VERTEX_HISTO_TMVA])->GetCumulative();
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
    TH1 *h = ((TH1D *)hh[VERTEX_HISTO_T])->GetCumulative();
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
  ((TH2D *)hh[VERTEX_HISTO_ZPHI])->Draw("colz");
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

  return cVTX;
}
