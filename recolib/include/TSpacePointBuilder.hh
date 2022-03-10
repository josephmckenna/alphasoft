#ifndef __SPACEPOINTBUILDER__
#define __SPACEPOINTBUILDER__

#include <vector>
#include "SignalsType.hh"
#include "TSpacePoint.hh"
#include "AnaSettings.hh"
#include "LookUpTable.hh"
class TSpacePointBuilder
{
   private:
      AnaSettings* ana_settings;
      double fMagneticField;
      const bool fTrace;
      LookUpTable* fSTR;

      std::string fLocation;
      
      double f_rfudge;
      double f_pfudge;
   public:

   TSpacePointBuilder(std::string json, double B, bool trace = false);
   TSpacePointBuilder(AnaSettings* ana_set, double B, std::string loc, bool trace = false);
   ~TSpacePointBuilder()
   {
      delete fSTR;
   }

   inline void SetFudgeFactors(double fdgr, double fdgp) {f_rfudge=fdgr; f_pfudge=fdgp;}
   inline void GetFudgeFactors(double& fdgr, double& fdgp) const {fdgr=f_rfudge; fdgp=f_pfudge;}

   void UseSTRfromData(int runNumber)
   {
      delete fSTR;
      std::cout<<"Reco::UseSTRfromData( "<<runNumber<<" )"<<std::endl;
      fSTR = new LookUpTable(runNumber);
      fMagneticField=0.; // data driven STR valid only for B=0T   
   }

   void BuildSpacePointArray( 
      const std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> > spacepoints,
      std::vector<TSpacePoint> &PointsArray,
      double z_fid  = std::numeric_limits<double>::infinity() 
      );

};


#endif