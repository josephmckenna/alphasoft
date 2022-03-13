#include "TSpacePointBuilder.hh"

TSpacePointBuilder::TSpacePointBuilder(std::string json, double B, bool trace ):
   fMagneticField (B), fTrace(trace), fLocation("CERN"),f_rfudge(1.),f_pfudge(1.)
{
   ana_settings = new AnaSettings(json.c_str());
   if( fMagneticField < 0. )
      fSTR = new LookUpTable(ALPHAg::_co2frac); // field map version (simulation)
   else
      fSTR = new LookUpTable(ALPHAg::_co2frac, fMagneticField, fLocation); // uniform field version (simulation)
   std::cout<<"Reco::Reco()  max time: "<<fSTR->GetMaxTime()<<" ns"<<std::endl;
}

TSpacePointBuilder::TSpacePointBuilder(AnaSettings* ana_set, double B, std::string loc, bool trace ):
   ana_settings(ana_set), fMagneticField (B), fTrace(trace), fLocation(loc),f_rfudge(1.),f_pfudge(1.)
{
   if( fMagneticField < 0. ) // garfield++ sim with field map
   {
      fSTR = new LookUpTable(ALPHAg::_co2frac); // field map version (simulation)
      fMagneticField = 1.;
   }
   else
   {
      fSTR = new LookUpTable(ALPHAg::_co2frac, fMagneticField, fLocation); // uniform field version (simulation)
   }
   std::cout<<"Reco::Reco()  max time: "<<fSTR->GetMaxTime()<<" ns"<<std::endl;
}

void TSpacePointBuilder::BuildSpacePointArray( 
    const std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> > spacepoints,
    std::vector<TSpacePoint> &PointsArray,
    double z_fid// Z cut by default off
   )
{
   int n = PointsArray.size();
   PointsArray.reserve(PointsArray.size() + spacepoints.size());
   for( auto sp=spacepoints.begin(); sp!=spacepoints.end(); ++sp )
      {
         // STR: (t,z)->(r,phi)
         const double time = sp->first.t, zed = sp->second.z;
         if( fabs(zed) > z_fid ) continue;
         if( fTrace )
            {
               double z = ( double(sp->second.idx) + 0.5 ) * ALPHAg::_padpitch - ALPHAg::_halflength;
               std::cout<<"Reco::AddSpacePoint "<<n<<" aw: "<<sp->first.idx
                        <<" t: "<<time
                        <<"\tcol: "<<sp->second.sec<<" row: "<<sp->second.idx<<" (z: "<<z
                        <<") ~ "<<sp->second.z<<" err: "<<sp->second.errz<<std::endl;
            }
         double r = fSTR->GetRadius( time , zed ),
            correction = fSTR->GetAzimuth( time , zed ),
            err = fSTR->GetdRdt( time , zed ),
            erp = fSTR->GetdPhidt( time , zed );
         
         r*=f_rfudge;
         correction*=f_pfudge;

         if( fTrace )
            {
               std::cout<<"\trad: "<<r<<" Lorentz: "<<correction
                        <<" Err rad: "<<err<<" Err Lorentz"<<erp<<std::endl;
            }
         PointsArray.emplace_back(sp->first.idx,
                      sp->second.sec,sp->second.idx,
                      time,
                      r,correction,zed,
                      err,erp,sp->second.errz,
                      sp->first.height, sp->second.height);
         ++n;
      }
   std::qsort(PointsArray.data(),PointsArray.size(),sizeof(TSpacePoint),SpacePointCompare);
   if( fTrace )
      std::cout<<"Reco::AddSpacePoint # entries: "<<PointsArray.size()<<std::endl;
}
