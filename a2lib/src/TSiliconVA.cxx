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
  
  //Strips
  for (int i=0; i<128; i++)
  {
    RawADC[i]   =-9999;
    PedSubADC[i]=-9999;
    stripRMS[i] =-99999;
    Hit[i]      =false;
  }
}

TSiliconVA::TSiliconVA( Int_t _ASICNumber, Int_t _VF48ChannelNumber ) 
{
  ASICNumber = _ASICNumber;
  VF48ChannelNumber = _VF48ChannelNumber;
  PSide = false;
  HitOR = false;
  
  nStrips=0;
  //Strips
  for (int i=0; i<128; i++)
  {
    RawADC[i]   =-9999;
    PedSubADC[i]=-9999;
    stripRMS[i] =-99999;
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

  nStrips             = VA->GetNoStrips();
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
   nStrips++;
}

void TSiliconVA::Reset()
{
  nStrips=0;
  
  //Strips
  for (int i=0; i<128; i++)
  {
    RawADC[i]   =-9999;
    PedSubADC[i]=-9999;
    stripRMS[i] =-99999;
    Hit[i]      =false;
  }
  PSide = false;
  HitOR = false;
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
      int raw_adc = RawADC[i];
      if (abs(raw_adc) > 1024) continue;
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

Int_t TSiliconVA::CalcFilteredADCMean()
{
  double p_side_filter = RawADCMean + (3.* RawADCRms);
  double n_side_filter = RawADCMean - (3.* RawADCRms);

  int sum0=0;
  Double_t sum1(0.);

  for( uint i=0; i<128; i++ )
    {
      Double_t raw_adc = RawADC[i];

      if( raw_adc > p_side_filter ) continue;
      if( raw_adc < n_side_filter ) continue;

      sum0++;
      sum1+=raw_adc;
    }

  if(sum0>0) FilteredADCMean = sum1/(double)sum0;
  else FilteredADCMean=RawADCMean;

  return 0;
}

Int_t TSiliconVA::CalcPedSubADCs_NoFit()
{
  // loop over the strips
  for( uint i=0; i<128; i++ )
    {
      double raw_adc = RawADC[i];
      PedSubADC[i] = raw_adc - GetFilteredADCMean();
    }
  return 0;
}

Int_t TSiliconVA::CalcPedSubADCs_LowPassFilter(const double &LowPassDelta)
{

   int stride=5;
   //Fill PedSubADC with difference to next strip (temporary)
   for (uint i=0; i<127; i++)
      PedSubADC[i]=RawADC[i]-RawADC[i+1];
   PedSubADC[127]=0;
   for( uint i=0; i<127; i++ )
   {
      double mean=0.;
      int count=0;
      for (int j=i-stride;j<i+stride; j++)
      {
         //Skip values out of range
         if (j<0) continue;
         if (j>127) break;
         //std::cout<<"DELTA i:"<<i<<"\tj:"<<j<<":\t"<<PedSubADC[j]<<std::endl;
          //printf("DELTA %d:\t%f\n",j,PedSubADC[i]);
         //skip strips that look like signal
         if (fabs(PedSubADC[j])>LowPassDelta) continue;
         mean+=RawADC[j];
         count++;
      }
      //Fill ADC with final filtered values
      if (count)
         PedSubADC[i]=RawADC[i]-mean/(double)count;
      else
         PedSubADC[i]=-9999;//?*/
   }

   return 5;
}

Int_t TSiliconVA::CalcHits( const double & nsigma, int & SiModNumber )
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
      if (abs(RawADC[i])>2000) continue;
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
          stripRMS[i] =0;
      }
   }
  
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

