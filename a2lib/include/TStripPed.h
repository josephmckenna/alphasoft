#ifndef _TStripPed_
#define _TStripPed_

#include <math.h>
#include <vector>
#include "TObject.h"
class TStripPed: public TObject
{
public:
   std::vector<int>* histo;
   std::vector<int>* rawhisto;
   double sigma=99999.;
   double rawADCMean=0.;
   double rawADCRMS=0.;
   //First pass variables
   double stripMean=0.;
   double stripRMS=0.;
   //Second pass variables
   double filteredMean=0.;
   double StripRMSsAfterFilter;
   
   double stripMeanSubRMS=-9999.;
   
   bool FirstPassFinished=false;
   int DataPoints=0.;

private:
   const double hmax;
   const double hmin;
   const double strip_bin_width;
   const double strip_bins;

public:
   TStripPed();
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
   //double GetRAWMean(const double &_min=-9999999., const double &_max=9999999.);
   //double GetRAWStdev(const double &mean,const double &_min=-9999999., const double &_max=9999999.);
   double GetStdev(const double &mean,const double &_min=-9999999., const double &_max=9999999.);
   double GetRMS(const double& mean,const double &_min=-9999999., const double &_max=9999999.);

   void CalculatePed();
   ClassDef(TStripPed,1)
};


#endif
