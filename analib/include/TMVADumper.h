
#include "generalizedspher.h"

class TMVADumper
{
   public:
   TTree* fTree;
   TMVADumper(TTree* tree)
   {
      fTree = tree;
   }
   virtual void LoadVariablesToReader(TMVA::Reader* reader) = 0;

   virtual bool UpdateVariables(TAFlowEvent* flow, int currentEventNumber) = 0;

   void Fill()
   {
      fTree->Fill();
   }
};


// Simple class that writes out the Event ID number and Run Number 
class TA2MVAEventIDDumper: public TMVADumper
{
   public:
      int fRunNumber;
      int fEventID;

   // Set up Tree Writer
   TA2MVAEventIDDumper(TTree* tree): TMVADumper(tree)
   {
      fTree->Branch("RunNumber", &fRunNumber, "RunNumber/I");
      fTree->Branch("EventID", &fEventID, "RunNumber/I");
   }

   // Set up Tree Reader
   void LoadVariablesToReader(TMVA::Reader* reader)
   {
      reader->AddVariable("RunNumber", &fRunNumber);      
      reader->AddVariable("EventID", &fEventID);
   }

   // Set 
   bool UpdateVariables(TAFlowEvent* flow, int currentEventNumber)
   {
      // Safely handle case where we are given the wrong flow
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
         return false;
      }
      // Safely handle case where we have the wrong Event ID
      TSiliconEvent* siliconEvent=fe->silevent;
      if ( currentEventNumber != siliconEvent->GetVF48NEvent() )
      {
         return false;
      }
      // Update class members
      fEventID = siliconEvent->GetVF48NEvent();
      fRunNumber = siliconEvent->GetRunNumber();
      // Report success!
      return true;
   }
   
   //
   

};

class TA2MVAClassicDumper: public TMVADumper
{
   public:
      float nhits,residual,r,S0rawPerp,S0axisrawZ,phi_S0axisraw,nCT,nGT,tracksdca,curvemin,curvemean,lambdamin,lambdamean,curvesign,phi;
   
   TA2MVAClassicDumper(TTree* tree): TMVADumper(tree)
   {
      fTree->Branch("nhits", &nhits, "nhits/F");
      fTree->Branch("residual", &residual, "residual/F");
      fTree->Branch("r", &r, "r/F");
      fTree->Branch("S0rawPerp", &S0rawPerp, "S0rawPerp/F");
      fTree->Branch("S0axisrawZ", &S0axisrawZ, "S0axisrawZ/F");
      fTree->Branch("phi_S0axisraw", &phi_S0axisraw, "phi_S0axisraw/F");
      fTree->Branch("nCT", &nCT, "nCT/F");
      fTree->Branch("nGT", &nGT, "nGT/F");
      fTree->Branch("tracksdca", &tracksdca, "tracksdca/F");
      fTree->Branch("curvemin", &curvemin, "curvemin/F");
      fTree->Branch("curvemean", &curvemean, "curvemean/F");
      fTree->Branch("lambdamin", &lambdamin, "lambdamin/F");
      fTree->Branch("lambdamean", &lambdamean, "lambdamean/F");
      fTree->Branch("curvesign", &curvesign, "curvesign/F");
      fTree->Branch("phi", &phi, "phi/F");
   }

   void LoadVariablesToReader(TMVA::Reader* reader)
   {
      reader->AddVariable("nhits", &nhits);      
      reader->AddVariable("residual", &residual);
      reader->AddVariable("r", &r);
      reader->AddVariable("S0rawPerp", &S0rawPerp);
      reader->AddVariable("S0axisrawZ", &S0axisrawZ);
      reader->AddVariable("phi_S0axisraw", &phi_S0axisraw);
      reader->AddVariable("nCT", &nCT);
      reader->AddVariable("nGT", &nGT);
      reader->AddVariable("tracksdca", &tracksdca);
      reader->AddVariable("curvemin", &curvemin);
      reader->AddVariable("curvemean", &curvemean);
      reader->AddVariable("lambdamin", &lambdamin);
      reader->AddVariable("lambdamean", &lambdamean);
      reader->AddVariable("curvesign", &curvesign);
      reader->AddVariable("phi", &phi);
   }


   bool UpdateVariables(TAFlowEvent* flow, int currentEventNumber)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
         return false;
      }
      TAlphaEvent* alphaEvent=fe->alphaevent;
      TSiliconEvent* siliconEvent=fe->silevent;

      if ( currentEventNumber != siliconEvent->GetVF48NEvent() )
      {
         //std::cout<<currentEventNumber <<" != "<< siliconEvent->GetVF48NEvent() <<std::endl;
         return false;
      }
      //std::cout<<"\t"<<currentEventNumber <<" == "<< siliconEvent->GetVF48NEvent()<< "  :)" <<std::endl;
      this->nhits=alphaEvent->GetNHits();
      this->residual = siliconEvent->GetResidual();
      TVector3* vtx = siliconEvent->GetVertex();
      this->r = vtx->Perp();
      this->phi = vtx->Phi();
      
      this->tracksdca=alphaEvent->GetVertex()->GetDCA();     
      std::vector<double> velxraw;
      std::vector<double> velyraw;
      std::vector<double> velzraw;

      Int_t nAT =  alphaEvent->GetNHelices(); // all tracks
      this->nCT = 0;
      Int_t nraw = 0;
  
      //Unused
      //Double_t AT_MeanHitSig=0; //Average hit significance
      //Unused
      //Double_t CT_MeanHitSig=0; //Average hit significance
      Double_t AT_SumHitSig=0;
      Double_t CT_SumHitSig=0;
      Int_t AT_HitSigCounter=0;
      Int_t CT_HitSigCounter=0;
      for (int i = 0; i< nAT ; ++i)
      {
         TAlphaEventHelix* aehlx = alphaEvent->GetHelix(i);
         if (!aehlx) continue;
         Double_t fc = aehlx->Getfc();
         Double_t fphi0 = aehlx->Getfphi();
         Double_t fLambda = aehlx->Getflambda();
         //Double_t s=0.; // calculates velx,y,z at POCA
         // special case for s = 0
         Int_t HelixHits=aehlx->GetNHits();
         for (int j=0; j<HelixHits; j++)
         {
            TAlphaEventHit* aehlx_hit=aehlx->GetHit(j);
            AT_SumHitSig+=aehlx_hit->GetHitSignifance();
            AT_HitSigCounter++;
            //delete aehlx_hit;
         }
         // select good helices, after removal of duplicates

         // This seems to be true all the time... please check - Joe
         if (aehlx->GetHelixStatus()==1)
         {
            for (int j=0; j<HelixHits; j++)
            {
               TAlphaEventHit* aehlx_hit=aehlx->GetHit(j);
               CT_SumHitSig+=aehlx_hit->GetHitSignifance();
               CT_HitSigCounter++;
               //delete aehlx_hit;
            }
            ++nraw; // == ntracks
            velxraw.push_back( - TMath::Sin(fphi0));
            velyraw.push_back( TMath::Cos(fphi0)) ;
            velzraw.push_back( fLambda );
            this->nCT++;
         }
      }
      std::vector<double> velx;
      std::vector<double> vely;
      std::vector<double> velz;
      // alpha event part
      TAlphaEventVertex* aevtx = alphaEvent->GetVertex();
      Int_t nGTL = aevtx->GetNHelices();// tracks with vertex
      this->nGT = 0;
      this->curvemin=9999.;
      this->curvemean=0.;
      this->lambdamin=9999.;
      this->lambdamean=0;
      this->curvesign=0;
      for (int i = 0; i< nGTL ; ++i)
      {
         TAlphaEventHelix* aehlx = aevtx->GetHelix(i);
         //if(aehlx->GetHelixStatus()<0) continue;
         Double_t fc = aehlx->Getfc();
         Double_t fphi0 = aehlx->Getfphi();
         Double_t fLambda = aehlx->Getflambda();

         //Unused
         //Double_t s=0.; // calculates velx,y,z at POCA
         // special case for s = 0
         velx.push_back( - TMath::Sin(fphi0) );
         vely.push_back( TMath::Cos(fphi0) ) ;
         velz.push_back( fLambda );

         // select good helices, after removal of duplicates
         if (aehlx->GetHelixStatus()==1)
         {
            this->nGT++;
            this->curvemin= fabs(fc)>this->curvemin? this->curvemin:fabs(fc);
            this->lambdamin= fabs(fLambda)>this->lambdamin? this->lambdamin:fabs(fLambda);
            this->curvemean+=fabs(fc);
            this->lambdamean+=fabs(fLambda);
            this->curvesign+=(fc>0)?1:-1;
         }
      }
      if(this->nGT>0){
         this->lambdamean/=this->nGT;
         this->curvemean/=this->nGT;
      }

      //Unused
      //Double_t S0rawl1 = -99.;
      //Unused
      //Double_t S0rawl2 = -99.;
      //Unused
      //Double_t S0rawl3 = -99.;
      //Unused in online_mva
      Double_t S0axisrawX = -99.;
      //Unused in online_mva
      Double_t S0axisrawY = -99.;
      this->S0axisrawZ = -99.;

      if(nraw>0)
      {
         TVector3* S0axisraw;
         TVector3* S0valuesraw;
         sphericity(velxraw, velyraw, velzraw, 0, &S0axisraw, &S0valuesraw); // generalizedspher.h
         this->S0rawPerp = S0valuesraw->Perp();
         //Unused
         //S0rawl1 = S0valuesraw->X();
         //Unused
         //S0rawl2 = S0valuesraw->Y();
         //Unused
         //S0rawl3 = S0valuesraw->Z();

         //Unused in online_mva
         S0axisrawX = S0axisraw->X();
         //Unused
         S0axisrawY = S0axisraw->Y();
         this->S0axisrawZ = S0axisraw->Z();
         this->phi_S0axisraw = TMath::ACos(S0axisrawY/TMath::Sqrt(S0axisrawX*S0axisrawX+S0axisrawY*S0axisrawY));
         delete S0axisraw;
         delete S0valuesraw;
      }
      return true;
   }


};



class TA2MVAXYZ: public TMVADumper
{
   public:
      float fX, fY, fZ;
   
   TA2MVAXYZ(TTree* tree): TMVADumper(tree)
   {
      fTree->Branch("X", &fX, "X/F");
      fTree->Branch("Y", &fY, "Y/F");
      fTree->Branch("Z", &fZ, "Z/F");
   }

   void LoadVariablesToReader(TMVA::Reader* reader)
   {
      reader->AddVariable("X", &fX);
      reader->AddVariable("Y", &fY);
      reader->AddVariable("Z", &fZ);
   }

   bool UpdateVariables(TAFlowEvent* flow, int currentEventNumber)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
         return false;
      }
      TSiliconEvent* siliconEvent=fe->silevent;
      if ( currentEventNumber != siliconEvent->GetVF48NEvent() )
         return false;
      TVector3* vtx = siliconEvent->GetVertex();
      this->fX = vtx->X();
      this->fY = vtx->Y();
      this->fZ = vtx->Z();
      return true;
   }

};
