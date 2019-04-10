#include "FileGetters.h"
TFile *Get_File(Int_t run_number, Bool_t die)
{
  TFile *f = NULL;
  TString file_name(getenv("AGRELEASE"));
  if (file_name.Length()<10)
  {
     std::cout <<"$AGRELEASE not set... please source agconfig.sh"<<std::endl;
     exit(0123);
  }
  //  file_name += "/ana/output";
  file_name += "/output";
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

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
