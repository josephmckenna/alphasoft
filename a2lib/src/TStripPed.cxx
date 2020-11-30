#include "TStripPed.h"
#include <TH1F.h>
//#include <iostream>
ClassImp(TStripPed);

TStripPed::TStripPed(const int nBins, const double binWidth):
   hmax(nBins),
   hmin(-hmax),
   strip_bin_width(binWidth),
   strip_bins((hmax-hmin)/strip_bin_width)
{
   //First pass variables
   stripMean=0.;
   stripRMS=0.;

   //Second pass variables
   filteredMean=0.;
   StripRMSsAfterFilter=0.;
   stripMeanSubRMS=-9999.;
   
   FirstPassFinished=false;
   DataPoints=0.;
   //histo=new std::vector<int>((int)strip_bins,0);
   //histo=new std::vector<int>((int)1,0);
   //LMG
   histo1=new TH1F("histo1", "Histogram", strip_bins, hmin, hmax);

   
}

TStripPed::~TStripPed()
{
   //histo->clear();
   //delete histo;

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
   // histo->at(bin)++;
   histo1->Fill(x);

}

double TStripPed::GetMean(const double &_min, const double &_max)
{
   histo1->GetXaxis()->SetRangeUser(_min, _max);
   double meanFromHist = histo1->GetMean();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   return meanFromHist;
}

double TStripPed::GetStdev(const double& mean,const double& _min, const double& _max)
{
   //histo1->GetXaxis()->SetRangeUser(_min, _max);
   double stdDevFromHist = histo1->GetStdDev();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   return stdDevFromHist;
}

double TStripPed::GetRMS(const double& mean,const double& _min, const double& _max)
{
   //histo1->GetXaxis()->SetRangeUser(_min, _max);
   double RMSFromHist = histo1->GetRMS();
   //histo1->GetXaxis()->SetRangeUser(0, 0);
   return RMSFromHist;
}

void TStripPed::CalculatePed()
{
   if (!FirstPassFinished)
   {
      FirstPassFinished=true;

      rawADCMean=rawADCMean/(double)DataPoints;
      rawADCRMS=rawADCRMS/(double)DataPoints - rawADCMean*rawADCMean;

      //stripRMS = histo1->GetRMS();
      //stripMean = histo1->GetMean();
      //printf("New means: \tstripMean:%f \tstripRMS:%f \t",stripMean,stripRMS);

      stripMean=stripMean/(double)DataPoints;
      stripRMS=stripRMS/(double)DataPoints - stripMean * stripMean;
      printf("rawMean:%f \tstripMean:%f \tstripRMS:%f \t",rawADCMean,stripMean,stripRMS);

      if (stripRMS>0.)
         stripRMS=sqrt(stripRMS);
      //printf("rawMean:%f \tstripMean:%f \tstripRMS:%f \t",rawADCMean,stripMean,stripRMS);
   }

   //Find range around mean (filter)
   double min=stripMean-sigma*stripRMS;
   double max=stripMean+sigma*stripRMS;

   stripMean=GetMean(min,max);

   stripRMS=GetStdev(stripMean,min,max);

   printf("StripRMSsAfterFilter:%f \tstripMeanSubRMS:%f\n",StripRMSsAfterFilter,stripMeanSubRMS);

}
