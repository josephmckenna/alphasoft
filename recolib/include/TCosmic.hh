// Cosmic Track class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: May 2019

#ifndef __TCOSMIC__
#define __TCOSMIC__ 1

#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"
#include "TSpacePoint.hh"

#include <TObjArray.h>
#include <vector>

class TCosmic: public TFitLine
{
public:
   TCosmic();
   TCosmic(TFitHelix*,TFitHelix*,double);
   TCosmic(TFitLine*,TFitLine*);
   TCosmic(TStoreHelix*,TStoreHelix*,double);
   TCosmic(TStoreLine*,TStoreLine*);

   void Fit();

   void SetMagneticField(double b) { fMagneticField = b; }

   double GetDCA() const { return fDCA; }
   double GetCosAngle() const { return fCosAngle; }
   double GetAngle() const { return fAngle; }

private:
   double fMagneticField;
   double fDCA;
   double fCosAngle;
   double fAngle;

   double fvstart[9];

   int AddAllPoints(const TObjArray*, const TObjArray*);
   int AddAllPoints(const std::vector<TSpacePoint*>*,
                    const std::vector<TSpacePoint*>*);
   int CalculateHelDCA(TStoreHelix*, TStoreHelix*);
   int CalculateHelDCA(TFitHelix*, TFitHelix*);
   double LineDistance(TStoreLine*, TStoreLine*);

   void Initialization();

   ClassDef(TCosmic,1)
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
