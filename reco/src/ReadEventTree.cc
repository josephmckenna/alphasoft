#include "ReadEventTree.hh"

void ReadEventTree::MakeHistos()
{
   std::cout<<"Preparing Histos"<<std::endl;

   // spacepoints
   fHisto->Book("hpxy","Spacepoints;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
   fHisto->GetHisto("hpxy")->SetStats(kFALSE);
   fHisto->Book("hpzr","Spacepoints;z [mm];r [mm]",600,-1200.,1200.,61,109.,174.);
   fHisto->GetHisto("hpzr")->SetStats(kFALSE);
   fHisto->Book("hpzp","Spacepoints;z [mm];#phi [deg]",600,-1200.,1200.,256,0.,360.);
   fHisto->GetHisto("hpzp")->SetStats(kFALSE);
   fHisto->Book("hprp","Spacepoints;#phi [deg];r [mm]",100,0.,TMath::TwoPi(),61,109.,174.);
   fHisto->GetHisto("hprp")->SetStats(kFALSE);

   fHisto->Book("hprad","Spacepoints;r [mm]",61,109.,174.);
   fHisto->GetHisto("hprad")->SetStats(kFALSE);
   fHisto->Book("hpphi","Spacepoints;#phi [deg]",256,0.,360.);
   fHisto->GetHisto("hpphi")->SetMinimum(0.);
   fHisto->GetHisto("hpphi")->SetStats(kFALSE);
   fHisto->Book("hpzed","Spacepoints;zed [mm]",600,-1200.,1200.);
   fHisto->GetHisto("hpzed")->SetStats(kFALSE);


   // tracks (lines) and spacepoints
   fHisto->Book("hspzphi","Spacepoint Axial-Azimuth for Good Tracks;z [mm];#phi [deg]",
                600,-1200.,1200.,256,0.,360.);
   fHisto->GetHisto("hspzphi")->SetStats(kFALSE);
   fHisto->Book("hspxy","Spacepoint X-Y for Good Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
   fHisto->GetHisto("hspxy")->SetStats(kFALSE);


   fHisto->Book("hspzr","Spacepoints in Tracks;z [mm];r [mm]",
                600,-1200.,1200.,61,109.,174.);
   fHisto->GetHisto("hspzr")->SetStats(kFALSE);
  
   fHisto->Book("hsprp","Spacepoints in Tracks;#phi [deg];r [mm]",
                100,0.,TMath::TwoPi(),61,109.,174.);
   fHisto->GetHisto("hsprp")->SetStats(kFALSE);

   int blen = 200;
   double maxlen = 400.;
   fHisto->Book("hsplen","Distance between First and Last Spacepoint;[mm]",blen,0.,maxlen);
   fHisto->Book("hsprlen","Distance between First and Last Spacepoint;r [mm]; d [mm]",
                100,108.,175.,blen,0.,maxlen);
   fHisto->Book("hspNlen","Distance between First and Last Spacepoint;Number of Points; d [mm]",
                200,0.,200.,blen,0.,maxlen);


   fHisto->Book("hqr","Intercept Point;r [mm]",200,100.,190.);
   fHisto->Book("hqphi","Intercept Point;#phi [deg]",180,-180.,180.);
   fHisto->GetHisto("hqphi")->SetMinimum(0.);
   fHisto->Book("hqz","Intercept Point;z [mm]",600,-1200.,1200.);
   fHisto->Book("hqxy","Intercept Point;x [mm];y [mm]",
                100,-190.,190.,100,-190.,190.);
   fHisto->GetHisto("hqxy")->SetStats(kFALSE);
   fHisto->Book("hqzr","Intercept Point;z [mm];r [mm]",
                600,-1200.,1200.,200,100.,190.);
   fHisto->GetHisto("hqzr")->SetStats(kFALSE);
   fHisto->Book("hqzphi","Intercept Point;z [mm];#phi [deg]",
                600,-1200.,1200.,180,-180.,180.);
   fHisto->GetHisto("hqzphi")->SetStats(kFALSE);
   fHisto->Book("hqrphi","Intercept Point;r [mm];#phi [deg];",
                200,100.,190.,180,-180.,180.);
   fHisto->GetHisto("hqzphi")->SetStats(kFALSE);


   // reco helices
   fHisto->Book("hNhel","Reconstructed Helices",10,0.,10.);
   //  hhdist = new TH1D("hhdist","Distance between  2 helices;s [mm]",200,0.,20.);
 

   fHisto->Book("hpt","Helix Transverse Momentum;p_{T} [MeV/c]",1000,0.,2000.);
   fHisto->Book("hpz","Helix Longitudinal Momentum;p_{Z} [MeV/c]",2000,-1000.,1000.);
   fHisto->Book("hpp","Helix Total Momentum;p_{tot} [MeV/c]",1000,0.,2000.);
   fHisto->Book("hptz","Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
                100,0.,2000.,200,-1000.,1000.);

   // reco helices spacepoints
   fHisto->Book("hhpattreceff","Track Finding Efficiency",201,-1.,200.);
   fHisto->GetHisto("hhpattreceff")->SetLineWidth(2);;
   fHisto->Book("hhspxy","Spacepoints in Helices;x [mm];y [mm]",
                100,-190.,190.,100,-190.,190.);
   fHisto->GetHisto("hhspxy")->SetStats(kFALSE);
   fHisto->Book("hhspzr","Spacepoints in Helices;z [mm];r [mm]",
                600,-1200.,1200.,61,109.,174.);
   fHisto->GetHisto("hhspzr")->SetStats(kFALSE);
   fHisto->Book("hhspzp","Spacepoints in Helices;z [mm];#phi [deg]",
                600,-1200.,1200.,100,0.,360.);
   fHisto->GetHisto("hhspzp")->SetStats(kFALSE);

   fHisto->Book("hhsprp","Spacepoints in Helices;#phi [deg];r [mm]",
                100,0.,TMath::TwoPi(),61,109.,174.);
   fHisto->GetHisto("hhsprp")->SetStats(kFALSE);


   // used helices
   fHisto->Book("hNusedhel","Used Helices",10,0.,10.);

   fHisto->Book("huhD","Used Hel D;[mm]",200,0.,200.);
   fHisto->Book("huhc","Used Hel c;[mm^{-1}]",200,-1.e-1,1.e-1);
   fHisto->Book("huhchi2R","Used Hel #chi^{2}_{R}",100,0.,50.);
   fHisto->Book("huhchi2Z","Used Hel #chi^{2}_{Z}",100,0.,50.);

   fHisto->Book("huhpt","Used Helix Transverse Momentum;p_{T} [MeV/c]",1000,0.,2000.);
   fHisto->Book("huhpz","Used Helix Longitudinal Momentum;p_{Z} [MeV/c]",2000,-1000.,1000.);
   fHisto->Book("huhpp","Used Helix Total Momentum;p_{tot} [MeV/c]",1000,0.,2000.);
   fHisto->Book("huhptz","Used Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
                100,0.,2000.,200,-1000.,1000.);

   // used helices spacepoints
   fHisto->Book("huhspxy","Spacepoints in Used Helices;x [mm];y [mm]",
                100,-190.,190.,100,-190.,190.);
   fHisto->GetHisto("huhspxy")->SetStats(kFALSE);
   fHisto->Book("huhspzr","Spacepoints in Used Helices;z [mm];r [mm]",
                600,-1200.,1200.,61,109.,174.);
   fHisto->GetHisto("huhspzr")->SetStats(kFALSE);
   fHisto->Book("huhspzp","Spacepoints in Used Helices;z [mm];#phi [deg]",
                600,-1200.,1200.,100,0.,360.);
   fHisto->GetHisto("huhspzp")->SetStats(kFALSE);

   fHisto->Book("huhsprp","Spacepoints in Used Helices;#phi [deg];r [mm]",
                100,0.,TMath::TwoPi(),90,109.,174.);
   fHisto->GetHisto("huhsprp")->SetStats(kFALSE);


   // cosmic time distribution
   fHisto->Book("hpois","Delta t between cosmics;#Delta t [ms]",200,0.,111.);
   fHisto->GetHisto("hpois")->SetMarkerColor(kBlack);
   fHisto->GetHisto("hpois")->SetMarkerStyle(8);
   fHisto->GetHisto("hpois")->SetLineColor(kBlack);

   // z axis intersection
   fHisto->Book("hldz","Minimum Radius;r [mm]",200,0.,190.);
   fHisto->GetHisto("hldz")->SetLineColor(kRed);
   fHisto->Book("hlr","Minimum Radius;r [mm]",200,0.,190.);
   fHisto->Book("hlz","Z intersection with min rad;z [mm]",300,-1200.,1200.);
   fHisto->Book("hlp","#phi intersection with min rad;#phi [deg]",100,-180.,180.);
   fHisto->Book("hlzp","Z-#phi intersection with min rad;z [mm];#phi [deg]",
                100,-1200.,1200.,90,-180.,180.);
   fHisto->GetHisto("hlzp")->SetStats(kFALSE);
   fHisto->Book("hlzr","Z-R intersection with min rad;z [mm];r [mm]",
                100,-1200.,1200.,100,0.,190.);
   fHisto->Book("hlrp","R-#phi intersection with min rad;r [mm];#phi [deg]",
                100,0.,190.,90,-180.,180.);
   fHisto->Book("hlxy","X-Y intersection with min rad;x [mm];y [mm]",
                100,-190.,190.,100,-190.,190.);


   // helices 
   fHisto->Book("hhr","Helix Minimum Radius;r [mm]",200,0.,190.);
   fHisto->Book("hhz","Helix Z intersection with min rad;z [mm]",300,-1200.,1200.);
   fHisto->Book("hhp","Helix #phi intersection with min rad;#phi [deg]",100,-180.,180.);
   fHisto->Book("hhzp","Helix Z-#phi intersection with min rad;z [mm];#phi [deg]",
                100,-1200.,1200.,90,-180.,180.);
   fHisto->GetHisto("hhzp")->SetStats(kFALSE);
   fHisto->Book("hhzr","Helix Z-R intersection with min rad;z [mm];r [mm]",
                100,-1200.,1200.,100,0.,190.);
   fHisto->Book("hhrp","Helix R-#phi intersection with min rad;r [mm];#phi [deg]",
                100,0.,190.,90,-180.,180.);
   fHisto->Book("hhxy","Helix X-Y intersection with min rad;x [mm];y [mm]",
                100,-190.,190.,100,-190.,190.);



 
   // line properties
   fHisto->Book("hNlines","Reconstructed Lines",10,0.,10.);
   fHisto->Book("hlchi2","#chi^{2} of Straight Lines",200,0.,600.); // chi^2 of line fit

   fHisto->Book("hpattreceff","Track Finding Efficiency",201,-1.,200.);
   fHisto->GetHisto("hpattreceff")->SetLineWidth(2);

   fHisto->Book("hsplenchi2","Track Length vs #chi^{2}",blen,0.,maxlen,200,0.,200.);
      
   fHisto->Book("hlphi","Direction #phi;#phi [deg]",200,-180.,180.);
   fHisto->Book("hltheta","Direction #theta;#theta [deg]",200,0.,180.);
   fHisto->Book("hlthetaphi","Direction #theta Vs. #phi;#theta [deg];#phi [deg]",100,0.,180.,100,-180.,180.);

   fHisto->Book("hlcosang","Cosine of Angle Formed by 2 Lines;cos(#alpha)",2000,-1.,1.);
   fHisto->Book("hldist","Distance between  2 Lines;s [mm]",200,0.,20.);

   fHisto->Book("hlcosangdist",
                "Correlation Angle-Distance;cos(#alpha);s [mm]",
                200,-1.,1.,200,0.,20.);

   fHisto->Book("hhchi2R","Hel #chi^{2}_{R}",200,0.,200.); // R chi^2 of helix
   fHisto->Book("hhchi2Z","Hel #chi^{2}_{Z}",200,0.,1000.); // Z chi^2 of helix
   fHisto->Book("hhD","Hel D;[mm]",500,-190.,190.);
   fHisto->GetHisto("hhD")->SetMinimum(0);
   fHisto->Book("hhc","Hel c;[mm^{-1}]",200,-1.e-1,1.e-1);

   // reco vertex
   fHisto->Book("hvr","Vertex Radius;r [mm]",190,0.,190.);
   fHisto->Book("hvphi","Vertex #phi; [deg]",360,-180.,180.);
   fHisto->GetHisto("hvphi")->SetMinimum(0.);
   fHisto->Book("hvz","Vertex Z;z [mm]",1000,-1152.,1152.);
   fHisto->Book("hvxy","Vertex X-Y;x [mm];y [mm]",200,-190.,190.,200,-190.,190.);
 
}

void ReadEventTree::DisplayHisto()
{
   TString cname;

   if( hht ) {
      // deconv signals histos
      cname = "deconv";
      cname+=tag;
      TCanvas* cdec = new TCanvas(cname.Data(),cname.Data(),1900,1000);
      cdec->Divide(3,2);

      cdec->cd(1);
      //  hht->Scale(1./hht->Integral());
      hht->Draw();
      //  hhpad->Scale(1./hhpad->Integral());
      if( hhpad->GetEntries() > 0. )
         {
            hhpad->Draw("same");
            //hht->GetXaxis()->SetRangeUser(0.,2100.);
            hht->GetXaxis()->SetRangeUser(0.,1500.);
            hht->GetYaxis()->SetRangeUser(0.,
                                          hht->GetBinContent(hht->GetMaximumBin()) > hhpad->GetBinContent(hhpad->GetMaximumBin()) ?
                                          hht->GetBinContent(hht->GetMaximumBin())*1.1 : hhpad->GetBinContent(hhpad->GetMaximumBin())*1.1
                                          );
         }
      else
         hht->GetYaxis()->SetRangeUser(0.,hht->GetBinContent(hht->GetMaximumBin())*1.1);

      TLegend* legdec = new TLegend(0.65,0.62,0.84,0.73);
      legdec->AddEntry(hht,"AW","l");
      legdec->AddEntry(hhpad,"pad","l");
      legdec->Draw("same");

      if( hmatch->GetEntries() > 0 )
         {
            cdec->cd(4);
            hmatch->Draw();
            //	hmatch->GetXaxis()->SetRangeUser(0.,2100.);
            hmatch->GetXaxis()->SetRangeUser(0.,1500.);
         }

      cdec->cd(2);
      hot->Scale(1./hot->Integral());
      //  hot->Draw("HIST");
      if( hocol->GetEntries() > 0 )
         {
            hot->Draw("HIST");
            hocol->Scale(1./hocol->Integral());
            hocol->Draw("HISTsame");
            hot->GetYaxis()->SetRangeUser(0.,
                                          hot->GetBinContent(hot->GetMaximumBin()) > hocol->GetBinContent(hocol->GetMaximumBin()) ?
                                          hot->GetBinContent(hot->GetMaximumBin())*1.1 : hocol->GetBinContent(hocol->GetMaximumBin())*1.1
                                          );

         }
      else
         {
            std::cout<<hot->GetName()<<"\t"<<hot->GetBinContent(hot->GetMaximumBin())<<std::endl;
            hot->Draw();
            hot->GetYaxis()->SetRangeUser( 0.,hot->GetBinContent(hot->GetMaximumBin())*1.1 );
         }

      cdec->cd(3);
      htt->GetXaxis()->SetRangeUser(0.,4200.);
      htt->Draw();
      if( htpad->GetEntries() > 0 )
         {
            htpad->Draw("same");

            cdec->cd(5);
            hawpadsector->SetStats(kFALSE);
            //  hawpadsector->RebinX();
            // hawpadsector->GetXaxis()->SetRangeUser(300.,4200.);
            // hawpadsector->GetYaxis()->SetRangeUser(300.,4200.);
            hawpadsector->GetXaxis()->SetRangeUser(10.,3200.);
            hawpadsector->GetYaxis()->SetRangeUser(10.,3200.);
            hawpadsector->Draw("colz");

            cdec->cd(6);
            hopad->SetStats(kFALSE);
            hopad->Draw("colz");
         }

      if(_save_plots) {
         cdec->SaveAs(savFolder+cname+TString(".pdf"));
         cdec->SaveAs(savFolder+cname+TString(".pdf"));
      }
   }
   // spacepoints
   if( fHisto->GetHisto("hpxy")->GetEntries() > 0 )
      {
         cname = "spacepoints";
         cname+=tag;
         TCanvas* cpnt = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         cpnt->Divide(2,2);
         cpnt->cd(1);
         //      fHisto->GetHisto("hprp")->Draw("colz");
         fHisto->GetHisto("hprp")->Draw("pol surf2");
         cpnt->cd(2);
         fHisto->GetHisto("hpxy")->Draw("colz");
         cpnt->cd(3);
         fHisto->GetHisto("hpzr")->Draw("colz");
         cpnt->cd(4);
         fHisto->GetHisto("hpzp")->Draw("colz");

         if( _save_plots ) {
            cpnt->SaveAs(savFolder+cname+TString(".pdf"));
            cpnt->SaveAs(savFolder+cname+TString(".pdf"));
         }

         cname = "spacepoints_coord";
         cname+=tag;
         TCanvas* cpntcoord = new TCanvas(cname.Data(),cname.Data(),1800,1400);
         cpntcoord->Divide(1,3);
         cpntcoord->cd(1);
         fHisto->GetHisto("hprad")->Draw();
         cpntcoord->cd(2);
         fHisto->GetHisto("hpphi")->Draw();
         cpntcoord->cd(3);
         fHisto->GetHisto("hpzed")->Draw();

         if( _save_plots )
            {
               cpntcoord->SaveAs(savFolder+cname+TString(".pdf"));
               cpntcoord->SaveAs(savFolder+cname+TString(".pdf"));
            }
      }

   // spacepoints in tracks
   if( fHisto->GetHisto("hspxy")->GetEntries() > 0 )
      {
         cname = "spacepoint_lines";
         cname+=tag;
         TCanvas* csp = new TCanvas(cname.Data(),cname.Data(),1900,1300);
         csp->Divide(3,2);
         //csprphi->cd(1);
         //fHisto->GetHisto("hsprp")->Draw("pol surf2");
         csp->cd(1);
         fHisto->GetHisto("hspxy")->Draw("colz");
         csp->cd(2);
         fHisto->GetHisto("hspzr")->Draw("colz");
         csp->cd(3);
         fHisto->GetHisto("hspzphi")->Draw("colz");
         csp->cd(4);
         fHisto->GetHisto("hsplen")->Draw();
         csp->cd(5);
         fHisto->GetHisto("hsprlen")->Draw("colz");
         csp->cd(6);
         fHisto->GetHisto("hspNlen")->Draw("colz");
         if( _save_plots ) {
            csp->SaveAs(savFolder+cname+TString(".pdf"));
            csp->SaveAs(savFolder+cname+TString(".pdf")); }
      }

   //  if( fHisto->GetHisto("hNlines")->GetEntries() )
   if( fHisto->GetHisto("hlphi")->GetEntries() )
      {
         cname = "lines_properties";
         cname+=tag;
         TCanvas* clp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         clp->Divide(2,2);
         clp->cd(1);
         fHisto->GetHisto("hpattreceff")->Draw();
         clp->cd(2);
         fHisto->GetHisto("hlchi2")->Draw("HIST");
         clp->cd(3);
         fHisto->GetHisto("hNlines")->Draw();
         fHisto->GetHisto("hNlines")->GetXaxis()->SetNdivisions(110);
         fHisto->GetHisto("hNlines")->GetXaxis()->CenterLabels();
         clp->cd(4);
         fHisto->GetHisto("hsplenchi2")->Draw("colz");
         if( _save_plots )
            {
               clp->SaveAs(savFolder+cname+TString(".pdf"));
               clp->SaveAs(savFolder+cname+TString(".pdf"));
            }


         cname = "lines_angle";
         cname+=tag;
         TCanvas* cla = new TCanvas(cname.Data(),cname.Data(),1800,1400);
         std::cout<<cname<<std::endl;
         cla->Divide(3,2);
         
         cla->cd(1);
         fHisto->GetHisto("hlphi")->Draw();
         fHisto->GetHisto("hlphi")->GetYaxis()->SetRangeUser(0.,fHisto->GetHisto("hlphi")->GetBinContent(fHisto->GetHisto("hlphi")->GetMaximumBin())*1.1);
         cla->cd(2);
         fHisto->GetHisto("hltheta")->Draw();
         cla->cd(3);
         fHisto->GetHisto("hlthetaphi")->Draw("colz");
         cla->cd(4);
         fHisto->GetHisto("hlcosang")->Draw();
         fHisto->GetHisto("hlcosang")->GetXaxis()->SetRangeUser(-1.,-0.9);
         cla->cd(5);
         fHisto->GetHisto("hldist")->Draw();
         cla->cd(6);
         fHisto->GetHisto("hlcosangdist")->Draw("colz");
         if( _save_plots ) {
            cla->SaveAs(savFolder+cname+TString(".pdf"));
            cla->SaveAs(savFolder+cname+TString(".pdf")); }

         cname = "lines_intercept";
         cname+=tag;
         TCanvas* cq = new TCanvas(cname.Data(),cname.Data(),2600,1400);
         std::cout<<cname<<std::endl;
         cq->Divide(3,2);
         cq->cd(1);
         fHisto->GetHisto("hqr")->Draw("HIST");
         cq->cd(2);
         fHisto->GetHisto("hqphi")->Draw("HIST");
         cq->cd(3);
         fHisto->GetHisto("hqz")->Draw("HIST");
         cq->cd(4);
         fHisto->GetHisto("hqxy")->Draw("colz");
         cq->cd(5);
         fHisto->GetHisto("hqzr")->Draw("colz");
         cq->cd(6);
         fHisto->GetHisto("hqzphi")->Draw("colz");
         // cq->cd(8);
         // fHisto->GetHisto("hqrphi")->Draw("colz");
         if(_save_plots){
            cq->SaveAs(savFolder+cname+TString(".pdf"));  
            cq->SaveAs(savFolder+cname+TString(".pdf"));}
      }

   // z axis intersection
   if( fHisto->GetHisto("hlz")->GetEntries() > 0 )
      {
         cname = "z_axis_intersection";
         cname+=tag;
         TCanvas* czint = new TCanvas(cname.Data(),cname.Data(),1600,1000);
         czint->Divide(3,2);
         czint->cd(1);
         fHisto->GetHisto("hlr")->Draw();
         //      fHisto->GetHisto("hldz")->Draw("same");
         czint->cd(2);
         fHisto->GetHisto("hlz")->Draw();
         czint->cd(3);
         fHisto->GetHisto("hlp")->Draw();
         fHisto->GetHisto("hlp")->GetYaxis()->SetRangeUser(0.,fHisto->GetHisto("hlp")->GetBinContent(fHisto->GetHisto("hlp")->GetMaximumBin())*1.1);
         czint->cd(4);
         fHisto->GetHisto("hlzp")->Draw("colz");
         czint->cd(5);
         //fHisto->GetHisto("hlzr")->Draw("colz");
         fHisto->GetHisto("hlxy")->Draw("colz");
         gPad->SetLogz();
         czint->cd(6);
         fHisto->GetHisto("hlrp")->Draw("colz");
         if(_save_plots){
            czint->SaveAs(savFolder+cname+TString(".pdf"));
            czint->SaveAs(savFolder+cname+TString(".pdf"));}
      }

   // cosmic time distribution
   if( fHisto->GetHisto("hpois")->GetEntries() > 0 )
      {
         // cosmic time distribution
         cname = "time_distribution_between_cosmics";
         cname+=tag;
         TCanvas* cpois = new TCanvas(cname.Data(),cname.Data(),1300,1000);
         fHisto->GetHisto("hpois")->Draw("P");
         fHisto->GetHisto("hpois")->Fit("expo","Q0EMW");
         TF1* fcosrate = fHisto->GetHisto("hpois")->GetFunction("expo");
         if( fcosrate )
            {
               double rate = fabs( fcosrate->GetParameter(1) )*1.e3,
                  rate_err = fabs( fcosrate->GetParError(1) )*1.e3;
               TString srate = TString::Format("Cosmic Rate R%d: (%1.1f#pm%1.1f) Hz",
                                               RunNumber,rate,rate_err);
               std::cout<<srate<<std::endl;
               fcosrate->Draw("same");
               TPaveText* trate = new TPaveText(0.5,0.53,0.87,0.6,"NDC");
               trate->AddText(srate.Data());
               trate->SetFillColor(0);
               trate->Draw();
            }
         if(_save_plots){
            cpois->SaveAs(savFolder+cname+TString(".pdf"));
            cpois->SaveAs(savFolder+cname+TString(".pdf"));}
      }

   // reco helices
   //  if(fHisto->GetHisto("hNhel->GetEntries()"))
   if(fHisto->GetHisto("hhD")->GetEntries())
      {
         cname = "chel";
         cname+=tag;
         TCanvas* chel = new TCanvas(cname.Data(),cname.Data(),1000,800);
         chel->Divide(2,1);
         chel->cd(1);
         fHisto->GetHisto("hNhel")->Draw();
         chel->cd(2);
         //      fHisto->GetHisto("hhdist")->Draw();
         fHisto->GetHisto("hhpattreceff")->Draw();
         if(_save_plots){
            chel->SaveAs(savFolder+cname+TString(".pdf"));
            chel->SaveAs(savFolder+cname+TString(".pdf"));}

         cname ="chelprop";
         cname+=tag;
         TCanvas* chelprop = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         chelprop->Divide(2,2);
         chelprop->cd(1);
         fHisto->GetHisto("hhD")->Draw();
         chelprop->cd(2);
         fHisto->GetHisto("hhc")->Draw();
         chelprop->cd(3);
         fHisto->GetHisto("hhchi2R")->Draw();
         chelprop->cd(4);
         fHisto->GetHisto("hhchi2Z")->Draw();
         if(_save_plots){
            chelprop->SaveAs(savFolder+cname+TString(".pdf"));
            chelprop->SaveAs(savFolder+cname+TString(".pdf"));}

         cname ="chelmom";
         cname+=tag;
         TCanvas* chelmom = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         chelmom->Divide(2,2);
         chelmom->cd(1);
         fHisto->GetHisto("hpt")->Draw();
         chelmom->cd(2);
         fHisto->GetHisto("hpz")->Draw();
         chelmom->cd(3);
         fHisto->GetHisto("hpp")->Draw();
         chelmom->cd(4);
         fHisto->GetHisto("hptz")->Draw("colz");
         if(_save_plots){
            chelmom->SaveAs(savFolder+cname+TString(".pdf"));
            chelmom->SaveAs(savFolder+cname+TString(".pdf"));}

         cname = "spacepoints_helices";
         cname+=tag;
         TCanvas* chsp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         chsp->Divide(2,2);
         chsp->cd(1);
         fHisto->GetHisto("hhspxy")->Draw("colz");
         chsp->cd(2);
         fHisto->GetHisto("hhspzr")->Draw("colz");
         chsp->cd(3);
         fHisto->GetHisto("hhspzp")->Draw("colz");
         chsp->cd(4);
         fHisto->GetHisto("hhsprp")->Draw("colz");
         if(_save_plots){
            chsp->SaveAs(savFolder+cname+TString(".pdf"));
            chsp->SaveAs(savFolder+cname+TString(".pdf"));}
      }

   // vertex
   if( fHisto->GetHisto("hvr")->GetEntries() )
      {
         cname="cvtx";
         cname+=tag;
         TCanvas* cvtx = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         cvtx->Divide(2,2);
         cvtx->cd(1);
         fHisto->GetHisto("hvr")->Draw();
         cvtx->cd(2);
         fHisto->GetHisto("hvphi")->Draw();
         cvtx->cd(3);
         fHisto->GetHisto("hvz")->Draw();
         cvtx->cd(4);
         fHisto->GetHisto("hvxy")->Draw("colz");
         if(_save_plots){
            cvtx->SaveAs(savFolder+cname+TString(".pdf"));
            cvtx->SaveAs(savFolder+cname+TString(".pdf"));}
      }

   // used helices
   //  if(fHisto->GetHisto("hNusedhel->GetEntries())
   if(fHisto->GetHisto("huhD")->GetEntries())
      {
         cname = "cusehel";
         cname+=tag;
         TCanvas* cusehel = new TCanvas(cname.Data(),cname.Data(),1000,800);
         fHisto->GetHisto("hNusedhel")->Draw();
         if(_save_plots){
            cusehel->SaveAs(savFolder+cname+TString(".pdf"));
            cusehel->SaveAs(savFolder+cname+TString(".pdf"));}

         cname ="cusehelprop";
         cname+=tag;
         TCanvas* cusehelprop = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         cusehelprop->Divide(2,2);
         cusehelprop->cd(1);
         fHisto->GetHisto("huhD")->Draw();
         cusehelprop->cd(2);
         fHisto->GetHisto("huhc")->Draw();
         cusehelprop->cd(3);
         fHisto->GetHisto("huhchi2R")->Draw();
         cusehelprop->cd(4);
         fHisto->GetHisto("huhchi2Z")->Draw();
         if(_save_plots){
            cusehelprop->SaveAs(savFolder+cname+TString(".pdf"));
            cusehelprop->SaveAs(savFolder+cname+TString(".pdf"));}

         cname ="cusehelmom";
         cname+=tag;
         TCanvas* cusehelmom = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         cusehelmom->Divide(2,2);
         cusehelmom->cd(1);
         fHisto->GetHisto("huhpt")->Draw();
         cusehelmom->cd(2);
         fHisto->GetHisto("huhpz")->Draw();
         cusehelmom->cd(3);
         fHisto->GetHisto("huhpp")->Draw();
         cusehelmom->cd(4);
         fHisto->GetHisto("huhptz")->Draw("colz");
         if(_save_plots){
            cusehelmom->SaveAs(savFolder+cname+TString(".pdf"));
            cusehelmom->SaveAs(savFolder+cname+TString(".pdf"));}

         cname = "spacepoints_usedhelices";
         cname+=tag;
         TCanvas* chsp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
         chsp->Divide(2,2);
         chsp->cd(1);
         fHisto->GetHisto("huhspxy")->Draw("colz");
         chsp->cd(2);
         fHisto->GetHisto("huhspzr")->Draw("colz");
         chsp->cd(3);
         fHisto->GetHisto("huhspzp")->Draw("colz");
         chsp->cd(4);
         fHisto->GetHisto("huhsprp")->Draw("colz");
         if(_save_plots){
            chsp->SaveAs(savFolder+cname+TString(".pdf"));
            chsp->SaveAs(savFolder+cname+TString(".pdf"));}
      }
}

void ReadEventTree::ProcessLine(TStoreLine* aLine)
{
   TVector3 u = *(aLine->GetDirection());
   TVector3 p = *(aLine->GetPoint());

   fHisto->FillHisto("hlphi",u.Phi()*TMath::RadToDeg());
   fHisto->FillHisto("hltheta",u.Theta()*TMath::RadToDeg());
   fHisto->FillHisto("hlthetaphi",u.Theta()*TMath::RadToDeg(),u.Phi()*TMath::RadToDeg());

   // z axis intersection
   double num = zaxis.Cross(p) * u.Cross(zaxis);
   double den = u.Cross(zaxis).Mag2();

   if( den > 0. )
      {
         double t = num/den;
         TVector3 zint = p + t*u;
         fHisto->FillHisto("hlr", zint.Perp() );
         fHisto->FillHisto("hlz", zint.Z() );
         fHisto->FillHisto("hlp", zint.Phi()*TMath::RadToDeg() );
         fHisto->FillHisto("hlzp", zint.Z(), zint.Phi()*TMath::RadToDeg() );
         fHisto->FillHisto("hlzr", zint.Z(), zint.Perp() );
         fHisto->FillHisto("hlrp", zint.Perp(), zint.Phi()*TMath::RadToDeg() );
         fHisto->FillHisto("hlxy", zint.X(), zint.Y() );
      }

   fHisto->FillHisto("hlchi2",aLine->GetChi2());

   fHisto->FillHisto("hqr",p.Perp());
   fHisto->FillHisto("hqz",p.Z());
   fHisto->FillHisto("hqphi",p.Phi()*TMath::RadToDeg());
   fHisto->FillHisto("hqxy",p.X(),p.Y());
   fHisto->FillHisto("hqzr",p.Z(),p.Perp());
   fHisto->FillHisto("hqzphi",p.Z(),p.Phi()*TMath::RadToDeg());
   fHisto->FillHisto("hqrphi",p.Perp(),p.Phi()*TMath::RadToDeg());


   const TObjArray* sp = aLine->GetSpacePoints();
   for( int ip = 0; ip<sp->GetEntries(); ++ip )
      {
         TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
         fHisto->FillHisto("hspxy", ap->GetX(), ap->GetY() );
         fHisto->FillHisto("hspzphi", ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
         fHisto->FillHisto("hspzr", ap->GetZ(), ap->GetR() );
         fHisto->FillHisto("hsprp", ap->GetPhi(), ap->GetR() );
      }
   double maxd= ((TSpacePoint*)sp->Last())->Distance( (TSpacePoint*)sp->First() );
   fHisto->FillHisto("hsplen", maxd );
   fHisto->FillHisto("hsprlen", ((TSpacePoint*)sp->Last())->GetR(), maxd );
   fHisto->FillHisto("hspNlen", double(sp->GetEntries()), maxd );

   fHisto->FillHisto("hsplenchi2", maxd, aLine->GetChi2());
}

double ReadEventTree::LineDistance(TStoreLine* l0, TStoreLine* l1)
{
   TVector3 u0 = *(l0->GetDirection());
   TVector3 u1 = *(l1->GetDirection());
   TVector3 p0 = *(l0->GetPoint());
   TVector3 p1 = *(l1->GetPoint());

   TVector3 n0 = u0.Cross( u1 ); // normal to lines
   TVector3 c =  p1 - p0;
   if( n0.Mag() == 0. ) return -1.;

   TVector3 n1 = n0.Cross( u1 ); // normal to plane formed by n0 and line1

   double tau = c.Dot( n1 ) / u0.Dot( n1 ); // intersection between
   TVector3 q0 = tau * u0 + p0;             // plane and line0

   double t1 = ( (q0-p0).Cross(n0) ).Dot( u0.Cross(n0) ) / ( u0.Cross(n0) ).Mag2();
   TVector3 q1 = t1 * u0 + p0;

   double t2 = ( (q0-p1).Cross(n0) ).Dot( u1.Cross(n0) ) / ( u1.Cross(n0) ).Mag2();
   TVector3 q2 = t2*u1+p1;

   TVector3 Q = q2 - q1;

   return Q.Mag();
}

void ReadEventTree::ProcessHelix(TStoreHelix* hel)
{
   fHisto->FillHisto("hhD",hel->GetD());
   fHisto->FillHisto("hhc",hel->GetC());
   fHisto->FillHisto("hhchi2R",hel->GetRchi2());
   fHisto->FillHisto("hhchi2Z",hel->GetZchi2());

   //  hel->GetMomentumV().Print();

   fHisto->FillHisto("hpt",hel->GetMomentumV().Perp());
   fHisto->FillHisto("hpz",hel->GetMomentumV().Z());
   fHisto->FillHisto("hpp",hel->GetMomentumV().Mag());
   fHisto->FillHisto("hptz",hel->GetMomentumV().Perp(),hel->GetMomentumV().Z());

   const TObjArray* sp = hel->GetSpacePoints();
   for( int ip = 0; ip<sp->GetEntries(); ++ip )
      {
         TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
         fHisto->FillHisto("hhspxy", ap->GetX(), ap->GetY() );
         fHisto->FillHisto("hhspzp", ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
         fHisto->FillHisto("hhspzr", ap->GetZ(), ap->GetR() );
         fHisto->FillHisto("hhsprp", ap->GetPhi(), ap->GetR() );
      }
}

void ReadEventTree::ProcessUsed(TFitHelix* hel)
{
   fHisto->FillHisto("huhD",hel->GetD());
   fHisto->FillHisto("huhc",hel->GetC());
   fHisto->FillHisto("huhchi2R",hel->GetRchi2());
   fHisto->FillHisto("huhchi2Z",hel->GetZchi2());

   //  hel->GetMomentumV().Print();

   fHisto->FillHisto("huhpt",hel->GetMomentumV().Perp());
   fHisto->FillHisto("huhpz",hel->GetMomentumV().Z());
   fHisto->FillHisto("huhpp",hel->GetMomentumV().Mag());
   fHisto->FillHisto("huhptz",hel->GetMomentumV().Perp(),hel->GetMomentumV().Z());

   const std::vector<TSpacePoint*> *sp = hel->GetPointsArray();
   for( unsigned int ip = 0; ip<sp->size(); ++ip )
      {
         TSpacePoint* ap = sp->at(ip);
         fHisto->FillHisto("huhspxy", ap->GetX(), ap->GetY() );
         fHisto->FillHisto("huhspzp", ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
         fHisto->FillHisto("huhspzr", ap->GetZ(), ap->GetR() );
         fHisto->FillHisto("huhsprp", ap->GetPhi(), ap->GetR() );
      }
}

void ReadEventTree::ProcessVertex(TVector3* v)
{
   fHisto->FillHisto("hvr",v->Perp());
   fHisto->FillHisto("hvphi",v->Phi()*TMath::RadToDeg());
   fHisto->FillHisto("hvz",v->Z());
   fHisto->FillHisto("hvxy",v->X(),v->Y());
}

void ReadEventTree::ProcessTree( )
{
   std::cout<<"ProcessTree"<<std::endl;
   TStoreEvent* event = new TStoreEvent();
   tin->SetBranchAddress("StoredEvent", &event);
   double temp=0.;
   double Nvtx=0.;
   for(int e=0; e<tin->GetEntries(); ++e)
      {
         if( e%1000 == 0 ) printf("*** %d\r",e);//std::cout<<"*** "<<e<<std::endl;
         event->Reset();
         tin->GetEntry(e);
         //      std::cout<<event->GetEventNumber()<<"\t"<<event->GetTimeOfEvent()<<std::endl;

         const TObjArray* points = event->GetSpacePoints();
         //      std::cout<<"Number of Points: "<<points->GetEntries()<<std::endl;
         for(int p=0; p<points->GetEntriesFast(); ++p)
            {
               TSpacePoint* ap = (TSpacePoint*) points->At(p);
               if( !ap->IsGood(ALPHAg::_cathradius, ALPHAg::_fwradius) ) continue;
               fHisto->FillHisto("hpxy", ap->GetX(), ap->GetY() );
               fHisto->FillHisto("hpzr", ap->GetZ(), ap->GetR() );
               fHisto->FillHisto("hpzp", ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
               //fHisto->FillHisto("hprp", ap->GetR(), ap->GetPhi()*TMath::RadToDeg() );
               fHisto->FillHisto("hprp", ap->GetPhi(), ap->GetR() );
         
               fHisto->FillHisto("hprad", ap->GetR() );
               fHisto->FillHisto("hpphi", ap->GetPhi()*TMath::RadToDeg() );
               fHisto->FillHisto("hpzed", ap->GetZ() );
            }
       
         const TObjArray* tracks = event->GetLineArray();
         int Ntracks = tracks->GetEntries();
         //      std::cout<<"Number of Tracks: "<<Ntracks<<std::endl;


         double Npoints = 0.;
         for(int i=0; i<Ntracks; ++i)
            {
               TStoreLine* aLine = (TStoreLine*) tracks->At(i);
               ProcessLine( aLine );
               Npoints += double(aLine->GetNumberOfPoints());
            }
         fHisto->FillHisto("hNlines", double(Ntracks) );
         if( Ntracks )
            {
               fHisto->FillHisto("hpattreceff",Npoints/double(Ntracks));
               //std::cout<<"PattRecEff: "<<Npoints/double(Ntracks)<<std::endl;
            }

         const TObjArray* helices = event->GetHelixArray();
         int Nhelices = helices->GetEntries();
         //      std::cout<<"Number of Helices: "<<Nhelices<<std::endl;
         Npoints = 0.;
         for(int i=0; i<Nhelices; ++i)
            {
               TStoreHelix* aHelix = (TStoreHelix*) helices->At(i);
               ProcessHelix( aHelix );
               Npoints += double(aHelix->GetNumberOfPoints());
            }
         fHisto->FillHisto("hNhel", double(Nhelices) );
         if( Nhelices )
            {
               fHisto->FillHisto("hhpattreceff",Npoints/double(Nhelices));
            }

         if( Ntracks == 2 )
            {
               TStoreLine* l0 = (TStoreLine*) tracks->At(0);
               TVector3 u0 = *(l0->GetDirection());
               TStoreLine* l1 = (TStoreLine*) tracks->At(1);
               TVector3 u1 = *(l1->GetDirection());
               double cang = u0.Dot(u1);
               fHisto->FillHisto("hlcosang", cang );
               double dist = LineDistance(l0,l1);
               fHisto->FillHisto("hldist", dist );
               fHisto->FillHisto("hlcosangdist", cang, dist );
            }

         // cosmic time distribution
         if( Ntracks >= 2 && Ntracks < 4 )
            //if( Nhelices >= 2 && Nhelices < 4 )
            {
               double delta = (event->GetTimeOfEvent() - temp)*1.e3;
               fHisto->FillHisto("hpois", delta );
               temp = event->GetTimeOfEvent();
            }
         TVector3 vtx = event->GetVertex();
         if(event->GetVertexStatus()>0)
            {
               const TObjArray* used_hel = event->GetUsedHelices();
               fHisto->FillHisto("hNusedhel", double(used_hel->GetEntries()) );
               for(int ih=0; ih<used_hel->GetEntries(); ++ih) ProcessUsed((TFitHelix*) used_hel->At(ih));
               ProcessVertex(&vtx);
               ++Nvtx;
            }
         //      std::cout<<"End of Event"<<std::endl;
      }
   std::cout<<"Number of Events Processed: "<<tin->GetEntries()<<std::endl;
   std::cout<<"Number of Reconstructed Vertexes: "<<Nvtx<<std::endl;
   std::cout<<"Total Runtime: "<<temp<<" s"<<std::endl;
   std::cout<<"Cosmic Rate: "<<Nvtx/temp<<" s^-1"<<std::endl;
}

void ReadEventTree::GetSignalHistos()
{
   std::cout<<"GetSignalHistos"<<std::endl;
   fin->cd();
   fin->cd("/");
   if( fin->cd("awdeconv") )
      {
         hht = (TH1D*)gROOT->FindObject("hNhitTop");
         if( hht ) {
            hht->SetStats(kFALSE);
            hht->SetLineColor(kOrange);
            hht->SetLineWidth(2);
            hht->SetTitle("Number of Hits per Event");
         }

         hot = (TH1D*)gROOT->FindObject("hOccTop");
         if( hot ) {
            hot->SetStats(kFALSE);
            hot->SetLineColor(kOrange);
            hot->SetLineWidth(2);
            hot->SetTitle("Occupancy per Channel");
         }

         htt = (TH1D*)gROOT->FindObject("hTimeTop");
         if( htt ) {
            htt->SetStats(kFALSE);
            htt->SetLineColor(kOrange);
            htt->SetLineWidth(2);
            htt->SetTitle("Drift Time Spectrum after Deconvolution");
         }
      }
   else
      {
         std::cout<<"skipping signals"<<std::endl;
         return;
      }

   if( fin->cd("paddeconv") )
      {
         hhpad = (TH1D*)gROOT->FindObject("hNhitPad");
         if( hhpad ) {
            hhpad->SetStats(kFALSE);
            hhpad->SetLineColor(kBlack);
            hhpad->SetLineWidth(2);
         }

         TH1D* hpadcol = (TH1D*)gROOT->FindObject("hOccCol");
         if( hpadcol ) {
            hocol = new TH1D("hOcccCol2",hpadcol->GetTitle(),256,0.,256.);
            hocol->SetStats(kFALSE);
            for(int b=1; b<=hpadcol->GetNbinsX(); ++b)
               {
                  double bc = hpadcol->GetBinContent(b);
                  // std::cout<<b-1<<"\t";
                  for( int s=0; s<8; ++s )
                     {
                        if( hpadcol->GetEntries() == 0 ) break;
                        double bc = hpadcol->GetBinContent(b);
                        // std::cout<<b-1<<"\t";
                        for( int s=0; s<8; ++s )
                           {
                              hocol->Fill((b-1)*8+s,bc);
                              // std::cout<<(b-1)*8+s<<" ";
                           }
                        // std::cout<<"\n";
                     }
                  // std::cout<<"\n";
               }
            hocol->SetLineColor(kBlack);
            hocol->SetLineWidth(2);

            htpad = (TH1D*)gROOT->FindObject("hTimePad");
            htpad->SetStats(kFALSE);
            htpad->SetLineColor(kBlack);
            htpad->SetLineWidth(2);

            hopad = (TH2D*)gROOT->FindObject("hOccPad");
            hopad->SetTitle("Pad channels Occupancy");
         }
      }

   if( fin->cd("match_el") )
      {
         // aw * pad
         hmatch = (TH1D*)gROOT->FindObject("hNmatch");
         hmatch->SetLineColor(kBlue);
         hmatch->SetLineWidth(2);

         hawpadsector = (TH2D*)gROOT->FindObject("hawcol_sector_time");
      }
}


void ReadEventTree::WriteRunStats()
{
   fout<<"===== Run Stats ====="<<std::endl;
   fout<<"Title\t\tEntries\tMean\tRMS\n";
   fout<<"----------------------------------------------------------------------------------------\n";
   fout<<hht->GetTitle()<<"\t"<<std::setw(8)<<hht->GetEntries()<<"\t"<<std::setw(5)<<hht->GetMean()<<"\t"<<std::setw(5)<<hht->GetRMS()<<std::endl;
   fout<<hhpad->GetTitle()<<"\t"<<std::setw(8)<<hhpad->GetEntries()<<"\t"<<std::setw(5)<<hhpad->GetMean()<<"\t"<<std::setw(5)<<hhpad->GetRMS()<<std::endl;  
   fout<<hmatch->GetTitle()<<"\t"<<std::setw(8)<<hmatch->GetEntries()<<"\t"<<std::setw(5)<<hmatch->GetMean()<<"\t"<<std::setw(5)<<hmatch->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hpzed")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hpzed")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hpzed")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hpzed")->GetRMS()<<std::endl;
   fout<<"\n";
   fout<<fHisto->GetHisto("hpattreceff")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hpattreceff")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hpattreceff")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hpattreceff")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hNlines")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hNlines")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hNlines")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hNlines")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hlcosang")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hlcosang")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hlcosang")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hlcosang")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hldist")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hldist")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hldist")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hldist")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hlchi2")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hlchi2")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hlchi2")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hlchi2")->GetRMS()<<std::endl;
   fout<<"\n";
   fout<<fHisto->GetHisto("hhpattreceff")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hhpattreceff")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hhpattreceff")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hhpattreceff")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hNhel")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hNhel")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hNhel")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hNhel")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hhchi2R")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hhchi2R")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hhchi2R")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hhchi2R")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hhchi2Z")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hhchi2Z")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hhchi2Z")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hhchi2Z")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hpp")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hpp")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hpp")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hpp")->GetRMS()<<std::endl;
   fout<<"\n";
   fout<<fHisto->GetHisto("hNusedhel")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hNusedhel")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hNusedhel")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hNusedhel")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("huhchi2R")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("huhchi2R")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("huhchi2R")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("huhchi2R")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("huhchi2Z")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("huhchi2Z")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("huhchi2Z")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("huhchi2Z")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("huhpp")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("huhpp")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("huhpp")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("huhpp")->GetRMS()<<std::endl;
   fout<<"\n";
   fout<<fHisto->GetHisto("hvr")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hvr")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hvr")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hvr")->GetRMS()<<std::endl;
   fout<<fHisto->GetHisto("hvz")->GetTitle()<<"\t"<<std::setw(8)<<fHisto->GetHisto("hvz")->GetEntries()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hvz")->GetMean()<<"\t"<<std::setw(5)<<fHisto->GetHisto("hvz")->GetRMS()<<std::endl;
   fout<<"\n";
  
   fout<<"\n";
   fout<<fHisto->GetHisto("hNlines")->GetTitle()<<"\t0 line: "<<fHisto->GetHisto("hNlines")->GetBinContent(1)<<"\t1 line: "<<fHisto->GetHisto("hNlines")->GetBinContent(2)<<"\t2 lines: "<<fHisto->GetHisto("hNlines")->GetBinContent(3)<<"\t>2 lines: "<<fHisto->GetHisto("hNlines")->Integral(4,10)<<std::endl;
   fout<<"\n";
   fout<<fHisto->GetHisto("hNhel")->GetTitle()<<"\t0 helixs: "<<fHisto->GetHisto("hNhel")->GetBinContent(1)<<"\t1 helix: "<<fHisto->GetHisto("hNhel")->GetBinContent(2)<<"\t2 helixs: "<<fHisto->GetHisto("hNhel")->GetBinContent(3)<<"\t>2 helixs: "<<fHisto->GetHisto("hNhel")->Integral(4,10)<<std::endl;
   fout.close();
}

void ReadEventTree::ProcessData( )
{
   std::cout<<"ProcessData --> Run Number: "<<RunNumber<<std::endl;
   fout<<"ProcessData --> Run Number: "<<RunNumber<<std::endl;

   MakeHistos();

   GetSignalHistos();

   ProcessTree();

   std::cout<<"DisplayHisto"<<std::endl;
   DisplayHisto();

   std::cout<<"Write Run Stats"<<std::endl;
   WriteRunStats();

   TString logfile = TString::Format("%s/R%d.log",
                                     getenv("AGRELEASE"),RunNumber);
   TString bkpfile = TString::Format("%s/ana/%s/R%d.log",
                                     getenv("AGRELEASE"),savFolder.Data(),
                                     RunNumber);
   copy_file(logfile.Data(),bkpfile.Data());
   std::cout<<"Process Complete"<<std::endl;
}

void ReadEventTree::copy_file( const char* srce_file, const char* dest_file )
{
   std::ifstream srce( srce_file, std::ios::binary ) ;
   std::ofstream dest( dest_file, std::ios::binary ) ;
   dest << srce.rdbuf() ;
}

ReadEventTree::ReadEventTree(TString fname):tag("_R"),RunNumber(0),_save_plots(true)
{
   std::cout<<"DATA"<<std::endl;

   fin=TFile::Open(fname,"READ");
   if( fin->IsOpen() )
      std::cout<<fname<<" FOUND"<<std::endl;
   else
      {
         std::cout<<"file "<<fname<<" not found"<<std::endl;
         return;
      }

   RunNumber = GetRunNumber( fname );
   std::cout<<"Run # "<<RunNumber<<std::endl;
   tag+=RunNumber;

   TString rootdir(getenv("AGRELEASE"));
   rootdir+="/";
   savFolder=MakeAutoPlotsFolder("time",rootdir);
   std::cout<<"Saving plots to: "<<savFolder<<std::endl;

   TString foutname(savFolder+"statR"+RunNumber+".txt");
   std::cout<<"Stat file: "<<foutname<<std::endl;
   fout.open(foutname.Data());
   fout<<"Filename: "<<fname<<std::endl;

   std::string histoname(savFolder+"/plots_R"+RunNumber+".root");
   std::cout<<"Saving histos to:"<<histoname<<std::endl;
   fHisto=new Histo(histoname);
   fin->cd();
   TObjString* sett = (TObjString*) gROOT->FindObject("ana_settings");
   if( sett ) 
      {
         TString str = sett->GetString();
         std::cout<<str<<std::endl;
         fout<<"\n-----------------------------------\n"<<
            str<<"\n-----------------------------------"<<std::endl;
         std::cout<<"Utils::WriteSettings AnaSettings to rootfile... "<<std::endl;
         int status = fHisto->WriteObject(sett,"ana_settings");
         if( status > 0 ) std::cout<<"Utils: Write AnaSettings Success!"<<std::endl;
      }
   else
      std::cout<<"Settings string not found"<<std::endl;

   tin = (TTree*) fin->Get("StoreEventTree");
   std::cout<<tin->GetTitle()<<" has "<<tin->GetEntries()<<" entries"<<std::endl;
   fout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<std::endl;
}

ReadEventTree::~ReadEventTree()
{
   delete fHisto;
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
