#include "TSpacePointBuilder.hh"

TSpacePointBuilder::TSpacePointBuilder(std::string json, double B, bool trace)
   : fMagneticField(B), fTrace(trace), fLocation("CERN"), f_rfudge(1.), f_pfudge(1.)
{
   ana_settings = new AnaSettings(json.c_str());
   if (fMagneticField < 0.)
      fSTR = new LookUpTable(ALPHAg::_co2frac); // field map version (simulation)
   else
      fSTR = new LookUpTable(ALPHAg::_co2frac, fMagneticField, fLocation); // uniform field version (simulation)
   std::cout << "Reco::Reco()  max time: " << fSTR->GetMaxTime() << " ns" << std::endl;
}

TSpacePointBuilder::TSpacePointBuilder(AnaSettings *ana_set, double B, std::string loc, bool trace)
   : ana_settings(ana_set), fMagneticField(B), fTrace(trace), fLocation(loc), f_rfudge(1.), f_pfudge(1.)
{
   if (fMagneticField < 0.) // garfield++ sim with field map
   {
      fSTR           = new LookUpTable(ALPHAg::_co2frac); // field map version (simulation)
      fMagneticField = 1.;
   } else {
      fSTR = new LookUpTable(ALPHAg::_co2frac, fMagneticField, fLocation); // uniform field version (simulation)
   }
   std::cout << "Reco::Reco()  max time: " << fSTR->GetMaxTime() << " ns" << std::endl;
}

void TSpacePointBuilder::UseSTRfromData(int runNumber)
{
   delete fSTR;
   std::cout << "Reco::UseSTRfromData( " << runNumber << " )" << std::endl;
   fSTR           = new LookUpTable(runNumber);
   fMagneticField = 0.; // data driven STR valid only for B=0T
}

void TSpacePointBuilder::BuildSpacePointArray(
   const std::vector<std::pair<ALPHAg::TWireSignal, ALPHAg::TPadSignal>> spacepoints,
   std::vector<TSpacePoint>                                             &PointsArray,
   double                                                                z_fid // Z cut by default off
)
{
   int n = PointsArray.size();
   PointsArray.reserve(PointsArray.size() + spacepoints.size());
   for (auto sp = spacepoints.begin(); sp != spacepoints.end(); ++sp) {
      // STR: (t,z)->(r,phi)
      const double time = sp->first.t, zed = sp->second.z;
      if (fabs(zed) > z_fid) continue;
      if (fTrace) {
         double z = (double(sp->second.idx) + 0.5) * ALPHAg::_padpitch - ALPHAg::_halflength;
         std::cout << "Reco::AddSpacePoint " << n << " aw: " << sp->first.idx << " t: " << time
                   << "\tcol: " << sp->second.sec << " row: " << sp->second.idx << " (z: " << z << ") ~ "
                   << sp->second.z << " err: " << sp->second.errz << std::endl;
      }
      double r = fSTR->GetRadius(time, zed), correction = fSTR->GetAzimuth(time, zed), err = fSTR->GetdRdt(time, zed),
             erp = fSTR->GetdPhidt(time, zed);

      r *= f_rfudge;
      correction *= f_pfudge;

      if (fTrace) {
         std::cout << "\trad: " << r << " Lorentz: " << correction << " Err rad: " << err << " Err Lorentz" << erp
                   << std::endl;
      }
      PointsArray.emplace_back(sp->first.idx, sp->second.sec, sp->second.idx, time, r, correction, zed, err, erp,
                               sp->second.errz, sp->first.height, sp->second.height);
      ++n;
   }
   std::qsort(PointsArray.data(), PointsArray.size(), sizeof(TSpacePoint), SpacePointCompare);
   if (fTrace) std::cout << "Reco::AddSpacePoint # entries: " << PointsArray.size() << std::endl;
}

#ifdef BUILD_AG_SIM
void TSpacePointBuilder::BuildMCSpacePointArray(const TClonesArray *points, std::vector<TSpacePoint> &PointsArray)
{
   int Npoints = points->GetEntries();
   for( int j=0; j<Npoints; ++j )
      {
         TMChit* h = (TMChit*) points->At(j);
         double time = h->GetTime(),
            zed = h->GetZ();

         double rad = fSTR->GetRadius( time , zed ), lor = fSTR->GetAzimuth( time , zed ),
            err = fSTR->GetdRdt( time , zed ), erp = fSTR->GetdPhidt( time , zed );

         double phi = h->GetPhi() - lor;
         if( phi < 0. ) phi += TMath::TwoPi();
         if( phi >= TMath::TwoPi() )
            phi = fmod(phi,TMath::TwoPi());
         int aw = phi/ALPHAg::_anodepitch - 0.5;
         int sec = int( phi/(2.*M_PI)*ALPHAg::_padcol ),
            row = int( zed/ALPHAg::_halflength*0.5*ALPHAg::_padrow );

         //      double y = rad*TMath::Sin( phi ), x = rad*TMath::Cos( phi );

         double erz = 1.5;
         TSpacePoint point;
         point.Setup( aw, sec, row, time,
                       rad, lor, zed,
                       err, erp, erz,
                       // This sets the energy for both wire and pad... 
                       // perhaps TMChit should have two energy entries
                       h->GetDepositEnergy(),h->GetDepositEnergy() );
         point.SetTrackID(h->GetTrackID());
         point.SetTrackPDG(h->GetTrackPDG());
         PointsArray.push_back(point);
      }
}
#endif
