
void yoink() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   std::vector<TString> names{ "2hits2tracksBV", "2hitsNtracksBV", "Nhits2tracksBV", "NhitsNtracksBV", "2hits2tracksTPC", "2hitsNtracksTPC", "Nhits2tracksTPC", "NhitsNtracksTPC", "2hits2tracksBV_NoMatch", "2hitsNtracksBV_NoMatch", "Nhits2tracksBV_NoMatch", "NhitsNtracksBV_NoMatch"  };
   std::vector<TString> histos{ "dZ", "dphi", "TOF", "dZ_dphi", "dZ_TOF", "dphi_TOF" };

   // Makes histos
   TH2D* hist2d;
   TH1D* hist1d;

   // Makes canvas
   TCanvas *c = new TCanvas();
   gROOT->SetBatch(kTRUE);

   // Saves histograms
   for (int i=0;i<12;i++)
      {
         for (int j=0;j<3;j++)
            {
               fin->GetObject("bv_tpc_matching_module/"+names[i]+"/"+names[i]+"_"+histos[j],hist1d);
               if (j==0 or j==2) hist1d->Rebin(10);
               hist1d->Draw("colz");
               c->Print("hists/"+names[i]+"_"+histos[j]+".png");
               fin->GetObject("bv_tpc_matching_module/"+names[i]+"/"+names[i]+"_"+histos[j+3],hist2d);
               if (j==0 or j==1) hist2d->RebinX(10);
               if (j==1 or j==2) hist2d->RebinY(10);
               hist2d->Draw("colz");
               c->Print("hists/"+names[i]+"_"+histos[j+3]+".png");
            }
      }


}
