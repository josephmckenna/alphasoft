
void RemoveZero() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   // TH2D* hHitsAdcVTdc = new TH2D("hHitsAdcVTdc","Number of hits on ADC vs TDC per bar end per event;ADC hits;TDC hits",10,-0.5,9.5);
   //TH2D* hHitsBotVTop = new TH2D("hHitsBotVTop","Number of hits on top vs bottom per bar per event;Bottom hits;Top hits",10,-0.5,9.5);
   TH2D* hHitsAdcVTdc;
   TH2D* hHitsAdcVTdc2;
   TH2D* hHitsBotVTop;
   TH2D* hHitsBotVTop2;

   // Loads zed histogram
   fin->GetObject("/bsc_tdc_module/hHitsAdcVTdc",hHitsAdcVTdc);
   fin->GetObject("/bsc_tdc_module/hHitsBotVTop",hHitsBotVTop);

   hHitsAdcVTdc2 = (TH2D*)hHitsAdcVTdc->Clone("hHitsAdcVTdc2");
   hHitsBotVTop2 = (TH2D*)hHitsBotVTop->Clone("hHitsBotVTop2");

   hHitsAdcVTdc2->SetBinContent(1,1,0);
   hHitsBotVTop2->SetBinContent(1,1,0);

}
