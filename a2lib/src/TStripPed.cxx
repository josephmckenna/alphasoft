#include "TStripPed.h"

ClassImp(TStripPed);

TStripPed::TStripPed():
   hmax(512),
   hmin(-hmax),
   strip_bin_width(0.1),
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
   histo=new std::vector<int>((int)strip_bins,0);
   rawhisto=new std::vector<int>(1024,0);
   
}

TStripPed::~TStripPed()
{
   histo->clear();
   delete histo;
}

void TStripPed::InsertValue(const double &x, const double &rawADC)
{
   //if (fabs(x)>1024) return;
   //if (fabs(x)> 1024 ) { printf("Fucking filter:%f\n",x); }
   if (rawADC<0) return; //No strip!
   if (rawADC>1024) { printf("FuckingRAW:%f",rawADC); return;}
   rawADCMean+=rawADC;
   rawADCRMS+=rawADC*rawADC;
   stripMean+=x;
   stripRMS+=x*x;
   DataPoints++;
   const int bin=GetBin(x);
   //Check if in range
   if (bin>strip_bins) return;
   if (bin<0) return;
   (*histo)[bin]++;
   (*rawhisto)[(int)rawADC]++;
}

double TStripPed::GetRAWMean(const double &_min, const double &_max)
{
   double mean=0.;
   int count=0;
   int stop=(int)_max;
   if (stop>strip_bins)
      stop=strip_bins;
   int start=(int)_min;
   if (start<0)
      start=0;

   for (int i=start; i<stop; i++)
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
   for (int i=start; i<stop; i++)
   {
      
      const double count=(double)(*histo)[i];
      //printf("bin:%d\tx:%f\t%f\n",i,x,count);
      counts+=count;
      mean+=x*(double)count;
      x+=strip_bin_width;
   }
   return mean/(double)counts;
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
}
double TStripPed::GetRAWStdev(const double& mean,const double& _min, const double& _max)
{
   int counts=0;
   double stdev=0.;
   int stop=(int)_max;
   if (stop>strip_bins)
      stop=strip_bins;
   int start=(int)_min;
   if (start<0)
      start=0;
   
   for (int i=start; i<stop; i++)
   {
      const double diff=(double)i-mean;
      const double count=(double)(*rawhisto)[i];
      stdev+=count*diff*diff;
      counts+=count;
   }
   return sqrt(stdev/(double)counts);
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
}

void TStripPed::CalculatePed()
{
   if (!FirstPassFinished)
   {

      FirstPassFinished=true;
      rawADCMean=rawADCMean/(double)DataPoints;
      rawADCRMS=rawADCRMS/(double)DataPoints - rawADCMean*rawADCMean;
      stripMean=stripMean/(double)DataPoints;
      stripRMS=stripRMS/(double)DataPoints - stripMean * stripMean;
      if (stripRMS>0.)
         stripRMS=sqrt(stripRMS);
      printf("rawMean:%f \tstripMean:%f \tstripRMS:%f \t",rawADCMean,stripMean,stripRMS);

   }

   //Find range around mean (filter)
   double min=rawADCMean-sigma*stripRMS;
   double max=rawADCMean+sigma*stripRMS;

   //Recalculate mean in range
   // double clean_mean=GetMean(min,max);
   //double preRMS=stripRMS;

   //StripRMSsAfterFilter=GetRAWStdev(rawADCMean,min,max);
   //stripMeanSubRMS=StripRMSsAfterFilter-GetRAWMean(min,max);
   printf("\nstripMean:%f\t",stripMean);
stripMean=GetMean(-sigma*stripRMS,sigma*stripRMS);
printf("\nstripMean:%f\n",stripMean);
stripRMS=GetStdev(stripMean,-sigma*stripRMS,sigma*stripRMS);
/*printf("stripMean2:%f\t",stripMean);
stripMean=GetMean();
printf("stripMean3:%f\n",stripMean);
stripMean=GetMean(-sigma*rawADCRMS,sigma*rawADCRMS);
printf("stripMean4:%f\n",stripMean);
*/


   stripMeanSubRMS=GetRAWStdev(rawADCMean,min,max);

   printf("StripRMSsAfterFilter:%f \tstripMeanSubRMS:%f\n",StripRMSsAfterFilter,stripMeanSubRMS);
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
