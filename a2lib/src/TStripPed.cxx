#include "TStripPed.h"
#include <TH1F.h>
//#include <iostream>
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
   histo=new std::vector<int>((int)1,0);
   //LMG
   histo1=new TH1F(std::to_string(ID).c_str(), "Histogram", strip_bins, hmin, hmax);

   
}

TStripPed::~TStripPed()
{
   histo->clear();
   delete histo;

   //LMG
   //histo1->Clear();
   
   if(histo1) 
   {
      delete histo1;
      histo1 = NULL;
   }
}

void TStripPed::InsertValue(const double &x, const double &rawADC)
{
   if (rawADC > 1024) return;
   rawADCMean+=rawADC;
   rawADCRMS +=rawADC*rawADC;
   stripMean +=x;
   stripRMS  +=x*x;
   DataPoints++;
   const int bin=GetBin(x);
   //Check if in range
   if (bin>=strip_bins) return;
   if (bin<0) return;
   //histo->at(bin)++;
   histo->at(bin)++;
   (*rawhisto)[(int)rawADC]++;
   histo1->Fill(x);

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
   for (int i=start; i<stop; i++)
   {
      
      const double count=(double)(*histo)[i];
      //printf("bin:%d\tx:%f\t%f\n",i,x,count);
      counts+=count;
      mean+=x*(double)count;
      x+=strip_bin_width;
   }
   return mean/(double)counts;

   histo1->GetXaxis()->SetRangeUser(_min, _max);
   double meanFromHist = histo1->GetMean();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   //return meanFromHist;
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
   for (int i=start; i<stop; i++)
   {
      const double count=(double)(*histo)[i];
      stdev+=count*diff*diff;
      counts+=count;
      diff+=strip_bin_width;
   }
   return sqrt(stdev/(double)counts);

   //histo1->GetXaxis()->SetRangeUser(_min, _max);
   double stdDevFromHist = histo1->GetStdDev();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   //return stdDevFromHist;
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
   return sqrt(RMS/(double)counts);

   //histo1->GetXaxis()->SetRangeUser(_min, _max);
   double RMSFromHist = histo1->GetRMS();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   //return RMSFromHist;
}

void TStripPed::CalculatePed()
{

   FirstPassFinished=true;

   rawADCMean=rawADCMean/(double)DataPoints;
   rawADCRMS=rawADCRMS/(double)DataPoints - rawADCMean*rawADCMean;
   rawADCRMS=sqrt(rawADCRMS);

   //stripRMS = histo1->GetRMS();
   //stripMean = histo1->GetMean();
   //printf("New means: \tstripMean:%f \tstripRMS:%f \t",stripMean,stripRMS);

   stripMean=stripMean/(double)DataPoints;
   stripRMS=stripRMS/(double)DataPoints - stripMean * stripMean;

   if (stripRMS>0.)
      stripRMS=sqrt(stripRMS);

   printf("rawMean:%f \tstripMean:%f \tstripRMS:%f \t",rawADCMean,stripMean,stripRMS);
   

   //Find range around mean (filter)
   double min=stripMean-sigma*stripRMS;
   double max=stripMean+sigma*stripRMS;

   int rawmin=rawADCMean - sigma*stripRMS;
   int rawmax=rawADCMean + sigma*stripRMS;
   stripMean=GetMean(min,max);
   //stripRMS=GetRAWStdev(rawADCMean,rawmin,rawmax);
   
   //printf("\nstripMean:%f\n",stripMean);
   StripRMSsAfterFilter=GetStdev(stripMean,min,max);
   
   printf("StripRMSsAfterFilter:%f \tstripMeanSubRMS:%f\n",StripRMSsAfterFilter,stripMeanSubRMS);

}
