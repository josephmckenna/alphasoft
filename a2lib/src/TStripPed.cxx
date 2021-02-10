#include "TStripPed.h"
#include <TH1F.h>
#include <TCanvas.h>
#include <iostream>
#include "TF1.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TRootCanvas.h"
#include "TFile.h"

ClassImp(TStripPed);

TStripPed::TStripPed(const int ID, const int nBins, const double binWidth):
   hmax(nBins),
   hmin(-hmax),
   strip_bin_width(binWidth),
   strip_bins((hmax-hmin)/strip_bin_width)
{
   //First pass variables
   stripMean=0.;
   stripRMS=0.;
   //Second pass variables
   stripMeanSubRMS=0.;
   StripRMSsAfterFilter=0.;

   stripMeanSubRMS=-9999.;
   
rawADCMean=0;
rawADCRMS=0;
   FirstPassFinished=false;
   DataPoints=0.;
   histo=new std::vector<int>((int)strip_bins,0);
   rawhisto=new std::vector<int>(1025,0);
   //LMG
   histo1=new TH1D(std::to_string(ID).c_str(), "Histogram", strip_bins, hmin, hmax);

   
}

TStripPed::~TStripPed()
{
   histo->clear();
   delete histo;
   rawhisto->clear();
   delete rawhisto;
}

void TStripPed::InsertValue(const double &x, const double &rawADC)
{
   //if (fabs(x)>1024) return;
   //if (fabs(x)> 1024 ) { printf("Fucking filter:%f\n",x); }
   if (rawADC >= 1024) return;
   
   const int bin=GetBin(x);
   //Check if in range
   if (bin>=strip_bins)
   {
      std::string name1(histo1->GetName());
      if(name1 == "4" || name1 == "13" || true)
      {
         //printf("Discarding value %f in Histo: %s \n",x, name1.c_str());
      }
      return;
   }
   if (bin<0)
   {
      std::string name1(histo1->GetName());
      if(name1 == "4" || name1 == "13" || true)
      {
         //printf("Discarding value %f in Histo: %s \n",x, name1.c_str());
      }
      return;
   }
   //std::cout<<bin<<">"<<strip_bins<<std::endl;
   rawADCMean+=rawADC;
   rawADCRMS +=rawADC*rawADC;
   stripMean +=x;
   stripRMS  +=x*x;
   DataPoints++;
   
   histo1->Fill(x);
   histo->at(bin)++;
   (*rawhisto)[(int)rawADC]++;
}

double TStripPed::GetRAWMean(const int & min, const int & max)
{
   double mean=0.;
   int count=0;
   for (int i=min; i<max; i++)
   {
     // printf("bin:%d\tx:%f\n",i,x);
      count++;
      mean+=(double)(i*(*rawhisto)[i]);
   }
   return mean/(double)count;
}

double TStripPed::GetMean(const double &_min, const double &_max)
{
   
   double mean=0.;
   int counts=0;
   int stop=GetBin(_max);
   if (stop>strip_bins)
      stop=strip_bins;
   int start=GetBin(_min);
   if (start<0)
      start=0;
   double x=GetX(start);
   //printf("GetMeanDataVector: min = %f, \t max = %f, \t start = %d, \t stop = %d, \t x_min = %f, \t x_max = %f \n",_min,_max,start,stop,GetX(start),GetX(stop));
   //printf("GetMeanDataHistogram: min = %f, \t max = %f, \t start = %d, \t stop = %d, \t x_min = %f, \t x_max = %f \n",histo1->GetXaxis()->GetXmin(),histo1->GetXaxis()->GetXmax(),histo1->GetMinimumBin(),histo1->GetMaximumBin(),histo1->GetMinimum(),histo1->GetMaximum());
   //x += strip_bin_width/2;
   //Changed from < to <=
   for (int i=start; i<=stop; i++)
   {
      
      const double count=(double)(*histo)[i];
      //printf("bin:%d\tx:%f\t%f\n",i,x,count);
      counts+=count;
      mean+=x*(double)count;
      x+=strip_bin_width;
   }
   //histo1->Print("all");
   double output = mean/(double)counts;
   double histooutput = histo1->GetMean();
   //printf("Counts Before: \t %d, \t %f \n", counts, histo1->Integral());
   histo1->GetXaxis()->SetRangeUser(_min, _max);
   double histooutputafterfilter = histo1->GetMean();
   //printf("Counts After: \t %d, \t %f \n", counts, histo1->Integral());
   histo1->GetXaxis()->SetRangeUser(0, 0);
   double histooutputafterreset = histo1->GetMean();
   //printf("Mean: \t %f,\t %f,\t %f,\t %f,\n", output, histooutput, histooutputafterfilter, histooutputafterreset);

   return output;

   //return mean/(double)counts;
}

double TStripPed::GetStdev(const double& mean,const double& _min, const double& _max)
{
   int counts=0;
   double stdev=0.;
   int stop=GetBin(_max);
   if (stop>strip_bins)
      stop=strip_bins;
   int start=GetBin(_min);
   if (start<0)
      start=0;
   const double x=GetX(start);
   double diff=x-mean;
   //Changed from <= to <
   for (int i=start; i<=stop; i++)
   {
      const double count=(double)(*histo)[i];
      stdev+=count*diff*diff;
      counts+=count;
      diff+=strip_bin_width;
   }
   double output = sqrt(stdev/(double)counts);
   double histooutput = histo1->GetStdDev();
   histo1->GetXaxis()->SetRangeUser(_min, _max);
   double histooutputafterfilter = histo1->GetStdDev();
   histo1->GetXaxis()->SetRangeUser(0, 0);
   double histooutputafterreset = histo1->GetStdDev();
   return output;
   
   //histo1->GetXaxis()->SetRangeUser(_min, _max);
   double stdDevFromHist = histo1->GetStdDev();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   //return stdDevFromHist;
}

double TStripPed::GetRAWStdev(const double& mean, int min, int max)
{
   int counts=0;
   double stdev=0.;
   
   if (min<0) min=0;
   if (max>1024) max=1024;

   for (int i=min; i<=max; i++)
   {
      const double diff=(double)i-mean;
      const double count=(double)rawhisto->at(i);
      stdev+=count*diff*diff;
      counts+=count;
   }

   int newmin = -512 + min * strip_bin_width;
   int newmax = -512 + max * strip_bin_width;

   

   double output = sqrt(stdev/(double)counts);
   double histooutput = histo1->GetStdDev();

   /*std::string name1(histo1->GetName());
   TFile *f1 = new TFile(name1.append("Before.root").c_str(),"UPDATE");
   f1->WriteTObject(histo1);
   f1->Close();
   //histo1->Print("all");
   
   histo1->GetXaxis()->SetRangeUser(newmin, newmax);
   std::string name2(histo1->GetName());
   TFile *f2 = new TFile(name2.append("During.root").c_str(),"UPDATE");
   f2->WriteTObject(histo1);*/
   double histooutputafterfilter = histo1->GetStdDev();
   /*f2->Close();
   
   histo1->GetXaxis()->SetRangeUser(0, 0);
   std::string name3(histo1->GetName());
   TFile *f3 = new TFile(name3.append("Reset.root").c_str(),"UPDATE");
   f3->WriteTObject(histo1);
   f3->Close();*/

   double histooutputafterreset = histo1->GetStdDev();
   //printf("RMS: \t %f,\t %f,\t %f,\t %f,\n", output, histooutput, histooutputafterfilter, histooutputafterreset);

   /*int argc = 1;
   char** argv;
   TApplication app("app", &argc, argv);
   TCanvas* c = new TCanvas("c", "Something", 0, 0, 800, 600);
   TH1F h1("h1","Histo from a Gaussian",100,-3,3);
   h1.FillRandom("gaus",10000);
   h1.Draw();
   c->Modified(); c->Update();
   TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
   rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
   app.Run();*/

   return output;
}

double TStripPed::GetRMS(const double& mean,const double& _min, const double& _max)
{
   int counts=0;
   double RMS=0.;
   int stop=GetBin(_max);
   if (stop>strip_bins)
      stop=strip_bins;
   int start=GetBin(_min);
   if (start<0)
      start=0;
   double x=GetX(start);
   for (int i=start; i<stop; i++)
   {
      const double count=(double)(*histo)[i];
      RMS+=count*x*x;
      counts+=count;
      x+=strip_bin_width;
   }
   double output = sqrt(RMS/(double)counts);
   double histooutput = histo1->GetRMS();
   histo1->GetXaxis()->SetRangeUser(_min, _max);
   double histooutputafterfilter = histo1->GetRMS();
   histo1->GetXaxis()->SetRangeUser(0, 0);
   double histooutputafterreset = histo1->GetRMS();
   //printf("\n\nRMS: \t %f,\t %f,\t %f,\t %f,\t", output, histooutput, histooutputafterfilter, histooutputafterreset);
   return output;
   //return sqrt(RMS/(double)counts);

   //histo1->GetXaxis()->SetRangeUser(_min, _max);
   //double RMSFromHist = histo1->GetRMS();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   //return RMSFromHist;
}

void TStripPed::CalculatePed()
{

   FirstPassFinished=true;

   rawADCMean=rawADCMean/(double)DataPoints;
   rawADCRMS=rawADCRMS/(double)DataPoints - rawADCMean*rawADCMean;
   rawADCRMS=sqrt(rawADCRMS);
   stripMean=stripMean/(double)DataPoints;
   stripRMS=stripRMS/(double)DataPoints - stripMean * stripMean;
   if (stripRMS>0.)
      stripRMS=sqrt(stripRMS);
   //printf("%d\trawMean:%f \tstripMean:%f \tstripRMS:%f \n",DataPoints,rawADCMean,stripMean,stripRMS);

   //Find range around mean (filter)
   double min=stripMean-sigma*stripRMS + strip_bin_width/2;
   double max=stripMean+sigma*stripRMS - strip_bin_width/2;

   int rawmin=rawADCMean - sigma*stripRMS;
   int rawmax=rawADCMean + sigma*stripRMS;

   //Recalculate mean in range
   // double clean_mean=GetMean(min,max);
   //double preRMS=stripRMS;
   double initialRMS = stripRMS;
   //StripRMSsAfterFilter=GetRAWStdev(rawADCMean,min,max);
   //stripMeanSubRMS=StripRMSsAfterFilter-GetRAWMean(min,max);
   //printf("\nstripMean:%f\t",stripMean);
   //stripMeanSubRMS=GetMean(min,max);
   stripRMS=GetRAWStdev(rawADCMean,rawmin,rawmax);
   
   //printf("\nstripMean:%f\n",stripMean);
   stripMeanSubRMS=GetMean(min,max);
   StripRMSsAfterFilter=GetStdev(stripMean,min,max);
/*printf("stripMean2:%f\t",stripMean);
stripMean=GetMean();
printf("stripMean3:%f\n",stripMean);
stripMean=GetMean(-sigma*rawADCRMS,sigma*rawADCRMS);
printf("stripMean4:%f\n",stripMean);
*/


//   stripMeanSubRMS=GetRAWStdev(rawADCMean,min,max);
   //printf("Mins: \t %f,\t %f,,\t %f,\n", min, max, histo1->GetMean());

   //printf("stripMeanSubRMS:%f\tStripRMSsAfterFilter:%f\n",stripMeanSubRMS,StripRMSsAfterFilter);
   //printf("Histo Mean:%f\t Histo RMS:%f\t Histo STDDEV:%f\n",histo1->GetMean(),histo1->GetRMS(), histo1->GetStdDev());
   //printf("\n\nMean difference: %f\t STDDEV/RMS difference: %f", stripMean - histo1->GetMean(), initialRMS - histo1->GetStdDev());
   histo1->GetXaxis()->SetRangeUser(min, max);
   //printf("Histo Mean:%f\t Histo RMS:%f\t Histo STDDEV:%f\n",histo1->GetMean(),histo1->GetRMS(), histo1->GetStdDev());
   //printf("\nMean difference: %f\t STDDEV/RMS difference: %f\n\n", stripMeanSubRMS - histo1->GetMean(), StripRMSsAfterFilter - histo1->GetStdDev());
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   
   Double_t givenMean = stripMeanSubRMS;
   Double_t histomean = histo1->GetMean();

   /*if((givenMean - histomean) > 0.001)
   {
      histo1->GetXaxis()->SetRangeUser(0, 0);
      std::string name1(histo1->GetName());
      //printf("Error in the means. Printing data for debug. Histo: %s", name1.c_str());
      TFile *f1 = new TFile(name1.append("HistoBefore.root").c_str(),"UPDATE");
      f1->WriteTObject(histo1);
      f1->Close();

      histo1->GetXaxis()->SetRangeUser(min, max);
      TFile *f2 = new TFile(name1.append("HistoAfter.root").c_str(),"UPDATE");
      f2->WriteTObject(histo1);
      f2->Close();

      histo1->GetXaxis()->SetRangeUser(0, 0);
   }*/
   bool usingOldMethod = true;  
   if(usingOldMethod)
   {
      stripMeanSubRMS = stripMean;
      StripRMSsAfterFilter = StripRMSsAfterFilter;
   }
   else
   {
      histo1->GetXaxis()->SetRangeUser(0, 0);
      histo1->GetXaxis()->SetRangeUser(min, max);
      stripMeanSubRMS = histo1->GetMean();
      StripRMSsAfterFilter = histo1->GetStdDev();
      histo1->GetXaxis()->SetRangeUser(0, 0);
   }
   
   
   //printf("RawMean:%f\tRMS:%f\tmin:%f\tmax:%f\tNewRMS:%f\n",rawADCMean,0,min,max,stripRMS);
   //First pass variables
   //printf("stripMean:%f\tRMS:\double stripMean=0.;
   //double stripRMS=0.;
   //Second pass variables
   //double filteredMean=0.;
   //double StripRMSsAfterFilter;

   /*
   //double clean_stdev=GetStdev(stripMean,min,max);
   {
   //Find range around mean (filter)
   double min=stripMean-sigma*stripRMS;
   double max=stripMean+sigma*stripRMS;
   StripRMSsAfterFilter=GetStdev(stripMean,min,max);
   }*/
   
   //printf("\nmean:%f\tstdev:%f\tmin:%f\tmax:%f\tclean_mean:%f\tclean_rms:%f\n", stripMean,stripRMS,min,max,clean_mean,clean_RMS);
   
   //stripRMS=StripRMSsAfterFilter;
   //stripMeanSubRMS=stripRMS-rawADCMean;

}
