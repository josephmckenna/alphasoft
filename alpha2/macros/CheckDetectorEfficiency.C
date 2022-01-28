

const double MIN_COUNTS = 1000;
const double MIN_DUMP_LENGTH = 2;
const double MAX_DUMP_LENGTH = 10;
const double MIN_REPS = 0;


std::vector<std::pair<double, double>> CheckDetectorEfficiency(int runno, const char* det1, const char* det2, const char* dump_name)
{
   TSISChannels ch(runno);
   int PMTCH = ch.GetChannel( det1, runno);
   int SiCH =  ch.GetChannel( det2, runno);
   std::vector<TA2Spill> hots = Get_A2_Spills(runno, {dump_name}, {-1});
   
   std::vector<std::pair<double, double>> counts;
   
   for (TA2Spill& h: hots)
   {
      if (h.ScalerData)
      {
         if (h.ScalerData->DetectorCounts[PMTCH] > MIN_COUNTS )
         {
            if (h.ScalerData->DetectorCounts[SiCH] > MIN_COUNTS )
            {
               double dump_length = h.GetStopTime() - h.GetStartTime();
               if (dump_length > MIN_DUMP_LENGTH && dump_length < MAX_DUMP_LENGTH)
                  counts.emplace_back(std::make_pair(h.ScalerData->DetectorCounts[PMTCH], h.ScalerData->DetectorCounts[SiCH]));
            }
         }
      }
    }
    return counts;
}

TCanvas* PlotDetectorEfficiency(int start, int stop, const char* det1 = "SIS_PMT_CATCH_OR", const char* det2 = "CT_SiPM_OR", const char* dump_name = "Hot Dump")
{
   uint32_t first_time = -1; //+inf
   uint32_t last_time = 0;
   TH1D* h = new TH1D("PMTvsSi","PMTvsSi",stop - start, start, stop);
   std::vector<double> x,y,ex,ey;
   for ( int i = start; i < stop; i ++)
   {
      std::vector<std::pair<double,double>> counts = 
         CheckDetectorEfficiency(i,det1,det2,dump_name);
      if (counts.size() > MIN_REPS)
      {
         uint32_t start_time = Get_A2Analysis_Report(i).GetRunStartTime();
         if (start_time) 
            first_time = std::min( first_time, start_time);
         last_time = std::max( last_time, start_time );
         double PMT = 0;
         double Si = 0;
         for (std::pair<double,double>& c: counts)
         {
            PMT += c.first;
            Si += c.second;
         }
         double PMT_2 = sqrt(PMT);
         double Si_2 = sqrt(Si);
         std::cout << PMT << " (±"<< PMT_2<< ") / " << Si << " (±"<< Si_2 << ")"<< std::endl;
         x.push_back(start_time);
         ex.push_back(1);
         y.push_back(PMT/Si);
         double err = pow( PMT_2 / PMT , 2) + pow( Si_2 / Si , 2);
         
         ey.push_back( y.back() * sqrt(err));
//         h->Fill(i, PMT / Si);
      }
   }
   TGraphErrors* g = new TGraphErrors(x.size(), x.data(), y.data(), ex.data(), ey.data());
   std::string title(det1);
   title += " / ";
   title += det2;
   g->SetTitle(title.c_str());
   g->GetXaxis()->SetTimeDisplay(1);
   g->GetXaxis()->SetLabelSize(0.03);
   std::cout << first_time << " - " << last_time <<std::endl; 
   std::cout <<"Range: " << ( last_time - first_time ) / (24*60*60)<<std::endl;
  g->GetXaxis()->SetTimeFormat("%d/%m/%y %H:%M %F 1970-01-01 01:00:00");
   
   TCanvas* c = new TCanvas();
   g->Draw("AP");
   return c;
}
