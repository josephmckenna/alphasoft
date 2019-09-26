#include "../include/TSiliconVA.h"
#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>
// TSiliconVA Class =========================================================================================
//
// Class representing a single VA chip.
//
// JWS 10/10/2008
//
// ==========================================================================================================

ClassImp(TSiliconVA);

TSiliconVA::TSiliconVA()
{
  ASICNumber = -999;
  VF48ChannelNumber = -999;
  PSide = false;
  HitOR = false;
  PedFitP0 = -999.;
  PedFitP1 = -999.; 
  PedFitP2 = -999.; 
  PedFitChi= -999.;
  
  //Strips
  for (int i=0; i<128; i++)
  {
    RawADC[i]   =-9999;
    PedSubADC[i]=-9999;
    stripRMS[i] =-9999;
    Hit[i]      =false;
  }
}

TSiliconVA::TSiliconVA( Int_t _ASICNumber, Int_t _VF48ChannelNumber ) 
{
  ASICNumber = _ASICNumber;
  VF48ChannelNumber = _VF48ChannelNumber;
  PSide = false;
  HitOR = false;
  PedFitP0 = -999.;
  PedFitP1 = -999.; 
  PedFitP2 = -999.;  
  PedFitChi= -999.;
  
  //Strips
  for (int i=0; i<128; i++)
  {
    RawADC[i]   =-9999;
    PedSubADC[i]=-9999;
    stripRMS[i] =-9999;
    Hit[i]      =false;
  }
}

TSiliconVA::TSiliconVA( TSiliconVA* & VA )
{
  ASICNumber          = VA->GetASICNumber();
  VF48ChannelNumber   = VA->GetVF48ChannelNumber();
  PSide               = VA->IsAPSide();
  HitOR               = VA->IsAHitOR(); 
  RawADCMean          = VA->GetRawADCMean();
  RawADCRms           = VA->GetRawADCRms();
  FilteredADCMean     = VA->GetFilteredADCMean();
  PedFitP0            = VA->GetPedFitP0();
  PedFitP1            = VA->GetPedFitP1();
  PedFitP2            = VA->GetPedFitP2();
  
  for (int i=0; i<128; i++)
  {
     RawADC[i]   =VA->RawADC[i];
     PedSubADC[i]=VA->PedSubADC[i];
     stripRMS[i] =VA->stripRMS[i];
     Hit[i]      =VA->Hit[i];
  }
}

TSiliconVA::~TSiliconVA()
{

}

void TSiliconVA::AddStrip(const int i, const int adc,const double rms)
{
   RawADC[i]=adc;
   PedSubADC[i]=-9999.;
   stripRMS[i]=rms;
   Hit[i] = false;
}

void TSiliconVA::Reset()
{
  //Strips
  for (int i=0; i<128; i++)
  {
    RawADC[i]   =-9999;
    PedSubADC[i]=-9999;
    stripRMS[i] =-9999;
    Hit[i]      =false;
  }
}

Int_t TSiliconVA::CalcRawADCMeanSigma()
{
  Int_t sum0 = 0;
  Double_t sum1 = 0.;
  Double_t sum2 = 0.;

  Double_t RawADCVar;
  // loop over the strips

  for( uint i=0; i<128; i++ )
    {
      double raw_adc = RawADC[i];
      if (fabs(raw_adc) > 1024) continue;
      sum0++;
      sum1 += raw_adc;
      sum2 += raw_adc*raw_adc;     
    } 

  if(sum0>0) {RawADCMean = sum1/(double)sum0;
    RawADCVar = sum2/(double)sum0 - (RawADCMean*RawADCMean);
  }
  else{RawADCMean =0.;
    RawADCVar = 9999.;
  }
  if( RawADCVar > 0 ) RawADCRms = sqrt(RawADCVar); 

  return 0;
}

Int_t TSiliconVA::FitP2Pedestal(Double_t* StripRMSs, int & SiModNumber)
{
  CalcRawADCMeanSigma();

  
//  TH1D * fit = new TH1D("fit","fit",128,0,127);
//  TGraphErrors * fit = new TH1D("fit","fit",128,0,127);
  TF1 * f1 = new TF1("f1","pol2",1,126);
  f1->SetParameters(445., 0., 0.); //Start at typical values
//  f1->SetParLimits(0,200,600);
//  f1->SetParLimits(1,-2,2);
//  f1->SetParLimits(2,-1,1);
 // f1->SetRange
  //double FitChi=100;
  double Sigma=3./0.75;
  bool DrawMe=false;
  double ADC[128];
  double ADCerror[128];
  double chann[128];
  double channerr[128];
  for( Int_t i=0; i< 128; i++)
  {
    ADC[i]=0;
    ADCerror[i]=0;
    chann[i]=0;
    channerr[i]=0;
  }
  int points;
  //double ChiCut=20.;
  //while (FitChi>ChiCut )
  //{
    //  fit->Reset();
    double p_side_filter = RawADCMean + (Sigma* RawADCRms);
    double n_side_filter = RawADCMean - (Sigma* RawADCRms);
    Sigma=Sigma*0.75;
 
    //if (Sigma <0.1) break;
    Int_t c = 0;
    points=0;
    Int_t k=-1;
    for( uint i=0; i<128; i++ )
    {
      Double_t raw_adc = RawADC[i];
      if (abs(raw_adc) > 1024) continue;
      c++; k++;
      //std::cout <<raw_adc<<"\t";
      if( raw_adc > p_side_filter ) continue;
      if( raw_adc < n_side_filter ) continue;
      //Int_t stripNumber = Strip->GetStripNumber() + 128*(ASICNumber-1) + 512*(SiModNumber);
     
      ADC[points]=raw_adc;

      ADCerror[points]=3.*StripRMSs[k];//Strip->GetPedSubADC();
      chann[points]=c;
      points++;
      
     // fit->Fill((double) c, (double) Strip->GetRawADC() );
    }
    //std::cout<<std::endl;
    TGraphErrors* fit=new TGraphErrors(points,chann,ADC,channerr,ADCerror);
    //fit->Fit("f1","FNRQ");
    fit->Fit("f1","FRNQ"); 
    //FitChi=f1->GetChisquare();
    //std::cout <<"Strips: "<< points<<" Sigma: "<<Sigma<< " chi: "<< FitChi <<std::endl;
 
    if (DrawMe) //
//    if( FitChi>ChiCut*3 )// || DrawMe)
    {
      DrawMe=true;
      TCanvas* a=new TCanvas("MyCanvas","Test Canvas",10,10,900,500);
      fit->Draw();
      f1->Draw("SAME");
      a->Update();
      char ch;  
      std::cout <<f1->GetParameter(0) <<"\t"<<f1->GetParameter(1) <<"\t"<<f1->GetParameter(2) <<"\t"<<std::endl; 
      std::cout <<f1->GetChisquare() <<std::endl;
      std::cin.get(ch);
      delete a;
    }
    delete fit;
 // }
  PedFitP0 = f1->GetParameter(0);
  PedFitP1 = f1->GetParameter(1);
  PedFitP2 = f1->GetParameter(2);
  PedFitChi= f1->GetChisquare();
  
//  f1->Draw();
//std::cout <<f1->GetChisquare() <<std::endl;
 /* if (f1->GetChisquare()==0)
  {
	  Int_t c = 0;

    for( Int_t i=0; i<Strips.GetEntries(); i++ )
    {
      Strip = (TSiliconStrip*) Strips.At(i) ;
      //Double_t
      std::cout <<  Strip->GetRawADC() <<std::endl;;
      fit->Fill((double) c++, (double) Strip->GetRawADC() );
    }
  TCanvas* b=new TCanvas("MyCanvas","Test Canvas",10,10,900,500);

  fit->Draw();
      b->Update();
          char ch;  
    std::cin.get(ch);
      delete b;
  }*/
/*  if (f1->GetChisquare() > ChiCut) 
  {
	  TCanvas* a=new TCanvas("MyCanvas","Test Canvas",10,10,900,500);

fit->Draw("ALP");
f1->Draw("SAME");
a->Update();
    char ch;  
    std::cout <<f1->GetChisquare() <<std::endl;
    std::cin.get(ch);
    
  delete a;


  }
  //f1->Print();*/
  delete f1;
  return 0;
}




//Older version (probably unused):
/*
Int_t TSiliconVA::FitP2Pedestal()
{
  CalcRawADCMeanSigma();

  double p_side_filter = RawADCMean + (3.* RawADCRms);
  double n_side_filter = RawADCMean - (3.* RawADCRms);

  TH1D * fit = new TH1D("fit","fit",128,0,127);
  TF1 * f1 = new TF1("f1","pol2",1,126);
  Int_t c = 0;

  TSiliconStrip* Strip;
  for( Int_t i=0; i<Strips.GetEntriesFast(); i++ )
    {
      Strip = (TSiliconStrip*) Strips.At(i) ;
      Double_t raw_adc = Strip->GetRawADC();
      if( raw_adc > p_side_filter ) continue;
      if( raw_adc < n_side_filter ) continue;

      fit->Fill((double) c++, (double) Strip->GetRawADC() );
    }

  fit->Fit("f1","NRCQ");

  PedFitP0 = f1->GetParameter(0);
  PedFitP1 = f1->GetParameter(1);
  PedFitP2 = f1->GetParameter(2);
  
  delete fit;
  delete f1;
  return 0;
}
*/
Int_t TSiliconVA::CalcFilteredADCMean()
{
  double p_side_filter = RawADCMean + (3.* RawADCRms);
  double n_side_filter = RawADCMean - (3.* RawADCRms);

  int sum0=0;
  Double_t sum1(0.);

  for( uint i=0; i<128; i++ )
    {
      Double_t raw_adc = RawADC[i];
      if (abs(raw_adc) > 1024) continue;
      if( raw_adc > p_side_filter ) continue;
      if( raw_adc < n_side_filter ) continue;

      sum0++;
      sum1+=raw_adc;
    }

  if(sum0>0) FilteredADCMean = sum1/(double)sum0;
  else FilteredADCMean=RawADCMean;

  return 0;
}

Int_t TSiliconVA::CalcPedSubADCs()
{

#define DRAWTHIS 0
#if DRAWTHIS
  Double_t PedSUB[128];
  Double_t FIT[128];
  Double_t RAW[128];
  Double_t channel[128];
#endif
     // fit->Fill((double) c, (double) Strip->GetRawADC() );
    
  // loop over the strips
  for( uint i=0; i<128; i++ )
    {
      Double_t raw_adc = RawADC[i];
      if (abs(raw_adc) > 1024) continue;
      //std::cout << Strip->GetRawADC() <<"-"<< GetPedADCForStrip( i )<<std::endl;
      PedSubADC[i] = (Double_t) raw_adc - GetPedADCForStrip( i );
      //std::cout << Strip->GetRawADC()<<"\t";
      #if DRAWTHIS
      PedSUB[i]=PedSubADC;

      RAW[i]=(Double_t)raw_adc
      FIT[i]=GetPedADCForStrip( i );
      channel[i]=(Double_t)i;
      #endif
    }
    //std::cout <<std::endl;
  #if DRAWTHIS
  TGraph* SUB=new TGraph(128,channel,PedSUB); 
  TGraph* RRAW=new TGraph(128,channel,RAW); 
  TGraph* FFIT=new TGraph(128,channel,FIT); 
  TCanvas* a=new TCanvas("MyCanvas","Test Canvas",10,10,900,500);
  a->Divide(2,1);
  a->cd(1);
  SUB->Draw();
  a->cd(2);
  RRAW->Draw();
  FFIT->Draw("SAME");
      //f1->Draw("SAME");
      a->Update();
      char ch;  
      std::cout <<  PedFitP0 <<"\t"<<PedFitP1 << "\t"<<PedFitP2 <<std::endl;
      //std::cout <<f1->GetChisquare() <<std::endl;
      std::cin.get(ch);
      delete a;
  #endif
  return 0;
}

Int_t TSiliconVA::CalcPedSubADCs_NoFit()
{
  // loop over the strips
  for( uint i=0; i<128; i++ )
    {
      double raw_adc = RawADC[i];
      if (abs(raw_adc) > 1024) continue;
      PedSubADC[i] = raw_adc - GetFilteredADCMean();
    }
  return 0;
}


/* Retired function
Int_t TSiliconVA::CalcHits()
{
  TSiliconStrip* Strip;
  Int_t countHits(0);

  // loop over the strips
  for( Int_t i=0; i<Strips.GetEntries(); i++ )
   {
      Strip=(TSiliconStrip*) Strips.At(i);
      Int_t stripno = Strip->GetStripNumber();
      Double_t adc =   Strip->GetPedSubADC();
      if( PSide &&
	  (adc< 3*PHitThreshold || (stripno%128 !=127 && adc< PHitThreshold) )) {
         Strip->SetHit( true );
         HitOR = true;
         countHits++;
      }
      else if(!PSide &&
	 (adc> 3*NHitThreshold || (stripno%128 !=127 && adc> NHitThreshold) )) {
         Strip->SetHit( true );
         HitOR = true;
         countHits++;
       }
      else 
         Strip->SetHit( false );
    }

  return countHits;
}*/

Int_t TSiliconVA::CalcHits( Double_t & nsigma, int & SiModNumber )
{

   Int_t countHits(0);

   // loop over the strips
   if (PSide)
   {
      for( uint i=0; i<128; i++ )
      {
         //Double_t raw_adc = RawADC[i];
         //if (abs(raw_adc) > 1024) continue;
         double adc    = PedSubADC[i];
         double PHitThreshold = -1.*nsigma*fabs(stripRMS[i]);
         if (i==127) PHitThreshold*=3.;
         if (adc< PHitThreshold )
         {
            Hit[i]= true;
            HitOR = true;
            countHits++;
         }
         else
         {
            Hit[i]=false;
         }
      }
   }
   else
   {  //N side
      // loop over the strips
     for( uint i=0; i<128; i++ )
      {
         //Double_t raw_adc = RawADC[i];
         //if (abs(raw_adc) > 1024) continue;
         double adc    = PedSubADC[i];
         double NHitThreshold = 1.*nsigma*fabs(stripRMS[i]);
         if (i==127) NHitThreshold*=3.;
         if (adc> NHitThreshold )
         {
            Hit[i]= true;
            HitOR = true;
            countHits++;
         }
         else
         {
            Hit[i]=false;
         }
      }
   }

   //Int_t result(0);
   //if( Hit[i] ) result = 1;
   //printf("%f \t %f \t %f \t %f \t %d \n",  PedSubADC[i], stripRMS[i], PHitThreshold,NHitThreshold, result );

   return countHits;
}


void TSiliconVA::Print()
{  
  for( uint i=0; i<128; i++ )
    {
      //if (RawADC[i]<0) continue;
      Int_t HitInt(0);
      if( Hit[i] ) HitInt = 1;  
      printf( "... Strip Number %d \t Raw ADC %d \t PedSub ADC %f \t Hit %d \n", i, RawADC[i], PedSubADC[i], HitInt );
      //assert("FIXME");
       //Strips[i].Print();
    } 
}

void TSiliconVA::PrintToFile( FILE * f, Int_t mod_num )
{
  for( uint i=0; i<128; i++ )
    {
      if (RawADC[i]<0) continue;
         fprintf( f, "%d %d ", i, RawADC[i] );
    } 
}


Int_t TSiliconVA::CompressStrips()
{
   for (int i=0; i<128; i++)
   {
      if (!Hit[i])
      {
          RawADC[i]   =-9999;
          PedSubADC[i]=-9999;
          stripRMS[i] =-9999;
      }
   }
 /* TSiliconStrip* Strip;
  Int_t NumberOfStrips = Strips.size();

  for( Int_t i=0; i<NumberOfStrips; i++ )
    {
      Strip = (TSiliconStrip*) Strips.at(i);
      if( !Strip->IsAHit() )
        {
          delete Strips.at(i);
          Strips.at(i)=NULL;
        }
    } 
  */
  //Strips.clear();?
  
  return 0;
}

Int_t TSiliconVA::SuppressNoiseyStrips()
{
  //TSiliconStrip* Strip;
  // suppress strip 0
  // Strip = (TSiliconStrip*) Strips.At(0);
  // Strip->SetHit(false);
  // suppress strip 1
  // Strip = (TSiliconStrip*) Strips.At(1);
  // Strip->SetHit(false);
  // suppress strip 2
  // Strip = (TSiliconStrip*) Strips.At(2);
  // Strip->SetHit(false);
  // suppress strip 125
  // Strip = (TSiliconStrip*) Strips.At(125);
  // Strip->SetHit(false);
  // suppress strip 126
  // Strip = (TSiliconStrip*) Strips.At(126);
  // Strip->SetHit(false);
  // suppress strip 127
  
 Hit[127]=false;

  return 0;
}

Int_t TSiliconVA::CalcNRawHits()
{
  Int_t NRawHits(0);
  for( uint i=0; i<128; i++ )
      if( Hit[i] ) NRawHits++;
  return NRawHits;
}

