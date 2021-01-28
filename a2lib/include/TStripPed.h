#ifndef _TStripPed_
#define _TStripPed_

#include <math.h>
#include <vector>
#include "TObject.h"
#include <TH1.h>
class TStripPed: public TObject
{
public:
   std::vector<int>* histo;
   TH1F* histo1;
   std::vector<int>* rawhisto;
   double sigma=99999.;
   double rawADCMean=0.;
   double rawADCRMS=0.;
   //First pass variables
   double stripMean=0.;
   double stripRMS=0.;
   //Second pass variables
   double stripMeanSubRMS=-9999.; //filtered mean
   double StripRMSsAfterFilter=0;
   
   
   
   bool FirstPassFinished=false;
   int DataPoints=0.;

private:
   const double hmax;
   const double hmin;
   const double strip_bin_width;
   const double strip_bins;

public:
   TStripPed(const int ID, const int nBins, const double binWidth);
   virtual ~TStripPed();
   inline int GetBin(const double &x)
   {
      return (x - hmin) / strip_bin_width;
   }
   inline double GetX(const int &x)
   {
      return x*strip_bin_width+hmin;
   }
   
   void InsertValue(const double &ped_sub, const double &raw_adc);
   double GetMean(const double &_min=-9999999., const double &_max=9999999.);
   double GetStdev(const double &mean,const double &_min=-9999999., const double &_max=9999999.);

   double GetRAWMean(const int &_min=-9999999., const int &_max=9999999.);
   double GetRAWStdev(const double &mean,int _min=-9999999., int _max=9999999.);

   double GetRMS(const double& mean,const double &_min=-9999999., const double &_max=9999999.);

   void CalculatePed();
   ClassDef(TStripPed,1)
};


#endif
