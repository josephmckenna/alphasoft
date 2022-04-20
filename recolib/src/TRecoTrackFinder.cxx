#include "TRecoTrackFinder.hh"

TRecoTrackFinder::TRecoTrackFinder(AnaSettings* ana_settings, bool trace ): fTrace(trace)
{
   fNspacepointsCut = ana_settings->GetInt("RecoModule","NpointsCut");
   fPointsDistCut = ana_settings->GetDouble("RecoModule","PointsDistCut");
   fMaxIncreseAdapt = ana_settings->GetDouble("RecoModule","MaxIncreseAdapt");
   fSeedRadCut = ana_settings->GetDouble("RecoModule","SeedRadCut");

   fLastPointRadCut = ana_settings->GetDouble("RecoModule","LastPointRadCut");

   fLambda = ana_settings->GetDouble("RecoModule","Lambda_NN");
   fAlpha = ana_settings->GetDouble("RecoModule","Alpha_NN");
   fB = ana_settings->GetDouble("RecoModule","B_NN");
   fTemp = ana_settings->GetDouble("RecoModule","T_NN");
   fC = ana_settings->GetDouble("RecoModule","C_NN");
   fMu = ana_settings->GetDouble("RecoModule","Mu_NN");
   fCosCut = ana_settings->GetDouble("RecoModule","CosCut_NN");
   fVThres = ana_settings->GetDouble("RecoModule","VThres_NN");

   fDNormXY = ana_settings->GetDouble("RecoModule","DNormXY_NN");
   fDNormZ = ana_settings->GetDouble("RecoModule","DNormZ_NN");

   fTscale = ana_settings->GetDouble("RecoModule","TScale_NN");
   fMaxIt = ana_settings->GetInt("RecoModule","MaxIt_NN");
   fItThres = ana_settings->GetDouble("RecoModule","ItThres_NN");

}

    


int TRecoTrackFinder::FindTracks(const std::vector<TSpacePoint> SortedPoints , std::vector<track_t>& TrackVector,  finderChoice finder)
{

   switch(finder)
      {
      case adaptive:
         pattrec = new AdaptiveFinder( &SortedPoints, fMaxIncreseAdapt, fLastPointRadCut);
         break;
      case neural:
         pattrec = new NeuralFinder(  &SortedPoints );
         ((NeuralFinder*)pattrec)->SetLambda(fLambda);
         ((NeuralFinder*)pattrec)->SetAlpha(fAlpha);
         ((NeuralFinder*)pattrec)->SetB(fB);
         ((NeuralFinder*)pattrec)->SetTemp(fTemp);
         ((NeuralFinder*)pattrec)->SetC(fC);
         ((NeuralFinder*)pattrec)->SetMu(fMu);
         ((NeuralFinder*)pattrec)->SetCosCut(fCosCut);
         ((NeuralFinder*)pattrec)->SetVThres(fVThres);
         ((NeuralFinder*)pattrec)->SetDNormXY(fDNormXY);
         ((NeuralFinder*)pattrec)->SetDNormZ(fDNormZ);
         ((NeuralFinder*)pattrec)->SetTscale(fTscale);
         ((NeuralFinder*)pattrec)->SetMaxIt(fMaxIt);
         ((NeuralFinder*)pattrec)->SetItThres(fItThres);
         break;
      case base:
         pattrec = new TracksFinder(  &SortedPoints ); 
         break;
      default:
         pattrec = new AdaptiveFinder(  &SortedPoints , fMaxIncreseAdapt,fLastPointRadCut);
         break;
      }
   
   if( fTrace ) 
      pattrec->SetDebug();

   pattrec->SetPointsDistCut(fPointsDistCut);
   pattrec->SetNpointsCut(fNspacepointsCut);
   pattrec->SetSeedRadCut(fSeedRadCut);

   int stat = pattrec->RecTracks(TrackVector);
   if( fTrace ) 
      std::cout<<"Reco::FindTracks status: "<<stat<<std::endl;
   int tk,npc,rc;
   pattrec->GetReasons(tk,npc,rc);
   track_not_advancing += tk;
   points_cut += npc;
   rad_cut += rc;

   delete pattrec;
   return stat;
}


void TRecoTrackFinder::PrintPattRec()
{
   std::cout<<"Reco:: pattrec failed\ttrack not advanving: "<<track_not_advancing
            <<"\tpoints cut: "<<points_cut
            <<"\tradius cut: "<<rad_cut<<std::endl;
}