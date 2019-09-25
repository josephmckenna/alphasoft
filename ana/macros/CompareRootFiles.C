Bool_t first = kFALSE;
Bool_t norm_int = kFALSE;
int col=2;
std::map<TString,TCanvas*> cmap;
TString savFolder;

void readdir(TDirectory *dir) 
{
  TDirectory *dirsav = gDirectory;
  cout<<" DIR: "<<dir->GetName()<<" : "<<dir->GetListOfKeys()->GetEntries()<<endl;
  TIter next(dir->GetListOfKeys());
  TKey *key;
  while( (key = (TKey*)next()) ) 
    {
      if( TString(key->GetName()).BeginsWith("chrono", TString::ECaseCompare::kIgnoreCase) ||
	  TString(key->GetName()).BeginsWith("sequencer", TString::ECaseCompare::kIgnoreCase) ) 
	continue; // kludge to fix unexplained infinite loop

      if( key->IsFolder() ) 
	{
	  dir->cd(key->GetName());
	  TDirectory *subdir = gDirectory;
	  readdir(subdir);
	  dirsav->cd();
	  continue;
	}

      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TH1")) continue;
      TH1 *h = (TH1*)key->ReadObj();
      h->SetStats(kFALSE);
      if( norm_int )
	h->Scale(1./h->Integral());
      h->SetLineColor(col);
      h->SetMarkerColor(col);
      TString cname("c");
      cname+=h->GetName();
      if(first) 
	{
	  //cout<<h->GetName()<<endl;
	  if( !cmap.count(cname) )
	    {
	      TCanvas* c = new TCanvas(cname,cname,1400,1200);
	      cmap.emplace(cname,c);
	    }
	  cmap[cname]->cd();
	  h->Draw();
	}
      else
	{
	  //TCanvas* c = (TCanvas*) gROOT->Get(cname);
	  cmap[cname]->cd();
	  h->Draw("same");
	}
    }
}


void CompareRootFiles() 
{
  gStyle->SetOptStat(0);
  TSeqCollection* filelist = gROOT->GetListOfFiles();
  savFolder=MakeAutoPlotsFolder("time");
  for(Int_t i=0; i<filelist->GetEntries(); ++i)
    {
      if(i) first = kFALSE;
      else first=kTRUE;
      TFile *f = (TFile*) filelist->At(i);
      if( f->IsZombie() )
	{
	  printf("File %s does not exist.\n",f->GetName());
	  return;
	}
      printf("Reading file ==> %s\n",f->GetName());
      printf("File size in bytes       = %lld\n",f->GetEND());
      printf("File compression factor  = %g\n",f->GetCompressionFactor());
    
      readdir(f);
      col++;
    }
  for(auto it = cmap.begin(); it != cmap.end(); ++it)
    {
      TString sname = TString::Format("%s%s.pdf",savFolder.Data(),it->second->GetName());
      it->second->Print(sname);
    }
}
