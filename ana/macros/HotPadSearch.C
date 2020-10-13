void HotPadSearch()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(fin->GetName());
  cout<<fname<<" FOUND"<<endl;

  int RunNumber = GetRunNumber( fname );
  cout<<"Run # "<<RunNumber<<endl;

  if( !gDirectory->cd("/") ) 
    {
      cout<<"something is wrong, exiting..."<<endl;
      gROOT->ProcessLine(".q");
    }

  // TObjString* sett = (TObjString*) gROOT->FindObject("ana_settings");
  // cout<<sett->GetString()<<endl;

  gDirectory->cd("paddeconv");
  TH2D* hocc = (TH2D*)gROOT->FindObject("hOccPad");
  hocc->SetStats(kFALSE);
  // //  hocc->SetMinimum(1000);
  // hocc->SetMinimum(800);
  // //hocc->SetMinimum(2000);
  // hocc->SetMaximum(2400);
  // //hocc->SetMaximum(5000);
  //  hocc->Scale(1./hocc->Integral());

  TString cname=TString::Format("coccpadR%d",RunNumber);
  TCanvas* c1 = new TCanvas(cname,cname,1700,1000);
  //hocc->Draw("surf2");
  //hocc->Draw();
  hocc->Draw("colz");
 

  double n=0.,m=0.,m2=0.;
  for(int r=0; r<576; ++r)
    {
      int bx=r+1;
      for(int s=0; s<32; ++s)
	{
	  int by=s+1;
	  int bin = hocc->GetBin(bx,by);
	  double bc = hocc->GetBinContent(bin);
	  m+=bc;
	  m2+=bc*bc;
	  n++;
	}
    } 
  double mu=m/n;
  double rms=sqrt(m2/n-mu*mu);
  cout<<"total: "<<n<<"\tmean: "<<mu<<"\trms: "<<rms<<endl;

  double thr=mu+3.*rms;
  cout<<"Hot Pads:\nsec\trow\tocc"<<endl;
  int Nhot=0;
  for(int r=0; r<576; ++r)
    {
      int bx=r+1;
      for(int s=0; s<32; ++s)
	{
	  int by=s+1;
	  int bin = hocc->GetBin(bx,by);
	  double bc = hocc->GetBinContent(bin);
	  if( bc > thr) 
	    {
	      cout<<s<<"\t"<<r<<"\t"<<bc<<endl;
	      ++Nhot;
	    }
	}
    } 
  cout<<"Total Hot: "<<Nhot<<endl;

  // thr=mu-5.*rms;
  // cout<<"Cold Pads:\nsec\trow\tocc"<<endl;
  // int Ncold=0;
  // for(int r=0; r<576; ++r)
  //   {
  //     int bx=r+1;
  //     for(int s=0; s<32; ++s)
  // 	{
  // 	  int by=s+1;
  // 	  bool skip=false;
  // 	  if( s == 17 || s == 18 ){
  // 	    for(int x=432; x<468; ++x) 
  // 	      {
  // 		if( r == x ) {
  // 		  skip=true;
  // 		  break;
  // 		}
  // 	      }
  // 	  }
  // 	  if( skip ) continue;
  // 	  int bin = hocc->GetBin(bx,by);
  // 	  double bc = hocc->GetBinContent(bin);
  // 	  if( bc < thr) 
  // 	    {
  // 	      cout<<s<<"\t"<<r<<"\t"<<bc<<endl;
  // 	      ++Ncold;
  // 	    }
  // 	}
  //   } 
  // cout<<"Total Cold: "<<Ncold<<endl;


  gDirectory->cd("/");
  TH2D* hsp = (TH2D*)gROOT->FindObject("hspzp");
  if( hsp )
    {
      hsp->SetStats(kFALSE);
      cname=TString::Format("cspZedPhiR%d",RunNumber);
      TCanvas* c2 = new TCanvas(cname,cname,1700,1000);
      hsp->Draw("colz");
      //hsp->SetMinimum(5);
      Int_t MaxBin = hsp->GetMaximumBin();
      Int_t x,y,z;
      hsp->GetBinXYZ(MaxBin, x, y, z);
      double maxval=hsp->GetBinContent(MaxBin);
      printf("The bin having the maximum value of %1.f is (%d,%d)\n",maxval,x,y);
      Int_t MinBin = hsp->GetMinimumBin();
      hsp->GetBinXYZ(MinBin, x, y, z);
      double minval=hsp->GetBinContent(MinBin); 
      printf("The bin having the maximum value of %1.f is (%d,%d)\n",minval,x,y);
    }


}
