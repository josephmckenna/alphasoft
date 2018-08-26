#include "RootUtils.h"

Int_t gNbin=100;


TFile *Get_File(Int_t run_number, Bool_t die)
{
  TFile *f = NULL;
  TString file_name(getenv("AGRELEASE"));
  file_name += "/ana/output";
  if (run_number < 10000)
    file_name += "0";
  if (run_number < 1000)
    file_name += "0";
  if (run_number < 100)
    file_name += "0";
  if (run_number < 10)
    file_name += "0";
  file_name += run_number;
  file_name += ".root";


// ALPHA G EOS PATH NOT SETUP YET...
  TString EOS_name(getenv("EOS_MGM_URL"));
  EOS_name += "//eos/experiment/alpha/alphaTrees";
  EOS_name += "output";
  if (run_number < 10000)
    EOS_name += "0";
  if (run_number < 1000)
    EOS_name += "0";
  if (run_number < 100)
    EOS_name += "0";
  if (run_number < 10)
    EOS_name += "0";
  EOS_name += run_number;
  EOS_name += ".root";
  //Set pointer to file if already open (by checking TFile file names)

  f = (TFile *)gROOT->GetListOfFiles()->FindObject(file_name);
  if (f == NULL)
  {
    f = (TFile *)gROOT->GetListOfFiles()->FindObject(EOS_name);
    if (f != NULL)
      return f;
  }
  else
  {
    return f;
  }
  f = new TFile(file_name.Data(), "READ");
  TString hostname = gSystem->HostName();
  if (!f->IsOpen() && strcmp(hostname.Data(), "alphasvnchecker.cern.ch") != 0) //disable alphasvnchecker getting files from cloud
  {
    //If the file isnt found locally... find it remotely on EOS
    f = TFile::Open(EOS_name);
  }
  if (!f->IsOpen())
  //if(f==NULL)
  {
    if (die)
    {
      Error("Get_File", "\033[33mCould not open tree file for run %d\033[00m", run_number);
      TObject *dum = NULL;
      dum->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
    }
    else
      return NULL;
  }
  return f;
}


TTree* Get_Chrono_Tree(Int_t runNumber, Int_t Chronobox, Int_t ChronoChannel)
{
  TFile* f=Get_File(runNumber);
  
  TTree *chrono_tree = NULL;
  TString Name="ChronoEventTree_";
            Name+=Chronobox;
            Name+="_";
            Name+=ChronoChannel;
  chrono_tree = (TTree *)f->Get(Name);
  if (chrono_tree == NULL)
  {
    Error("Get_Chrono_Tree", "\033[31mChrono Tree for run number %d not found\033[00m", runNumber);
    chrono_tree->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
  }

  return chrono_tree;
}

TH1D* Get_Chrono(Int_t runNumber, Int_t Chronobox, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  TTree* t=Get_Chrono_Tree(runNumber,Chronobox,ChronoChannel);
  TChrono_Event* e=new TChrono_Event();
  TString name="ChronoBox_";
  name+=Chronobox;
  name+="_";
  name+=ChronoChannel;
  TH1D* hh = new TH1D(	name.Data(),
                      name.Data(),
                      gNbin,tmin,tmax);

  t->SetBranchAddress("ChronoEvent", &e);
  for (Int_t i = 0; i < t->GetEntries(); ++i)
  {
     t->GetEntry(i);
     if (e->GetRunTime()<tmin) continue;
     if (e->GetRunTime()>tmax) continue;
     hh->Fill(e->GetRunTime());
   }
   return hh;
}
void Plot_Chrono(Int_t runNumber, Int_t Chronobox, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  TH1D* h=Get_Chrono( runNumber, Chronobox, ChronoChannel, tmin, tmax);
  h->Draw();
  return;  
} 
